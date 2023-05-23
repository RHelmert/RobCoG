// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#include "ManusSkeleton.h"
#include "CoreSdk.h"
#include "Manus.h"
#include "ManusTools.h"
#include "ManusSettings.h"
#include "ManusComponent.h"
#include "Engine/SkeletalMesh.h"


UManusSkeleton::UManusSkeleton()
{
	SkeletalMesh = NULL;

	HandsAnimationSetup[(int)EManusHandType::Left].FingersStretchRotationAxis = EManusAxisOption::Z_Neg;
	HandsAnimationSetup[(int)EManusHandType::Left].FingersSpreadRotationAxis = EManusAxisOption::Y;
	HandsAnimationSetup[(int)EManusHandType::Left].ThumbStretchRotationAxis = EManusAxisOption::Z_Neg;
	HandsAnimationSetup[(int)EManusHandType::Left].ThumbSpreadRotationAxis = EManusAxisOption::Y;
	HandsAnimationSetup[(int)EManusHandType::Right].FingersStretchRotationAxis = EManusAxisOption::Z_Neg;
	HandsAnimationSetup[(int)EManusHandType::Right].FingersSpreadRotationAxis = EManusAxisOption::Y;
	HandsAnimationSetup[(int)EManusHandType::Right].ThumbStretchRotationAxis = EManusAxisOption::Z_Neg;
	HandsAnimationSetup[(int)EManusHandType::Right].ThumbSpreadRotationAxis = EManusAxisOption::Y;

	for (int HandType = 0; HandType < (int)EManusHandType::Max; HandType++)
	{
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Thumb].FingerSpreadExtent = FFloatRange(FFloatRangeBound::Inclusive(-30.0f), FFloatRangeBound::Inclusive(45.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Thumb].JointsStretchExtents[(int)EManusPhalangeName::Proximal] = FFloatRange(FFloatRangeBound::Inclusive(-20.0f), FFloatRangeBound::Inclusive(25.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Thumb].JointsStretchExtents[(int)EManusPhalangeName::Intermediate] = FFloatRange(FFloatRangeBound::Inclusive(-20.0f), FFloatRangeBound::Inclusive(45.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Thumb].JointsStretchExtents[(int)EManusPhalangeName::Distal] = FFloatRange(FFloatRangeBound::Inclusive(-15.0f), FFloatRangeBound::Inclusive(80.0f));

		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Index].FingerSpreadExtent = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(20.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Index].JointsStretchExtents[(int)EManusPhalangeName::Proximal] = FFloatRange(FFloatRangeBound::Inclusive(-17.5f), FFloatRangeBound::Inclusive(80.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Index].JointsStretchExtents[(int)EManusPhalangeName::Intermediate] = FFloatRange(FFloatRangeBound::Inclusive(-5.0f), FFloatRangeBound::Inclusive(100.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Index].JointsStretchExtents[(int)EManusPhalangeName::Distal] = FFloatRange(FFloatRangeBound::Inclusive(-5.0f), FFloatRangeBound::Inclusive(90.0f));

		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Middle].FingerSpreadExtent = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(20.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Middle].JointsStretchExtents[(int)EManusPhalangeName::Proximal] = FFloatRange(FFloatRangeBound::Inclusive(-15.0f), FFloatRangeBound::Inclusive(80.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Middle].JointsStretchExtents[(int)EManusPhalangeName::Intermediate] = FFloatRange(FFloatRangeBound::Inclusive(-5.0f), FFloatRangeBound::Inclusive(100.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Middle].JointsStretchExtents[(int)EManusPhalangeName::Distal] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(90.0f));

		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Ring].FingerSpreadExtent = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(20.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Ring].JointsStretchExtents[(int)EManusPhalangeName::Proximal] = FFloatRange(FFloatRangeBound::Inclusive(-15.0f), FFloatRangeBound::Inclusive(80.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Ring].JointsStretchExtents[(int)EManusPhalangeName::Intermediate] = FFloatRange(FFloatRangeBound::Inclusive(-5.0f), FFloatRangeBound::Inclusive(100.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Ring].JointsStretchExtents[(int)EManusPhalangeName::Distal] = FFloatRange(FFloatRangeBound::Inclusive(-5.0f), FFloatRangeBound::Inclusive(90.0f));

		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Pinky].FingerSpreadExtent = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(20.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Pinky].JointsStretchExtents[(int)EManusPhalangeName::Proximal] = FFloatRange(FFloatRangeBound::Inclusive(-15.0f), FFloatRangeBound::Inclusive(80.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Pinky].JointsStretchExtents[(int)EManusPhalangeName::Intermediate] = FFloatRange(FFloatRangeBound::Inclusive(-5.0f), FFloatRangeBound::Inclusive(100.0f));
		HandsAnimationSetup[HandType].FingersRotationsExtents[(int)EManusFingerName::Pinky].JointsStretchExtents[(int)EManusPhalangeName::Distal] = FFloatRange(FFloatRangeBound::Inclusive(-5.0f), FFloatRangeBound::Inclusive(90.0f));
	}
	
	bIsUsedForFullBodyTracking = false;
	FullBodyStretchAxis = EBoneAxis::BA_X;
	FullBodyHeight = 1.8f;
}

USkeleton* UManusSkeleton::GetSkeleton()
{
	if (SkeletalMesh)
	{
		return SkeletalMesh->Skeleton;
	}
	return NULL;
}

void UManusSkeleton::OnManusSkeletonChanged()
{
	if (bIsUsedForFullBodyTracking)
	{
		TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
		if (ManusLocalLiveLinkSource.IsValid())
		{
			ManusLocalLiveLinkSource->InitPolygonForAllManusLiveLinkUsers(true);
		}
	}
}

void UManusSkeleton::OnRetargetingSettingsChanged()
{
	TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	for (int i = 0; i < ManusLiveLinkUsers.Num(); i++)
	{
		if (ManusLiveLinkUsers[i].ManusSkeleton == this) 
		{
			int64 SkeletonId = ManusTools::GenerateManusIdFromManusLiveLinkUser(i);
			CoreSdk::SetRetargetingSettings(SkeletonId, RetargetingParameters);
		}
	}
}

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 26

void UManusSkeleton::PostLoad()
{
	Super::PostLoad();

	if (HasAnyFlags(RF_WasLoaded) && SkeletalMesh)
	{
		bool bConvertedBoneMap = false;

		for (int BoneIndex = 0; BoneIndex < (int)EManusBoneName::Max; BoneIndex++)
		{
			if (BoneMap[BoneIndex].BoneName.IsNone() && !BoneNameMap_DEPRECATED[BoneIndex].IsNone())
			{
				bConvertedBoneMap = true;
				BoneMap[BoneIndex].BoneName = BoneNameMap_DEPRECATED[BoneIndex];
			}
			BoneNameMap_DEPRECATED[BoneIndex] = NAME_None;
		}

		if (bConvertedBoneMap)
		{
			UE_LOG(LogManus, Warning, TEXT("The bone name map of the Manus Skeleton %s was converted to a bone map that uses bone references. Please resave this asset."), *GetName());
		}
	}
}

#endif

#if WITH_EDITOR
void UManusSkeleton::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
	FProperty* TailProperty = PropertyChangedEvent.PropertyChain.GetTail()->GetValue();
#else
	UProperty* TailProperty = PropertyChangedEvent.PropertyChain.GetTail()->GetValue();
#endif
	FName TailPropertyName = TailProperty->GetFName();

	if (TailPropertyName == FName("SkeletalMesh"))
	{
		ManusTools::AutomaticSkeletonBoneNameMapping(GetSkeleton(), BoneMap);
		FullBodyStretchAxis = ManusTools::AutomaticSkeletonStretchAxisDetection(GetSkeleton());
		FullBodyHeight = ManusTools::AutomaticSkeletonHeightDetection(GetSkeleton());
		for (int HandType = 0; HandType < (int)EManusHandType::Max; HandType++)
		{
			ManusTools::AutomaticFingersRotationAxesDetection(this, (EManusHandType)HandType, HandsAnimationSetup[(int)HandType].FingersStretchRotationAxis, HandsAnimationSetup[(int)HandType].FingersSpreadRotationAxis);
			HandsAnimationSetup[(int)HandType].ThumbStretchRotationAxis = HandsAnimationSetup[(int)HandType].FingersStretchRotationAxis;
			HandsAnimationSetup[(int)HandType].ThumbSpreadRotationAxis = HandsAnimationSetup[(int)HandType].FingersSpreadRotationAxis;
		}

		// Update the skeleton in every Manus component using it
		TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
		if (ManusLocalLiveLinkSource.IsValid())
		{
			const TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
			for (int i = 0; i < ManusLiveLinkUsers.Num(); i++)
			{
				if (FManusModule::Get().ManusLiveLinkUsers.IsValidIndex(i))
				{
					FManusLiveLinkUser& ManusLiveLinkUser = FManusModule::Get().ManusLiveLinkUsers[i];
					for (int j = 0; j < ManusLiveLinkUser.ObjectsUsingUser.Num(); j++)
					{
						if (ManusLiveLinkUser.ObjectsUsingUser[j].IsValid())
						{
							UManusComponent* ManusComponent = Cast<UManusComponent>(ManusLiveLinkUser.ObjectsUsingUser[j].Get());
							if (ManusComponent)
							{
								ManusComponent->RefreshManusSkeleton();
							}
						}
					}
				}
			}
		}
	}

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
	FProperty* HeadProperty = PropertyChangedEvent.PropertyChain.GetHead()->GetValue();
#else
	UProperty* HeadProperty = PropertyChangedEvent.PropertyChain.GetHead()->GetValue();
#endif
	FName HeadPropertyName = HeadProperty->GetFName();

	if (HeadPropertyName == FName("HandsAnimationSetup"))
	{
		TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
		if (ManusLocalLiveLinkSource.IsValid())
		{
			ManusLocalLiveLinkSource->PreviewSkeletonFingerRotationExtents(this);
		}
	}
	
	if (HeadPropertyName == FName("RetargetingParameters")) 
	{
		OnRetargetingSettingsChanged();
	}
	else 
	{
		OnManusSkeletonChanged();
	}
}
#endif //WITH_EDITOR
