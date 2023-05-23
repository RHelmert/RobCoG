// Copyright 2015-2020 Manus

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
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 24
	SetReplicatingMovement(false);
#else
	SetReplicateMovement(false);
#endif

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
#else
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
#endif
}

void AManusReplicator::BeginPlay()
{
	Super::BeginPlay();

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
	if (IsLiveLinkSourceLocal())
	{
		// Add LiveLinkTicked callback
		TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
		if (ManusLocalLiveLinkSource.IsValid() && ManusLocalLiveLinkSource->GetLiveLinkClient())
		{
			ManusLocalLiveLinkSource->GetLiveLinkClient()->OnLiveLinkTicked().AddUObject(this, &AManusReplicator::OnLiveLinkTicked);
		}
	}
#endif
}

void AManusReplicator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLiveLinkSourceLocal())
	{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
		// Remove LiveLinkTicked callback
		TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
		if (ManusLocalLiveLinkSource.IsValid() && ManusLocalLiveLinkSource->GetLiveLinkClient())
		{
			ManusLocalLiveLinkSource->GetLiveLinkClient()->OnLiveLinkTicked().RemoveAll(this);
		}
#endif
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

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
void AManusReplicator::OnLiveLinkTicked()
#else
void AManusReplicator::Tick(float DeltaTime)
#endif
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 25
	Super::Tick(DeltaTime);

	if (IsLiveLinkSourceLocal())
	{
#endif

		FManusReplicatedData DataToReplicate;

		TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
		if (ManusLocalLiveLinkSource.IsValid())
		{
			DataToReplicate.ReplicatedFrameDataArray = ManusLocalLiveLinkSource->ReplicatedFrameDataArray;

			if (DataToReplicate.ReplicatedFrameDataArray.Num() > 0)
			{
				SendReplicatedDataToServer(DataToReplicate);
			}
		}

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 25
	}
#endif
}

bool AManusReplicator::SendReplicatedDataToServer_Validate(FManusReplicatedData DataToReplicate)
{
	return true;
}

