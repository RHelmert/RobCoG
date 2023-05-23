// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#include "AnimNode_ManusLiveLinkPose.h"
#include "Manus.h"
#include "ManusBlueprintTypes.h"
#include "ManusBlueprintLibrary.h"
#include "ManusSettings.h"
#include "ManusSkeleton.h"
#include "ManusComponent.h"
#include "ManusLiveLinkSource.h"
#include "Animation/AnimInstance.h"
#include "Animation/Skeleton.h"
#include "ManusLiveLinkRemapAsset.h"
#if WITH_EDITOR
#include "ManusEditorUserSettings.h"
#endif // WITH_EDITOR

DECLARE_CYCLE_STAT(TEXT("Manus Anim Node Pre Update"), STAT_Manus_AnimNodePreUpdate, STATGROUP_Manus);


FAnimNode_ManusLiveLinkPose::FAnimNode_ManusLiveLinkPose()
{
	// Default values
	ManusDashboardUserIndex = 0;
	MotionCaptureType = EManusMotionCaptureType::LeftHand;

	// Init Live Link subject name
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
	LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(0);
#else
	SubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(0);
#endif

	// Init retarget asset
	RetargetAsset = UManusLiveLinkRemapAsset::StaticClass();

	// Init bone map
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EManusBoneName"), true);
	check(EnumPtr != nullptr);
	for (int i = 0; i < (int)EManusBoneName::Max; i++)
	{
		FName BoneName = FName(*EnumPtr->GetNameStringByIndex(i));
		BoneNameToEnum.Add(BoneName, (EManusBoneName)i);
	}

	// Init tracking device delta transforms
	for (int i = 0; i < (int)EManusHandType::Max; i++)
	{
		TrackingDeviceDeltaTransform[i].SetIdentity();
	}
}

void FAnimNode_ManusLiveLinkPose::PreUpdate(const UAnimInstance* InAnimInstance)
{
	Super::PreUpdate(InAnimInstance);

	SCOPE_CYCLE_COUNTER(STAT_Manus_AnimNodePreUpdate);

	UManusLiveLinkRemapAsset* ManusLiveLinkRemapAsset = Cast<UManusLiveLinkRemapAsset>(CurrentRetargetAsset);

	// Assign node to retarget asset
	if (ManusLiveLinkRemapAsset)
	{
		ManusLiveLinkRemapAsset->ManusLiveLinkNode = *this;

		// Manus component
		UManusComponent* ManusComponent = Cast<UManusComponent>(InAnimInstance->GetOwningComponent());

		// Determine whether we should animate
		ManusLiveLinkRemapAsset->bShouldAnimate = false;
		int ManusLiveLinkUserIndex = FManusModule::Get().GetManusLiveLinkUserIndex(ManusComponent ? ManusComponent->ManusDashboardUserIndex : ManusDashboardUserIndex, ManusComponent ? ManusComponent->ManusSkeleton : ManusSkeleton);
		if (ManusLiveLinkUserIndex != INDEX_NONE)
		{
			ManusLiveLinkRemapAsset->bShouldAnimate = FManusModule::Get().GetManusLiveLinkUser(ManusLiveLinkUserIndex).bShouldUpdateLiveLinkData;

			// When we are animating
			if (ManusLiveLinkRemapAsset->bShouldAnimate)
			{
				if (ManusComponent)
				{
					// Update Live Link subject name from the Manus component Manus Live Link User index
#if ENGINE_MAJOR_VERSION == 5 ||  ENGINE_MINOR_VERSION >= 23
					LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusComponent);
#else
					SubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusComponent);
#endif

					// Update Manus motion capture type from the Manus component Manus motion capture type 
					ManusLiveLinkRemapAsset->MotionCaptureType = ManusComponent->MotionCaptureType;

					// Update hand mirroring
					ManusLiveLinkRemapAsset->bMirrorHandBone = ManusComponent->bMirrorHandBone;

					// Update Skeletal Mesh scale (with mirroring compensation)
					ManusLiveLinkRemapAsset->SkeletalMeshScale = ManusComponent->GetComponentScale();
					if (ManusComponent->bMirrorHandBone)
					{
						ManusLiveLinkRemapAsset->SkeletalMeshScale = ManusLiveLinkRemapAsset->SkeletalMeshScale.GetAbs();
					}

					// Update Skeletal Mesh local transform in Actor space (with mirroring compensation)
					FTransform SkeletalMeshTransform = ManusComponent->GetComponentTransform();
					SkeletalMeshTransform = SkeletalMeshTransform.GetRelativeTransform(ManusComponent->GetOwner()->GetTransform());
					if (ManusComponent->bMirrorHandBone)
					{
						SkeletalMeshTransform.SetScale3D(SkeletalMeshTransform.GetScale3D().GetAbs());
						SkeletalMeshTransform.SetLocation(SkeletalMeshTransform.GetLocation() * FVector(1, -1, 1));
						SkeletalMeshTransform.SetRotation(SkeletalMeshTransform.GetRotation().Inverse());
					}
					ManusLiveLinkRemapAsset->SkeletalMeshActorSpaceTransform = SkeletalMeshTransform;

					// Update bone name map
					FMemory::Memcpy(BoneMap, ManusComponent->ManusSkeleton->BoneMap);
				}
				else
				{
					// Update Live Link subject name
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
					LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndex);
#else
					SubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndex);
#endif

					// Update Manus motion capture type
					ManusLiveLinkRemapAsset->MotionCaptureType = MotionCaptureType;

					// Default values
					ManusLiveLinkRemapAsset->bMirrorHandBone = false;
					ManusLiveLinkRemapAsset->SkeletalMeshScale = FVector::OneVector;
					ManusLiveLinkRemapAsset->SkeletalMeshActorSpaceTransform.SetIdentity();

					// Update bone name map
					FMemory::Memcpy(BoneMap, ManusSkeleton->BoneMap);
				}
			}
		}
	}
}
