// Copyright 2015-2022 Manus

#include "ManusComponent.h"
#include "Manus.h"
#include "ManusBlueprintTypes.h"
#include "ManusBlueprintLibrary.h"
#include "ManusReplicator.h"
#include "ManusSettings.h"
#include "ManusSkeleton.h"
#include "CoreSdk.h"

#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/CollisionProfile.h"
#include "Net/UnrealNetwork.h"
#include "Engine/NetConnection.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"


UManusComponent::UManusComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bTickInEditor = true;
	SetGenerateOverlapEvents(true);
	SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	OnComponentHit.AddDynamic(this, &UManusComponent::OnHit);
	BodyInstance.bNotifyRigidBodyCollision = true;

	// Replication
	SetIsReplicatedByDefault(true);

	// Default values
    ManusReplicatorId = 0;

	bFingerHaptics = true;
}

void UManusComponent::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITOR
	// Tell the Manus Live Link source that we are using this user's data.
	if (GIsEditor)
	{
		// Register objects loaded in Editor so that we can animate in Editor viewports.
        if (ManusSkeleton != NULL)
        {
            FManusModule::Get().AddObjectUsingManusLiveLinkUser(ManusSkeleton->TargetUserIndexData.userIndex, ManusSkeleton, this);
        }
	}
#endif // WITH_EDITOR

	// Make sure we are using the correct skeletal mesh
	RefreshManusSkeleton();
}

void UManusComponent::BeginDestroy()
{
	Super::BeginDestroy();

#if WITH_EDITOR
	// Tell the Manus Live Link source that we are not using this user's data anymore.
	// The same is done in EndPlay when GIsEditor is false.
	if (GIsEditor)
	{
        if (ManusSkeleton != NULL)
        {
            FManusModule::Get().RemoveObjectUsingManusLiveLinkUser(ManusSkeleton->TargetUserIndexData.userIndex, ManusSkeleton, this);
        }
	}
#endif // WITH_EDITOR
}

void UManusComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UManusComponent, ManusReplicatorId, COND_InitialOnly);
	//DOREPLIFETIME_CONDITION(UManusComponent, ManusSkeleton->TargetUserIndexData.userIndex, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UManusComponent, ManusSkeleton, COND_InitialOnly);
}

void UManusComponent::BeginPlay()
{
	Super::BeginPlay();

	// Tell the Manus Live Link source that we are using this user's data.
    if (ManusSkeleton != NULL)
    {
        FManusModule::Get().AddObjectUsingManusLiveLinkUser(ManusSkeleton->TargetUserIndexData.userIndex, ManusSkeleton, this);
    }
	// Setup everything to have the Finger Haptics working
	if (bFingerHaptics)
	{
		SetNotifyRigidBodyCollision(true);

		if (!BodyInstance.bNotifyRigidBodyCollision)
		{
			BodyInstance.bNotifyRigidBodyCollision = true;
			UE_LOG(LogManus, Warning, TEXT("Manus component: \"Simulation Generates Hit Events\" was set to TRUE to support the Manus Glove finger haptics."));
		}
		if (!GetGenerateOverlapEvents())
		{
			SetGenerateOverlapEvents(true);
			UE_LOG(LogManus, Warning, TEXT("Manus component: \"Generate Overlap Events\" was set to TRUE to support the Manus Glove finger haptics."));
		}
	}
}

void UManusComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Tell the Manus Live Link source that we are not using this user's data anymore.
    if (ManusSkeleton != NULL)
    {
        FManusModule::Get().RemoveObjectUsingManusLiveLinkUser(ManusSkeleton->TargetUserIndexData.userIndex, ManusSkeleton, this);
    }
	Super::EndPlay(EndPlayReason);
}

void UManusComponent::TickComponent(float DeltaSeconds, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);

	// Init Manus Replicator ID
	InitManusReplicatorID(); // todo may be obsolete, but may also be part of unreal networking. so going to keep it for now.
}


