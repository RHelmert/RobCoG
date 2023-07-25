// Copyright 2015-2022 Manus

#include "ManusReplicator.h"
#include "Manus.h"

#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "ILiveLinkClient.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"


AManusReplicator::AManusReplicator()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicatingMovement(false); // is this ok? yes we handle animation. but movement is not quite the same.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AManusReplicator::BeginPlay()
{
	Super::BeginPlay();

	if (IsLiveLinkSourceLocal())
	{
		// Add LiveLinkTicked callback
		TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
		if (ManusLocalLiveLinkSource.IsValid() && ManusLocalLiveLinkSource->GetLiveLinkClient())
		{
			ManusLocalLiveLinkSource->GetLiveLinkClient()->OnLiveLinkTicked().AddUObject(this, &AManusReplicator::OnLiveLinkTicked);
		}
	}
    else
    {
        TSharedPtr<FManusLiveLinkSource> ManusReplicatedLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Replicated));
        if (ManusReplicatedLiveLinkSource.IsValid() && ManusReplicatedLiveLinkSource->GetLiveLinkClient())
        {
            ManusReplicatedLiveLinkSource->GetLiveLinkClient()->OnLiveLinkTicked().AddUObject(this, &AManusReplicator::OnLiveLinkTicked);
        }
    }
}

void AManusReplicator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLiveLinkSourceLocal())
	{
		TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
		if (ManusLocalLiveLinkSource.IsValid() && ManusLocalLiveLinkSource->GetLiveLinkClient())
		{
			ManusLocalLiveLinkSource->GetLiveLinkClient()->OnLiveLinkTicked().RemoveAll(this);
		}
	}
	else
	{
		// Stop replicating
		TSharedPtr<FManusLiveLinkSource> ManusReplicatedLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Replicated));
		if (ManusReplicatedLiveLinkSource.IsValid())
		{
			ManusReplicatedLiveLinkSource->StopReplicatingLiveLink(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool AManusReplicator::IsLiveLinkSourceLocal()
{
	if (GetOwner() == GEngine->GetFirstLocalPlayerController(GetWorld()))
	{
		return true;
	}
	return false;
}

void AManusReplicator::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AManusReplicator, ReplicatorId, COND_InitialOnly);
	DOREPLIFETIME(AManusReplicator, ReplicatedData);
}

void AManusReplicator::OnLiveLinkTicked()
{
	FManusReplicatedData DataToReplicate;
    
    if (HasAuthority())
    {
        TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
        if (ManusLocalLiveLinkSource.IsValid())
        {
            DataToReplicate.ReplicatedFrameDataArray = ManusLocalLiveLinkSource->m_ReplicatedFrameDataArray;

            if (DataToReplicate.ReplicatedFrameDataArray.Num() > 0)
            {
                SendReplicatedDataToServer(DataToReplicate);
            }
        }
    }
}

bool AManusReplicator::SendReplicatedDataToServer_Validate(FManusReplicatedData DataToReplicate) // unreal required function
{
	return true;
}

void AManusReplicator::SendReplicatedDataToServer_Implementation(FManusReplicatedData DataToReplicate) // unreal required function
{
	ReplicatedData = DataToReplicate;

	// Call OnReplicatedDataReceivedFromServer manually here, because OnReplicatedDataReceivedFromServer won't be called on a listen server
	OnReplicatedDataReceivedFromServer();
}

void AManusReplicator::OnReplicatedDataReceivedFromServer()
{
	TSharedPtr<FManusLiveLinkSource> ManusReplicatedLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Replicated));
	if (ManusReplicatedLiveLinkSource.IsValid())
	{
		ManusReplicatedLiveLinkSource->ReplicateLiveLink(this);
	}
}

void AManusReplicator::CompressReplicatedFrameData(const FLiveLinkAnimationFrameData& UncompressedFrameData, FManusReplicatedFrameData& CompressedFrameData)
{
	// Reset array
	CompressedFrameData.MayoTransforms.Reset();

	for (int i = 0; i < UncompressedFrameData.Transforms.Num(); i++)
	{
		CompressedFrameData.MayoTransforms.Add(UncompressedFrameData.Transforms[i]);
	}
}

void AManusReplicator::DecompressReplicatedFrameData(FLiveLinkAnimationFrameData& UncompressedFrameData, const FManusReplicatedFrameData& CompressedFrameData)
{
	UncompressedFrameData.Transforms.Reset();
	for (int i = 0; i < CompressedFrameData.MayoTransforms.Num(); i++)
	{
		{
			UncompressedFrameData.Transforms.Add( CompressedFrameData.MayoTransforms[i]);
		}
	}
}
