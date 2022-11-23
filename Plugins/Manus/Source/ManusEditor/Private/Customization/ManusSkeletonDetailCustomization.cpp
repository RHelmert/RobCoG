// Copyright 2015-2020 Manus

#include "ManusSkeletonDetailCustomization.h"

#include "ManusBlueprintTypes.h"
#include "ManusSkeleton.h"
#include "ManusTools.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "ManusSkeletonDetailCustomization"


void FManusSkeletonDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	// Is Used For Full Body Tracking
	bool IsUsedForFullBodyTracking = true;
	TSharedRef<IPropertyHandle> IsUsedForFullBodyTrackingPropertyHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(UManusSkeleton, bIsUsedForFullBodyTracking), UManusSkeleton::StaticClass());
	IsUsedForFullBodyTrackingPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FManusSkeletonDetailCustomization::ForceRefresh));
	void* IsUsedForFullBodyTrackingValuePtr = nullptr;
	FPropertyAccess::Result ModeResult = IsUsedForFullBodyTrackingPropertyHandle->GetValueData(IsUsedForFullBodyTrackingValuePtr);
	if (ModeResult != FPropertyAccess::MultipleValues && ModeResult != FPropertyAccess::Fail && IsUsedForFullBodyTrackingValuePtr)
	{
		const bool IsUsedForFullBodyTrackingValue = *reinterpret_cast<bool*>(IsUsedForFullBodyTrackingValuePtr);
		IsUsedForFullBodyTracking = IsUsedForFullBodyTrackingValue;
	}

	// Get the retargeting target
	EManusRetargetingTarget RetargetTarget = EManusRetargetingTarget::BodyEstimation;
	TSharedRef<IPropertyHandle> RetargetTargetPropertyHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(UManusSkeleton, RetargetingTarget), UManusSkeleton::StaticClass());
	RetargetTargetPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FManusSkeletonDetailCustomization::ForceRefresh));
	void* RetargetTargetValuePtr = nullptr;
	FPropertyAccess::Result RetargetTargetModeResult = RetargetTargetPropertyHandle->GetValueData(RetargetTargetValuePtr);
	if (RetargetTargetModeResult != FPropertyAccess::MultipleValues && RetargetTargetModeResult != FPropertyAccess::Fail && RetargetTargetValuePtr)
	{
		const EManusRetargetingTarget RetargetTargetValue = *reinterpret_cast<EManusRetargetingTarget*>(RetargetTargetValuePtr);
		RetargetTarget = RetargetTargetValue;
	}


	// All categories
	TArray<FName> CategoryNames;
	InDetailBuilder.GetCategoryNames(CategoryNames);
	for (int i = 0; i < CategoryNames.Num(); i++)
	{
		if (i == 0)
		{
			// Hide top category
			InDetailBuilder.HideCategory(CategoryNames[i]);
		}
		else
		{
			IDetailCategoryBuilder& Category = InDetailBuilder.EditCategory(CategoryNames[i]);
			TArray<TSharedRef<IPropertyHandle>> Properties;
			Category.GetDefaultProperties(Properties);
			for (int PropertyIndex = 0; PropertyIndex < Properties.Num(); ++PropertyIndex)
			{
				TSharedPtr<IPropertyHandle> PropertyHandle = Properties[PropertyIndex];

				if (PropertyHandle.IsValid())
				{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
					FProperty* Property = PropertyHandle->GetProperty();
#else
					UProperty* Property = PropertyHandle->GetProperty();
#endif
					
					if (!IsUsedForFullBodyTracking 
						&& (Property->GetFName() == FName(TEXT("FullBodyStretchAxis")) 
						|| Property->GetFName() == FName(TEXT("FullBodyHeight")) 
						|| Property->GetFName() == FName(TEXT("ScaleToUser"))
						|| Property->GetFName() == FName(TEXT("RetargetingParameters"))
						|| Property->GetFName() == FName(TEXT("RetargetingTarget"))
						|| Property->GetFName() == FName(TEXT("TargetName")))
						|| (Property->GetFName() == FName(TEXT("TargetName")) && RetargetTarget != EManusRetargetingTarget::TargetSkeleton))
					{
						DetailBuilder->HideProperty(PropertyHandle);
					}
					else
					{
						Category.AddProperty(PropertyHandle);
					}

					if (Property->GetFName() == FName(TEXT("SkeletalMesh")))
					{
						Category.AddCustomRow(LOCTEXT("AutomaticSetup", "Automatic Setup"))
						.ValueContent()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(LOCTEXT("AutomaticSetup", "Automatic Setup"))
								.OnClicked(this, &FManusSkeletonDetailCustomization::OnAutomaticSetupClicked)
							]
						];
					}
				}
			}
		}
	}
}

void FManusSkeletonDetailCustomization::ForceRefresh()
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}

FReply FManusSkeletonDetailCustomization::OnAutomaticSetupClicked()
{
	if (DetailBuilder)
	{
		TArray<TWeakObjectPtr<UObject>> SelectedObjectsList;
		DetailBuilder->GetObjectsBeingCustomized(SelectedObjectsList);
		for (int32 Index = 0; Index < SelectedObjectsList.Num(); ++Index)
		{
			UManusSkeleton* CurrentManusSkeleton = Cast<UManusSkeleton>(SelectedObjectsList[Index].Get());
			if (CurrentManusSkeleton && CurrentManusSkeleton->GetSkeleton())
			{
				CurrentManusSkeleton->Modify();
				ManusTools::AutomaticSkeletonBoneNameMapping(CurrentManusSkeleton->GetSkeleton(), CurrentManusSkeleton->BoneMap);
				CurrentManusSkeleton->FullBodyStretchAxis = ManusTools::AutomaticSkeletonStretchAxisDetection(CurrentManusSkeleton->GetSkeleton());
				CurrentManusSkeleton->FullBodyHeight = ManusTools::AutomaticSkeletonHeightDetection(CurrentManusSkeleton->GetSkeleton());
				for (int HandType = 0; HandType < (int)EManusHandType::Max; HandType++)
				{
					ManusTools::AutomaticFingersRotationAxesDetection(CurrentManusSkeleton, (EManusHandType)HandType, CurrentManusSkeleton->HandsAnimationSetup[(int)HandType].FingersStretchRotationAxis, CurrentManusSkeleton->HandsAnimationSetup[(int)HandType].FingersSpreadRotationAxis);
					CurrentManusSkeleton->HandsAnimationSetup[(int)HandType].ThumbStretchRotationAxis = CurrentManusSkeleton->HandsAnimationSetup[(int)HandType].FingersStretchRotationAxis;
					CurrentManusSkeleton->HandsAnimationSetup[(int)HandType].ThumbSpreadRotationAxis = CurrentManusSkeleton->HandsAnimationSetup[(int)HandType].FingersSpreadRotationAxis;
				}
				
				CurrentManusSkeleton->OnManusSkeletonChanged();
				FManusSkeletonDetailCustomization::ForceRefresh();
			}
		}
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
