// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#pragma once

#include "CoreTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimTypes.h"

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 26

#include "BoneContainer.h"
#include "Interfaces/Interface_BoneReferenceSkeletonProvider.h"

#endif

#include "ManusBlueprintTypes.h"
#include "ManusSkeleton.generated.h"

#if ENGINE_MAJOR_VERSION == 5 ||ENGINE_MINOR_VERSION >= 26
typedef FBoneReference BoneName_t;
#define GET_BONE_NAME(Bone) Bone.BoneName
#else
typedef FName BoneName_t;
#define GET_BONE_NAME(Bone) Bone
#endif

UCLASS(BlueprintType, hidecategories = (Object))
class MANUS_API UManusSkeleton : public UObject, public IBoneReferenceSkeletonProvider
{
	GENERATED_BODY()

public:
	UManusSkeleton();

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 26
	virtual void PostLoad() override;
#endif
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR

public:
	class USkeleton* GetSkeleton();

#if ENGINE_MAJOR_VERSION == 5 ||ENGINE_MINOR_VERSION >= 26
	virtual class USkeleton* GetSkeleton(bool& bInvalidSkeletonIsError) override { bInvalidSkeletonIsError = false; return GetSkeleton(); }
#endif
	void OnManusSkeletonChanged();
	void OnRetargetingSettingsChanged();

public:
	/** The skeletal mesh to use with Manus. */
	UPROPERTY(EditAnywhere, Category = "Manus | Skeleton")
	class USkeletalMesh* SkeletalMesh;

	/* TODO: Remove in a future version. Kept to convert previously saved asset. */
	UPROPERTY()
	FName BoneNameMap_DEPRECATED[(int)EManusBoneName::Max];

	/** The skeleton bones to use with Manus. */
	UPROPERTY(EditAnywhere, Category = "Manus | Skeleton")
	FBoneReference BoneMap[(int)EManusBoneName::Max];

	/** The hands animation setup. */
	UPROPERTY(EditAnywhere, Category = "Manus | Hand Tracking")
	FManusHandAnimationSetup HandsAnimationSetup[(int)EManusHandType::Max];

	/** The transforms that define the position of the Manus Glove relative to its Tracking Device (X-axis is forward, Z-axis is up). */
	UPROPERTY(EditAnywhere, Category = "Manus | Hand Tracking")
	FQuat TrackingDeviceToManusGloveTransform = FQuat::MakeFromEuler(FVector(90.0f, 0.0f, 180.0f));

#if WITH_EDITORONLY_DATA
	/** Fingers rotations extents preview settings. */
	UPROPERTY(EditAnywhere, Category = "Manus | Hand Tracking")
	FManusHandPreviewSettings FingersRotationsExtentsPreviewSettings;
#endif // WITH_EDITORONLY_DATA

	/** Whether this Manus skeleton is used for Polygon full body tracking technology. */
	UPROPERTY(EditAnywhere, Category = "Manus | Full Body Tracking", meta = (DisplayName = "Used for Polygon full body tracking"))
	bool bIsUsedForFullBodyTracking;
	
	/** Option to scale the skeleton to the user, turn off to keep the original sizes */
	UPROPERTY(EditAnywhere, Category = "Manus | Full Body Tracking", meta = (DisplayName = "Scale skeleton to user"))
	bool ScaleToUser;

	UPROPERTY(EditAnywhere, Category = "Manus | Full Body Tracking", meta = (DisplayName = "Retargeting Target"))
	EManusRetargetingTarget RetargetingTarget = EManusRetargetingTarget::BodyEstimation;

	UPROPERTY(EditAnywhere, Category = "Manus | Full Body Tracking", meta = (DisplayName = "Target Skeleton Name"))
	FString TargetName = "";

	/** The local axis of the bones that can be scaled to stretch the full body skeleton. */
	UPROPERTY(EditAnywhere, Category = "Manus | Full Body Tracking")
	TEnumAsByte<EBoneAxis> FullBodyStretchAxis;

	/** The full body skeleton height in meters. */
	UPROPERTY(EditAnywhere, Category = "Manus | Full Body Tracking")
	float FullBodyHeight;

	UPROPERTY(EditAnywhere, Category = "Manus | Full Body Tracking")
	FManusPolygonParameters RetargetingParameters;

	/** The map that maps Manus bone indices to their skeleton bone indices counterparts. */
	UPROPERTY(Transient)
	TMap<int, int> BoneIndexMap;

	/** The skeleton bones orientations that Manus Core is using internally. */
	UPROPERTY(Transient)
	TMap<int, FQuat> ManusInternalOrientations;

	/** The skeleton bones delta orientations between the reference pose orientation and the Manus Core internal orientation. */
	UPROPERTY(Transient)
	TMap<int, FQuat> ManusInternalDeltaOrientations;

	/** The skeleton bones initial scales. */
	UPROPERTY(Transient)
	TMap<int, FVector> InitialScales;
};
