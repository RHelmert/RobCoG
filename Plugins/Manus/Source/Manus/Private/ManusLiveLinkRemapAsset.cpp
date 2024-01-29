// Copyright Epic Games, Inc. All Rights Reserved.

#include "ManusLiveLinkRemapAsset.h"
#include "Manus.h"
#include "ManusSkeleton.h"
#include "ManusBlueprintLibrary.h"
#include "AnimNode_ManusLiveLinkPose.h"
#include "BonePose.h"
#include "Roles/LiveLinkAnimationTypes.h"

DECLARE_CYCLE_STAT(TEXT("Manus Build Pose From Animation Data"), STAT_Manus_BuildPoseFromAnimationData, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("Manus Build Hand Pose From Animation Data"), STAT_Manus_BuildHandPoseFromAnimationData, STATGROUP_Manus);

#define LOCTEXT_NAMESPACE "FManusModule"

UManusLiveLinkRemapAsset::UManusLiveLinkRemapAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UManusLiveLinkRemapAsset::BuildPoseFromAnimationData(float DeltaTime, const FLiveLinkSkeletonStaticData* InSkeletonData, const FLiveLinkAnimationFrameData* InFrameData, FCompactPose& OutPose)
{
	SCOPE_CYCLE_COUNTER(STAT_Manus_BuildPoseFromAnimationData);

	if (bShouldAnimate)
	{
		BuildHandPoseFromAnimationData(EManusHandType::Right, InSkeletonData, InFrameData, OutPose); // TODO handtype and optimize.
	}
}

void UManusLiveLinkRemapAsset::BuildHandPoseFromAnimationData(EManusHandType HandType, const FLiveLinkSkeletonStaticData* InSkeletonData, const FLiveLinkAnimationFrameData* InFrameData, FCompactPose& OutPose)
{
	SCOPE_CYCLE_COUNTER(STAT_Manus_BuildHandPoseFromAnimationData);

	for (int32 BoneIndex = 0; BoneIndex < InFrameData->Transforms.Num(); ++BoneIndex)
	{
		FTransform BoneTransform = InFrameData->Transforms[BoneIndex];

		// Do not touch the bone if there was no valid data from Manus
		if (!BoneTransform.GetScale3D().IsZero())
		{
			FCompactPoseBoneIndex CPIndex = OutPose.GetBoneContainer().MakeCompactPoseIndex(FMeshPoseBoneIndex(BoneIndex)); // its still the same as boneindex but in a format that makes no sense.
			if (CPIndex != INDEX_NONE)
			{
                OutPose[CPIndex] = BoneTransform;
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
