// Copyright 2015-2020 Manus

#pragma once

#include "ManusBlueprintTypes.h"
#include "Animation/AnimTypes.h"
#include "BoneContainer.h"
#include "ManusSkeleton.h"

struct MANUS_API ManusTools
{
	static int64 GenerateManusIdFromManusLiveLinkUser(int ManusLiveLinkUserIndex);

#if WITH_EDITORONLY_DATA
	static void AutomaticSkeletonBoneNameMapping(class USkeleton* Skeleton, BoneName_t* BoneMap);
	static EBoneAxis AutomaticSkeletonStretchAxisDetection(class USkeleton* Skeleton);
	static float AutomaticSkeletonHeightDetection(class USkeleton* Skeleton);
	static void AutomaticFingersRotationAxesDetection(class UManusSkeleton* ManusSkeleton, EManusHandType HandType, EManusAxisOption& OutStretchAxis, EManusAxisOption& OutSpreadAxis);
#endif // WITH_EDITORONLY_DATA

	static bool CalculateManusInternalOrientations(int ManusLiveLinkUserIndex, TMap<int, FQuat>& OutOrientations, TMap<int, FQuat>& OutDeltaOrientations);
	static bool CalculateManusInternalOrientation(EManusBoneName Bone, class USkeleton* Skeleton, const BoneName_t* BoneMap, FQuat& OutOrientation, FQuat& OutDeltaOrientation);
	static FVector CalculateManusInternalForward(TArray<TTuple<FTransform, FTransform>> Directions);
	static FQuat LookRotation(FVector LookAtDirection, FVector UpDirection);
	static FQuat AngleAxis(float Angle, EManusAxisOption Axis);
};
