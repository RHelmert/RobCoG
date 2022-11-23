// Copyright 2015-2020 Manus

#include "ManusComponentDetailCustomization.h"

#include "Manus.h"
#include "ManusBlueprintTypes.h"
#include "ManusComponent.h"
#include "ManusSkeleton.h"

#include "Editor.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "ISettingsModule.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Modules/ModuleManager.h"
#if ENGINE_MAJOR_VERSION == 5 ||  ENGINE_MINOR_VERSION >= 24
#include "Subsystems/AssetEditorSubsystem.h"
#else
#include "Toolkits/AssetEditorManager.h"
#endif

#define LOCTEXT_NAMESPACE "ManusComponentDetailCustomization"


void FManusComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	// Manus category
	IDetailCategoryBuilder& ManusCategory = DetailBuilder->EditCategory("Manus", FText::GetEmpty(), ECategoryPriority::Important);

	// Motion capture type
	TSharedRef<IPropertyHandle> MotionCaptureTypePropertyHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(UManusComponent, MotionCaptureType), UManusComponent::StaticClass());

	MotionCaptureTypePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FManusComponentDetailCustomization::ForceRefresh));

	void* MotionCaptureTypeValuePtr = nullptr;
	FPropertyAccess::Result ModeResult = MotionCaptureTypePropertyHandle->GetValueData(MotionCaptureTypeValuePtr);
	if (ModeResult == FPropertyAccess::MultipleValues || ModeResult == FPropertyAccess::Fail || MotionCaptureTypeValuePtr == nullptr)
	{
		return;
	}
	const EManusMotionCaptureType MotionCaptureType = *reinterpret_cast<EManusMotionCaptureType*>(MotionCaptureTypeValuePtr);

	if (MotionCaptureType != EManusMotionCaptureType::LeftHand && MotionCaptureType != EManusMotionCaptureType::RightHand)
	{
		TSharedRef<IPropertyHandle> MirrorHandBonePropertyHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(UManusComponent, bMirrorHandBone), UManusComponent::StaticClass());
		DetailBuilder->HideProperty(MirrorHandBonePropertyHandle);
	}

	// Mesh category
	IDetailCategoryBuilder& MeshCategory = InDetailBuilder.EditCategory(TEXT("Mesh"));
	MeshCategory.AddProperty(InDetailBuilder.GetProperty("SkeletalMesh", USkinnedMeshComponent::StaticClass()))
		.Visibility(EVisibility::Hidden);
}

void FManusComponentDetailCustomization::ForceRefresh()
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}

#undef LOCTEXT_NAMESPACE