void UManusComponent::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Update vibrate powers according to current overlaps (used when the body is set to Block)
	if (bFingerHaptics && (OtherActor != NULL) && (OtherComp != NULL) && !OtherComp->IsAttachedTo(this))
	{
		int64 t_SkeletonId = ManusSkeleton->ManusSkeletonId;
        EManusHandType t_Side = EManusHandType::Left;
        for (TPair<FName, FManusChainSetup> t_ChainPair : ManusSkeleton->ChainsIndexMap)
        {
            if (t_ChainPair.Value.Side == EManusSide::Right) t_Side = EManusHandType::Right;
            break;
        }                    
        UManusBlueprintLibrary::VibrateFingersForSkeleton(t_SkeletonId, t_Side); // nothing finger specific yet. for that we have to analyze the entire hand.
        return;        
	}
}

bool UManusComponent::ComponentOverlapMultiImpl(TArray<struct FOverlapResult>& OutOverlaps, const UWorld* World, const FVector& Pos, const FQuat& Quat, ECollisionChannel TestChannel, const struct FComponentQueryParams& Params, const struct FCollisionObjectQueryParams& ObjectQueryParams) const
{
	OutOverlaps.Reset();

	if (!Bodies.IsValidIndex(RootBodyData.BodyIndex))
	{
		return false;
	}

	const FTransform WorldToComponent(GetComponentTransform());// .Inverse());
	const FCollisionResponseParams ResponseParams(GetCollisionResponseToChannels());

	FComponentQueryParams ParamsWithSelf = Params;
	ParamsWithSelf.AddIgnoredComponent(this);

	// Below is a rewritten version of the original ComponentOverlapMultiImpl function,
	// with its original version commented right below. It was rewritten to insert the
	// Body index in the overlap results, so that we can know what bone was overlapped.

/**	bool bHaveBlockingHit = false;
*	for (const FBodyInstance* Body : Bodies)
*	{
*		checkSlow(Body);
*		if (Body->OverlapMulti(OutOverlaps, World, &WorldToComponent, Pos, Quat, TestChannel, ParamsWithSelf, ResponseParams, ObjectQueryParams))
*		{
*			bHaveBlockingHit = true;
*		}
*	}
*/	

	bool bHaveBlockingHit = false;
	for (int i = 0; i < Bodies.Num(); i++)
	{
		const FBodyInstance* Body = Bodies[i];
		checkSlow(Body);
		int PreviousOverlapsNum = OutOverlaps.Num();
		if (Body->OverlapMulti(OutOverlaps, World, &WorldToComponent, Pos, Quat, TestChannel, ParamsWithSelf, ResponseParams, ObjectQueryParams))
		{
			bHaveBlockingHit = true;
		}
		for (int j = PreviousOverlapsNum; j < OutOverlaps.Num(); j++)
		{
			OutOverlaps[j].ItemIndex = i;
		}
	}
	return bHaveBlockingHit;
}

void UManusComponent::InitManusReplicatorID()
{
	if (GetOwner()->HasAuthority() && ManusReplicatorId == 0)
	{
		UNetConnection* NetConnection = GetOwner()->GetNetConnection();
		if (NetConnection)
		{
			APlayerController* PlayerController = Cast<APlayerController>(NetConnection->OwningActor);
			if (PlayerController && PlayerController->PlayerState)
			{
				ManusReplicatorId = PlayerController->PlayerState->GetPlayerId();
			}
		}
		if (ManusReplicatorId == 0)
		{
            //UE_LOG(LogManus, Warning, TEXT("this is not the client ."));
			APawn* Pawn = Cast<APawn>(GetOwner());
			if (Pawn && Pawn->GetPlayerState())
			{
				ManusReplicatorId = Pawn->GetPlayerState()->GetPlayerId();
			}
			else
			{
				APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
				if (LocalPlayerController && LocalPlayerController->PlayerState)
				{
					ManusReplicatorId = LocalPlayerController->PlayerState->GetPlayerId();
				}
			}
		}
	}
}
FString UManusComponent::GetLiveLinkSubjectName() const
{
	FString s = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(this).ToString();

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->GetPlayerState())
	{
		s += " - ";
		s += FString::FromInt(Pawn->GetPlayerState()->GetPlayerId());
	}

	return s;
}

