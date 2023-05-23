// Copyright 2015-2020 Manus

#include "ManusAnimGraphNodeDetailCustomization.h"

#include "Manus.h"
#include "ManusLiveLinkUser.h"
#include "ManusBlueprintTypes.h"
#include "AnimNode_ManusLiveLinkPose.h"
#include "AnimGraphNode_ManusLiveLinkPose.h"
#include "ManusTools.h"
#include "ManusSkeleton.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Modules/ModuleManager.h"
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 24
#include "Subsystems/AssetEditorSubsystem.h"
#else
#include "Toolkits/AssetEditorManager.h"
#endif
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "ManusAnimGraphNodeDetailCustomization"


void FManusAnimGraphNodeDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	DetailBuilder->HideCategory(FName(TEXT("SourceData")));
	DetailBuilder->HideCategory(FName(TEXT("Retarget")));

	// Get first animgraph node
	TArray< TWeakObjectPtr<UObject> > SelectedObjectsList;
	DetailBuilder->GetObjectsBeingCustomized(SelectedObjectsList);
	UAnimGraphNode_ManusLiveLinkPose* AnimGraphNode = Cast<UAnimGraphNode_ManusLiveLinkPose>(SelectedObjectsList[0].Get());
	if (AnimGraphNode == nullptr)
	{
		return;
	}

	// Make sure type matches with all the nodes
	bool ShouldHideTrackingDeviceDeltaTransform = true;
	const UAnimGraphNode_ManusLiveLinkPose* FirstNodeType = AnimGraphNode;
	for (int32 Index = 0; Index < SelectedObjectsList.Num(); ++Index)
	{
		UAnimGraphNode_ManusLiveLinkPose* CurrentNode = Cast<UAnimGraphNode_ManusLiveLinkPose>(SelectedObjectsList[Index].Get());
		if (!CurrentNode || CurrentNode->GetClass() != FirstNodeType->GetClass())
		{
			// If type mismatches, multi selection doesn't work, just return
			return;
		}
		if (ShouldHideTrackingDeviceDeltaTransform && CurrentNode->Node.MotionCaptureType != EManusMotionCaptureType::FullBody)
		{
			ShouldHideTrackingDeviceDeltaTransform = false;
		}
	}

	// Hide TrackingDeviceDeltaTransform when MotionCaptureType is Full Body
	EManusMotionCaptureType MotionCaptureTypeVal = EManusMotionCaptureType::LeftHand;
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
	TSharedPtr<IPropertyHandle> MotionCaptureTypePropertyHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(FAnimNode_ManusLiveLinkPose, MotionCaptureType), FAnimNode_ManusLiveLinkPose::StaticStruct());
#else
	TSharedPtr<IPropertyHandle> MotionCaptureTypePropertyHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(FAnimNode_ManusLiveLinkPose, MotionCaptureType), (UClass*)(FAnimNode_ManusLiveLinkPose::StaticStruct()));
#endif
	if (MotionCaptureTypePropertyHandle)
	{
		MotionCaptureTypePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FManusAnimGraphNodeDetailCustomization::ForceRefresh));
	}

	// Look for inner properties
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
	FStructProperty* NodeProperty = AnimGraphNode->GetFNodeProperty();
#else
	UStructProperty* NodeProperty = AnimGraphNode->GetFNodeProperty();
#endif
	if (NodeProperty)
	{
		TSharedRef<IPropertyHandle> NodePropertyHandle = DetailBuilder->GetProperty(NodeProperty->GetFName(), AnimGraphNode->GetClass());
		uint32 NumChildHandles = 0;
		FPropertyAccess::Result Result = NodePropertyHandle->GetNumChildren(NumChildHandles);
		if (Result != FPropertyAccess::Fail)
		{
			IDetailCategoryBuilder& ManusLiveLinkCategory = InDetailBuilder.EditCategory(TEXT("ManusLiveLink"));

			for (uint32 ChildHandleIndex = 0; ChildHandleIndex < NumChildHandles; ++ChildHandleIndex)
			{
				TSharedPtr<IPropertyHandle> TargetPropertyHandle = NodePropertyHandle->GetChildHandle(ChildHandleIndex);
				if (TargetPropertyHandle.IsValid())
				{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
					FProperty* TargetProperty = TargetPropertyHandle->GetProperty();
#else
					UProperty* TargetProperty = TargetPropertyHandle->GetProperty();
#endif

					if (TargetPropertyHandle->GetDefaultCategoryName() == FName(TEXT("ManusLiveLink")))
					{
						if (TargetProperty->GetFName() == FName(TEXT("TrackingDeviceDeltaTransform")) && ShouldHideTrackingDeviceDeltaTransform)
						{
							DetailBuilder->HideProperty(TargetPropertyHandle);
						}
						else
						{
							ManusLiveLinkCategory.AddProperty(TargetPropertyHandle);
						}
					}
				}
			}
		}
	}
}

void FManusAnimGraphNodeDetailCustomization::ForceRefresh()
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}

#undef LOCTEXT_NAMESPACE
