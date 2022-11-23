// Copyright 2015-2020 Manus

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
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 24
	SetIsReplicatedByDefault(true);
#else
	SetIsReplicated(true);
#endif

	// Default values
	ManusDashboardUserIndex = 0;
	MotionCaptureType = EManusMotionCaptureType::BothHands;
	bMirrorHandBone = false;
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
		FManusModule::Get().AddObjectUsingManusLiveLinkUser(ManusDashboardUserIndex, ManusSkeleton, this);
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
		FManusModule::Get().RemoveObjectUsingManusLiveLinkUser(ManusDashboardUserIndex, ManusSkeleton, this);
	}
#endif // WITH_EDITOR
}

void UManusComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UManusComponent, ManusReplicatorId, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UManusComponent, ManusDashboardUserIndex, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UManusComponent, ManusSkeleton, COND_InitialOnly);
}

void UManusComponent::BeginPlay()
{
	Super::BeginPlay();

	// Tell the Manus Live Link source that we are using this user's data.
	FManusModule::Get().AddObjectUsingManusLiveLinkUser(ManusDashboardUserIndex, ManusSkeleton, this);

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
	FManusModule::Get().RemoveObjectUsingManusLiveLinkUser(ManusDashboardUserIndex, ManusSkeleton, this);

	Super::EndPlay(EndPlayReason);
}

void UManusComponent::TickComponent(float DeltaSeconds, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);

	// Init Manus Replicator ID
	InitManusReplicatorID();

	// Only when locally controlled
	if (IsLocallyControlled())
	{
		// Finger Haptics
		if (bFingerHaptics)
		{
			TickFingerHaptics();
		}

		// Gesture detection
		TickGestureDetection();
	}
}

void UManusComponent::TickFingerHaptics()
{
	// Update vibrate powers according to current overlaps (used when the body is set to Overlap)
	const TArray<FOverlapInfo>& OverlapInfos = GetOverlapInfos();
	for (const FOverlapInfo& OverlapInfo : OverlapInfos)
	{
		// Don't vibrate when the overlapping component is attached to the Manus component
		if (!OverlapInfo.OverlapInfo.Component->IsAttachedTo(this))
		{
			// Retrieve the bone name from the body index
			if (Bodies.IsValidIndex(OverlapInfo.OverlapInfo.Item))
			{
				AddFingerVibration(Bodies[OverlapInfo.OverlapInfo.Item]->BodySetup->BoneName);
			}
		}
	}

	if (FManusModule::Get().IsActive())
	{
		// Vibrate
		if (MotionCaptureType == EManusMotionCaptureType::LeftHand
			|| MotionCaptureType == EManusMotionCaptureType::BothHands
			|| MotionCaptureType == EManusMotionCaptureType::FullBody
			) {
			UManusBlueprintLibrary::VibrateFingers(
				EManusHandType::Left,
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Thumb],
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Index],
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Middle],
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Ring],
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Pinky]);
		}
		if (MotionCaptureType == EManusMotionCaptureType::RightHand
			|| MotionCaptureType == EManusMotionCaptureType::BothHands
			|| MotionCaptureType == EManusMotionCaptureType::FullBody
			) {
			UManusBlueprintLibrary::VibrateFingers(
				EManusHandType::Right,
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Thumb],
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Index],
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Middle],
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Ring],
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Pinky]);
		}
	}

	// Reset vibrate powers
	for (int i = 0; i < (int)EManusFingerName::Max; i++)
	{
		LeftHandFingersCollisionVibratePowers[i] = 0.0f;
		RightHandFingersCollisionVibratePowers[i] = 0.0f;
	}
}

