// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

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

/**
  * Manus LiveLink remapping asset
  */
UCLASS(Blueprintable)
class UManusLiveLinkRemapAsset : public ULiveLinkRetargetAsset
{
	GENERATED_UCLASS_BODY()

public:
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
	virtual void BuildPoseFromAnimationData(float DeltaTime, const FLiveLinkSkeletonStaticData* InSkeletonData, const FLiveLinkAnimationFrameData* InFrameData, FCompactPose& OutPose) override;
	virtual void BuildHandPoseFromAnimationData(EManusHandType HandType, const FLiveLinkSkeletonStaticData* InSkeletonData, const FLiveLinkAnimationFrameData* InFrameData, FCompactPose& OutPose);
#else
	virtual void BuildPoseForSubject(float DeltaTime, const FLiveLinkSubjectFrame& InFrame, FCompactPose& OutPose, FBlendedCurve& OutCurve) override;
	virtual void BuildHandPoseForSubject(EManusHandType HandType, const FLiveLinkSubjectFrame& InFrame, FCompactPose& OutPose, FBlendedCurve& OutCurve);
#endif

public:
	/** Whether we should animate */
	UPROPERTY(Transient)
	bool bShouldAnimate;

	/** Manus Live Link node */
	UPROPERTY(Transient)
	FAnimNode_ManusLiveLinkPose ManusLiveLinkNode;

	/** Motion capture type */
	UPROPERTY(Transient)
	EManusMotionCaptureType MotionCaptureType;

	/** Whether to mirror hand bone (useful when you use the same Skeletal Mesh for both hands) */
	UPROPERTY(Transient)
	bool bMirrorHandBone;

	/** Current scale of the Skeletal Mesh */
	UPROPERTY(Transient)
	FVector SkeletalMeshScale;

	/** Skeletal Mesh local transform in Actor space */
	UPROPERTY(Transient)
	FTransform SkeletalMeshActorSpaceTransform;

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION <= 25
private:
	/** Copied this method from UE4.26 (where it got introduced), this method is needed to convert transforms to local when a skeleton is using intermediate bones */
	void ConvertComponentPosesToLocalPoses(FCSPose<FCompactPose>& InPose, FCompactPose& OutPose);
#endif
};
