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