void AManusReplicator::SendReplicatedDataToServer_Implementation(FManusReplicatedData DataToReplicate)
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

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
void AManusReplicator::CompressReplicatedFrameData(const FLiveLinkAnimationFrameData& UncompressedFrameData, FManusReplicatedFrameData& CompressedFrameData)
#else
void AManusReplicator::CompressReplicatedFrameData(const FLiveLinkFrameData& UncompressedFrameData, FManusReplicatedFrameData& CompressedFrameData)
#endif
{
	// Reset arrays
	CompressedFrameData.LeftHandFingerAngles.Reset();
	CompressedFrameData.RightHandFingerAngles.Reset();
	CompressedFrameData.PolygonTransforms.Reset();

	// Prepare going through bones data
	const int HandFingerTransformNum = HAND_LIVE_LINK_BONE_NUM - 1; // Don't count the Hand bone

	const int FirstLeftGloveBoneIndex = (int)EManusBoneName::LeftHandThumb1;
	const int LastLeftGloveBoneIndex = FirstLeftGloveBoneIndex + HandFingerTransformNum - 1;
	bool HasLeftFingerTransforms = (UncompressedFrameData.Transforms[FirstLeftGloveBoneIndex].GetScale3D() != FVector::ZeroVector);

	const int FirstRightGloveBoneIndex = (int)EManusBoneName::RightHandThumb1;
	const int LastRightGloveBoneIndex = FirstRightGloveBoneIndex + HandFingerTransformNum - 1;
	bool HasRightFingerTransforms = (UncompressedFrameData.Transforms[FirstRightGloveBoneIndex].GetScale3D() != FVector::ZeroVector);

	const int PolygonTransformNum = BODY_LIVE_LINK_BONE_NUM;
	const int FirstPolygonBoneIndex = (int)EManusBoneName::Root;
	const int LastPolygonBoneIndex = FirstPolygonBoneIndex + PolygonTransformNum - 1;
	bool HasPolygonTransforms = (UncompressedFrameData.Transforms[FirstPolygonBoneIndex].GetScale3D() != FVector::ZeroVector);

	// Left hand tracker
	CompressedFrameData.LeftHandTransform = UncompressedFrameData.Transforms[(int)EManusBoneName::LeftHand];
	CompressedFrameData.LeftHandTrackerTransform = UncompressedFrameData.Transforms[(int)EManusBoneName::LeftHandTracker];

	// Left hand fingers data (only has the rotations)
	if (HasLeftFingerTransforms)
	{
		for (int i = 0; i < HandFingerTransformNum; i++)
		{
			const int BoneIndex = FirstLeftGloveBoneIndex + i;
			CompressedFrameData.LeftHandFingerAngles.Add(UncompressedFrameData.Transforms[BoneIndex].GetRotation());
		}
	}

	// Right hand
	CompressedFrameData.RightHandTransform = UncompressedFrameData.Transforms[(int)EManusBoneName::RightHand];
	CompressedFrameData.RightHandTrackerTransform = UncompressedFrameData.Transforms[(int)EManusBoneName::RightHandTracker];

	// Right hand fingers data (only has the rotations)
	if (HasRightFingerTransforms)
	{
		for (int i = 0; i < HandFingerTransformNum; i++)
		{
			const int BoneIndex = FirstRightGloveBoneIndex + i;
			CompressedFrameData.RightHandFingerAngles.Add(UncompressedFrameData.Transforms[BoneIndex].GetRotation());
		}
	}

	// Polygon data
	if (HasPolygonTransforms)
	{
		for (int i = 0; i < PolygonTransformNum; i++)
		{
			const int BoneIndex = FirstPolygonBoneIndex + i;
			CompressedFrameData.PolygonTransforms.Add(UncompressedFrameData.Transforms[BoneIndex]);
		}
	}
}

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
void AManusReplicator::DecompressReplicatedFrameData(FLiveLinkAnimationFrameData& UncompressedFrameData, const FManusReplicatedFrameData& CompressedFrameData)
#else
void AManusReplicator::DecompressReplicatedFrameData(FLiveLinkFrameData& UncompressedFrameData, const FManusReplicatedFrameData& CompressedFrameData)
#endif
{
	// Prepare going through bones data
	const int HandFingerTransformNum = HAND_LIVE_LINK_BONE_NUM - 1; // Don't count the Hand bone

	const int FirstLeftGloveBoneIndex = (int)EManusBoneName::LeftHandThumb1;
	const int LastLeftGloveBoneIndex = FirstLeftGloveBoneIndex + HandFingerTransformNum - 1;
	bool HasLeftFingerTransforms = (CompressedFrameData.LeftHandFingerAngles.Num() > 0);

	const int FirstRightGloveBoneIndex = (int)EManusBoneName::RightHandThumb1;
	const int LastRightGloveBoneIndex = FirstRightGloveBoneIndex + HandFingerTransformNum - 1;
	bool HasRightFingerTransforms = (CompressedFrameData.RightHandFingerAngles.Num() > 0);

	const int PolygonTransformNum = BODY_LIVE_LINK_BONE_NUM;
	const int FirstPolygonBoneIndex = (int)EManusBoneName::Root;
	const int LastPolygonBoneIndex = FirstPolygonBoneIndex + PolygonTransformNum - 1;
	bool HasPolygonTransforms = (CompressedFrameData.PolygonTransforms.Num() > 0);

	// Left hand
	UncompressedFrameData.Transforms[(int)EManusBoneName::LeftHand] = CompressedFrameData.LeftHandTransform;
	UncompressedFrameData.Transforms[(int)EManusBoneName::LeftHandTracker] = CompressedFrameData.LeftHandTrackerTransform;

	// Left hand fingers data (only has the rotations)
	for (int i = 0; i < HandFingerTransformNum; i++)
	{
		const int BoneIndex = FirstLeftGloveBoneIndex + i;
		UncompressedFrameData.Transforms[BoneIndex].SetIdentity();
		if (HasLeftFingerTransforms)
		{
			UncompressedFrameData.Transforms[BoneIndex].SetRotation(CompressedFrameData.LeftHandFingerAngles[i]);
		}
		else
		{
			UncompressedFrameData.Transforms[BoneIndex].SetScale3D(FVector::ZeroVector);
		}
	}

	// Right hand
	UncompressedFrameData.Transforms[(int)EManusBoneName::RightHand] = CompressedFrameData.RightHandTransform;
	UncompressedFrameData.Transforms[(int)EManusBoneName::RightHandTracker] = CompressedFrameData.RightHandTrackerTransform;

	// Right hand fingers data (only has the rotations)
	for (int i = 0; i < HandFingerTransformNum; i++)
	{
		const int BoneIndex = FirstRightGloveBoneIndex + i;
		UncompressedFrameData.Transforms[BoneIndex].SetIdentity();
		if (HasRightFingerTransforms)
		{
			UncompressedFrameData.Transforms[BoneIndex].SetRotation(CompressedFrameData.RightHandFingerAngles[i]);
		}
		else
		{
			UncompressedFrameData.Transforms[BoneIndex].SetScale3D(FVector::ZeroVector);
		}
	}

	// Polygon data
	for (int i = 0; i < PolygonTransformNum; i++)
	{
		const int BoneIndex = FirstPolygonBoneIndex + i;
		if (HasPolygonTransforms)
		{
			UncompressedFrameData.Transforms[BoneIndex] = CompressedFrameData.PolygonTransforms[i];
		}
		else if (i != (int)EManusBoneName::LeftHand && i != (int)EManusBoneName::RightHand)
		{
			UncompressedFrameData.Transforms[BoneIndex].SetIdentity();
			UncompressedFrameData.Transforms[BoneIndex].SetScale3D(FVector::ZeroVector);
		}
	}
}