EManusRet UManusComponent::VibrateManusGloveFingers(EManusHandType HandType, float ThumbPower /*= 1.0f*/, float IndexPower /*= 1.0f*/, float MiddlePower /*= 1.0f*/, float RingPower /*= 1.0f*/, float PinkyPower /*= 1.0f*/)
{
	int64 t_HapticDongleId;
	EManusRet t_Ret = UManusBlueprintLibrary::GetFirstHapticDongle(t_HapticDongleId);
	if (t_Ret == EManusRet::Success)
	{
		return UManusBlueprintLibrary::VibrateFingers(t_HapticDongleId, HandType, ThumbPower, IndexPower, MiddlePower, RingPower, PinkyPower);
	}
	return t_Ret;
}

EManusRet UManusComponent::VibrateManusGloveFingersForSkeleton(int64 SkeletonId, EManusHandType HandType, float ThumbPower /*= 1.0f*/, float IndexPower /*= 1.0f*/, float MiddlePower /*= 1.0f*/, float RingPower /*= 1.0f*/, float PinkyPower /*= 1.0f*/)
{
	return UManusBlueprintLibrary::VibrateFingersForSkeleton(SkeletonId, HandType, ThumbPower, IndexPower, MiddlePower, RingPower, PinkyPower);
}


bool UManusComponent::IsLocallyControlled() const
{
	if (GEngine && GEngine->GetWorldContextFromWorld(GetWorld()))
	{
		APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
		return (LocalPlayerController && GetOwner() == LocalPlayerController->GetPawn());
	}
	return true;
}

bool UManusComponent::IsLocallyOwned() const
{
	if (GEngine && GEngine->GetWorldContextFromWorld(GetWorld()) && GetOwner())
	{
		APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
		if (LocalPlayerController)
		{
			if (GetOwner() == LocalPlayerController->GetPawn())
			{
				return true;
			}

			UNetConnection* NetConnection = GetOwner()->GetNetConnection();
			if (NetConnection)
			{
				return (NetConnection->OwningActor == LocalPlayerController);
			}
			else
			{
				return GetOwner()->GetLocalRole() > ROLE_SimulatedProxy;
			}
		}

		if (!GetOwner()->GetOwner())
		{
			return false;
		}
	}
	return true;
}

void UManusComponent::RefreshManusSkeleton()
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
    if (ManusSkeleton && GetSkinnedAsset() != ManusSkeleton->SkeletalMesh) // the manus skeleton mesh may need to be swapped out for version 5.2. right now it is between version so it is ok. future unreal version may need work
    {
        SetSkinnedAsset(ManusSkeleton->SkeletalMesh);
    }
#else
    if (ManusSkeleton && SkeletalMesh != ManusSkeleton->SkeletalMesh)
    {
        SetSkeletalMesh(ManusSkeleton->SkeletalMesh);
    }
#endif
}

#if WITH_EDITOR
void UManusComponent::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange)
	{
		const FName PropertyName = PropertyAboutToChange->GetFName();
        if (ManusSkeleton != NULL)
        {
            // When the Manus Live Link User index is about to change
            if (PropertyName == GET_MEMBER_NAME_CHECKED(UManusComponent, ManusSkeleton))
            {
                // We are switching user, update which user's data we're using
                FManusModule::Get().RemoveObjectUsingManusLiveLinkUser(ManusSkeleton->TargetUserIndexData.userIndex, ManusSkeleton, this);
            }
        }
	}
}

void UManusComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
    
	// When the Manus Live Link User index changed
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UManusComponent, ManusSkeleton))
	{
		// We switched user, update which user's data we're using
		FManusModule::Get().AddObjectUsingManusLiveLinkUser(ManusSkeleton->TargetUserIndexData.userIndex, ManusSkeleton, this);

		// Use the skeleton of the new Manus Live Link User
		RefreshManusSkeleton();
	}
    
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif //WITH_EDITOR


int UManusComponent::GetManusSkeletonId() const
{
    return ManusSkeleton->ManusSkeletonId;
}