void UManusComponent::TickGestureDetection()
{
	int ManusLiveLinkUserIndex = FManusModule::Get().GetManusLiveLinkUserIndex(ManusDashboardUserIndex, ManusSkeleton);
	if (ManusLiveLinkUserIndex != INDEX_NONE)
	{
		FManusLiveLinkUser& ManusLiveLinkUser = FManusModule::Get().GetManusLiveLinkUser(ManusLiveLinkUserIndex);

		const TArray<FHandGestureDescriptor>& HandGestureDescriptors = GetDefault<UManusSettings>()->HandGestureDescriptors;
		for (int HandType = 0; HandType < (int)EManusHandType::Max; HandType++)
		{
			if (MotionCaptureType == EManusMotionCaptureType::BothHands
				|| MotionCaptureType == EManusMotionCaptureType::FullBody
				|| (MotionCaptureType == EManusMotionCaptureType::LeftHand && (EManusHandType)HandType == EManusHandType::Left)
				|| (MotionCaptureType == EManusMotionCaptureType::RightHand && (EManusHandType)HandType == EManusHandType::Right)
				) {
				ManusComponentHandLiveData[HandType].IsGestureOnGoing.SetNum(HandGestureDescriptors.Num());

				const TArray<float>& DetectedHandGestureTimers = ManusLiveLinkUser.HandGestureDetectionData[HandType].ManusLiveLinkUserDetectedHandGestureTimers;
				for (int GestureIndex = 0; GestureIndex < HandGestureDescriptors.Num() && GestureIndex < DetectedHandGestureTimers.Num(); GestureIndex++)
				{
					if (DetectedHandGestureTimers[GestureIndex] > 0.0f)
					{
						if (!ManusComponentHandLiveData[HandType].IsGestureOnGoing[GestureIndex])
						{
							// Gesture started delegate
							OnGestureStarted.Broadcast((EManusHandType)HandType, HandGestureDescriptors[GestureIndex].GestureName);
						}

						// Gesture on-going delegate
						OnGestureOnGoing.Broadcast((EManusHandType)HandType, HandGestureDescriptors[GestureIndex].GestureName, DetectedHandGestureTimers[GestureIndex]);

						// Update on-going gesture status
						ManusComponentHandLiveData[HandType].IsGestureOnGoing[GestureIndex] = true;
					}
					else if (ManusComponentHandLiveData[HandType].IsGestureOnGoing[GestureIndex])
					{
						// Update on-going gesture status
						ManusComponentHandLiveData[HandType].IsGestureOnGoing[GestureIndex] = false;

						// Gesture finished delegate
						OnGestureFinished.Broadcast((EManusHandType)HandType, HandGestureDescriptors[GestureIndex].GestureName);
					}
				}
			}
		}
	}
}

void UManusComponent::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Update vibrate powers according to current overlaps (used when the body is set to Block)
	if (bFingerHaptics && (OtherActor != NULL) && (OtherComp != NULL) && !OtherComp->IsAttachedTo(this))
	{
		AddFingerVibration(Hit.MyBoneName);
	}
}

void UManusComponent::AddFingerVibration(FName BoneName)
{
	if (ManusSkeleton)
	{
		if (MotionCaptureType == EManusMotionCaptureType::LeftHand
			|| MotionCaptureType == EManusMotionCaptureType::BothHands
			|| MotionCaptureType == EManusMotionCaptureType::FullBody
			) {
			if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandThumb1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandThumb2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandThumb3]))
			{
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Thumb] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandIndex1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandIndex2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandIndex3]))
			{
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Index] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandMiddle1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandMiddle2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandMiddle3]))
			{
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Middle] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandRing1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandRing2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandRing3]))
			{
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Ring] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandPinky1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandPinky2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandPinky3]))
			{
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Pinky] += 1.0f / (int)EManusPhalangeName::Max;
			}
		}
		if (MotionCaptureType == EManusMotionCaptureType::RightHand
			|| MotionCaptureType == EManusMotionCaptureType::BothHands
			|| MotionCaptureType == EManusMotionCaptureType::FullBody
			) {
			if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandThumb1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandThumb2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandThumb3]))
			{
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Thumb] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandIndex1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandIndex2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandIndex3]))
			{
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Index] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandMiddle1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandMiddle2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandMiddle3]))
			{
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Middle] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandRing1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandRing2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandRing3]))
			{
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Ring] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandPinky1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandPinky2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandPinky3]))
			{
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Pinky] += 1.0f / (int)EManusPhalangeName::Max;
			}
		}
	}
}

