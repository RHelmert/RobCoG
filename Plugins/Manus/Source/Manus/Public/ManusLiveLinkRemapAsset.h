// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Set up a Doxygen group.
/** @addtogroup UManusLiveLinkRemapAsset
 *  @{
 */

#include "AnimNode_ManusLiveLinkPose.h"
#include "ManusBlueprintTypes.h"
#include "BonePose.h"
#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "BoneIndices.h"
#include "Animation/AnimTypes.h"
#include "CustomBoneIndexArray.h"
#include "Animation/AnimStats.h"
#include "Misc/Base64.h"
#include "Animation/Skeleton.h"
#include "BoneContainer.h"
#include "ManusLiveLinkRemapAsset.generated.h"

/// @brief Manus LiveLink remapping asset
UCLASS(Blueprintable)
class UManusLiveLinkRemapAsset : public ULiveLinkRetargetAsset
{
	GENERATED_UCLASS_BODY()

public:
	virtual void BuildPoseFromAnimationData(float DeltaTime, const FLiveLinkSkeletonStaticData* InSkeletonData, const FLiveLinkAnimationFrameData* InFrameData, FCompactPose& OutPose) override;
	virtual void BuildHandPoseFromAnimationData(EManusHandType HandType, const FLiveLinkSkeletonStaticData* InSkeletonData, const FLiveLinkAnimationFrameData* InFrameData, FCompactPose& OutPose);

public:
    /// @brief Whether we should animate 
	UPROPERTY(Transient)
	bool bShouldAnimate;

    /// @brief Manus Live Link node 
	UPROPERTY(Transient)
	FAnimNode_ManusLiveLinkPose ManusLiveLinkNode;

    /// @brief Current scale of the Skeletal Mesh 
	UPROPERTY(Transient)
	FVector SkeletalMeshScale;

    /// @brief Skeletal Mesh local transform in Actor space 
	UPROPERTY(Transient)
	FTransform SkeletalMeshActorSpaceTransform;

};


// Close the Doxygen group.
/** @} */
