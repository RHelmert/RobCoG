// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2022 Manus

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
	
	// Init Live Link subject name
	LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(0);

	// Init retarget asset
	RetargetAsset = UManusLiveLinkRemapAsset::StaticClass();

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

        int ManusLiveLinkUserIndex = INDEX_NONE;
        if ((ManusComponent != NULL) &&
            (ManusComponent->ManusSkeleton != NULL))
        {
            ManusLiveLinkUserIndex = FManusModule::Get().GetManusLiveLinkUserIndex(ManusComponent->ManusSkeleton->TargetUserIndexData.userIndex , ManusComponent->ManusSkeleton );
        }
        else
        {
            if (ManusSkeleton == NULL)
            {
                UE_LOG(LogManus, Warning, TEXT("No ManusSkeleton Assigned. please assign a skeleton first to the manus livelink pose!"));
                return;
            }
            ManusLiveLinkUserIndex = FManusModule::Get().GetManusLiveLinkUserIndex(ManusSkeleton->TargetUserIndexData.userIndex , ManusSkeleton);
        }

		if (ManusLiveLinkUserIndex != INDEX_NONE)
		{
			ManusLiveLinkRemapAsset->bShouldAnimate = FManusModule::Get().GetManusLiveLinkUser(ManusLiveLinkUserIndex).bShouldUpdateLiveLinkData;

			// When we are animating
			if (ManusLiveLinkRemapAsset->bShouldAnimate)
			{
				if (ManusComponent)
				{
					// Update Live Link subject name from the Manus component Manus Live Link User index
					LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusComponent);

					// Update Skeletal Mesh scale (with mirroring compensation)
					ManusLiveLinkRemapAsset->SkeletalMeshScale = ManusComponent->GetComponentScale();
               
					// Update Skeletal Mesh local transform in Actor space (with mirroring compensation)
					FTransform SkeletalMeshTransform = ManusComponent->GetComponentTransform();
					SkeletalMeshTransform = ManusComponent->GetOwner()->GetTransform();
                  
					ManusLiveLinkRemapAsset->SkeletalMeshActorSpaceTransform = SkeletalMeshTransform;
				}
				else
				{
					// Update Live Link subject name
					LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndex);

					// Default values
					ManusLiveLinkRemapAsset->SkeletalMeshScale = FVector::OneVector;
					ManusLiveLinkRemapAsset->SkeletalMeshActorSpaceTransform.SetIdentity();
				}
			}
		}
	}
}