bool UManusComponent::ComponentOverlapMultiImpl(TArray<struct FOverlapResult>& OutOverlaps, const UWorld* World, const FVector& Pos, const FQuat& Quat, ECollisionChannel TestChannel, const struct FComponentQueryParams& Params, const struct FCollisionObjectQueryParams& ObjectQueryParams) const
{
	OutOverlaps.Reset();

	if (!Bodies.IsValidIndex(RootBodyData.BodyIndex))
	{
		return false;
	}

	const FTransform WorldToComponent(GetComponentTransform().Inverse());
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
// 

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
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
				ManusReplicatorId = PlayerController->PlayerState->GetPlayerId();
#else
				ManusReplicatorId = PlayerController->PlayerState->PlayerId;
#endif
			}
		}
		if (ManusReplicatorId == 0)
		{
			APawn* Pawn = Cast<APawn>(GetOwner());
			if (Pawn && Pawn->GetPlayerState())
			{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
				ManusReplicatorId = Pawn->GetPlayerState()->GetPlayerId();
#else
				ManusReplicatorId = Pawn->GetPlayerState()->PlayerId;
#endif
			}
			else
			{
				APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
				if (LocalPlayerController && LocalPlayerController->PlayerState)
				{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
					ManusReplicatorId = LocalPlayerController->PlayerState->GetPlayerId();
#else
					ManusReplicatorId = LocalPlayerController->PlayerState->PlayerId;
#endif
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
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
		s += FString::FromInt(Pawn->GetPlayerState()->GetPlayerId());
#else
		s += FString::FromInt(Pawn->GetPlayerState()->PlayerId);
#endif
	}

	return s;
}

int64 UManusComponent::GetManusGloveID(EManusHandType HandType) const
{
	int64 GloveId = 0;
	UManusBlueprintLibrary::GetManusDashboardUserGloveId(ManusDashboardUserIndex, HandType, GloveId);
	return GloveId;
}

EManusRet UManusComponent::VibrateManusGloveHand(EManusHandType HandType, float Power /*= 0.6f*/, int32 Milliseconds /*= 300*/)
{
	return UManusBlueprintLibrary::VibrateGlove(GetManusGloveID(HandType), Power, Milliseconds);
}

EManusRet UManusComponent::VibrateManusGloveFingers(EManusHandType HandType, float ThumbPower /*= 1.0f*/, float IndexPower /*= 1.0f*/, float MiddlePower /*= 1.0f*/, float RingPower /*= 1.0f*/, float PinkyPower /*= 1.0f*/)
{
	return UManusBlueprintLibrary::VibrateFingers(HandType, ThumbPower, IndexPower, MiddlePower, RingPower, PinkyPower);
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
	if (ManusSkeleton && SkeletalMesh != ManusSkeleton->SkeletalMesh)
	{
		SetSkeletalMesh(ManusSkeleton->SkeletalMesh);
	}
}

#if WITH_EDITOR
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
void UManusComponent::PreEditChange(FProperty* PropertyAboutToChange)
#else
void UManusComponent::PreEditChange(UProperty* PropertyAboutToChange)
#endif
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange)
	{
		const FName PropertyName = PropertyAboutToChange->GetFName();

		// When the Manus Live Link User index is about to change
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UManusComponent, ManusDashboardUserIndex) || PropertyName == GET_MEMBER_NAME_CHECKED(UManusComponent, ManusSkeleton))
		{
			// We are switching user, update which user's data we're using
			FManusModule::Get().RemoveObjectUsingManusLiveLinkUser(ManusDashboardUserIndex, ManusSkeleton, this);
		}
	}
}

void UManusComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// When the Manus Live Link User index changed
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UManusComponent, ManusDashboardUserIndex) || PropertyName == GET_MEMBER_NAME_CHECKED(UManusComponent, ManusSkeleton))
	{
		// We switched user, update which user's data we're using
		FManusModule::Get().AddObjectUsingManusLiveLinkUser(ManusDashboardUserIndex, ManusSkeleton, this);

		// Use the skeleton of the new Manus Live Link User
		RefreshManusSkeleton();
	}

	// Make sure negative scale is only used for mirroring the hand
	FVector ComponentScale = GetComponentScale();
	if (ComponentScale.X < 0.0f || ComponentScale.Z < 0.0f
	|| (!bMirrorHandBone && ComponentScale.Y < 0.0f)
	|| (bMirrorHandBone && ComponentScale.Y > 0.0f)
	) {
		UE_LOG(LogManus, Warning, TEXT("Manus Component: Negative scale is reserved exclusively for mirroring the Skeletal Mesh of the Manus component. Scale has been fixed."));

		ComponentScale.X = FMath::Abs(ComponentScale.X);
		ComponentScale.Y = FMath::Abs(ComponentScale.Y) * (bMirrorHandBone ? -1.0f : 1.0f);
		ComponentScale.Z = FMath::Abs(ComponentScale.Z);
		SetWorldScale3D(ComponentScale);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif //WITH_EDITOR
