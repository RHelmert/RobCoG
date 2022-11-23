// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#include "ManusEditor.h"
#include "Manus.h"
#include "ManusSettings.h"
#include "ManusEditorUserSettings.h"
#include "ManusComponent.h"
#include "ManusSkeleton.h"
#include "ManusEditorStyle.h"
#include "Customization/ManusComponentDetailCustomization.h"
#include "AnimGraphNode_ManusLiveLinkPose.h"
#include "Customization/ManusAnimGraphNodeDetailCustomization.h"
#include "Customization/ManusSkeletonDetailCustomization.h"
#include "AssetTools/ManusSkeletonAssetActions.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ISettingsModule.h"
#include "AssetToolsModule.h"
#include "LevelEditor.h"

IMPLEMENT_MODULE(FManusEditorModule, ManusEditor)
DEFINE_LOG_CATEGORY(LogManusEditor);
#define LOCTEXT_NAMESPACE "FManusEditorModule"

void FManusEditorModule::StartupModule()
{
	UE_LOG(LogManusEditor, Log, TEXT("Started the Manus editor module"));

	FManusEditorStyle::Initialize();

	RegisterSettings();
	RegisterCustomizations();
	RegisterAssets();
}

void FManusEditorModule::ShutdownModule()
{
	UE_LOG(LogManusEditor, Log, TEXT("Shut down the Manus editor module"));

	FManusEditorStyle::Shutdown();

	UnregisterSettings();
	UnregisterCustomizations();
	UnregisterAssets();
}

void FManusEditorModule::RegisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule != nullptr)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Manus",
			LOCTEXT("ManusSettingsName", "Manus"),
			LOCTEXT("ManusSettingsDescription", "Configure the Manus plugin."),
			GetMutableDefault<UManusSettings>()
		);
		SettingsModule->RegisterSettings("Project", "Plugins", "Manus Editor",
			LOCTEXT("ManusEditorUserSettingsName", "Manus Editor"),
			LOCTEXT("ManusEditorUserSettingsDescription", "Configure the Editor settings of the Manus plugin."),
			GetMutableDefault<UManusEditorUserSettings>()
		);
	}
}

void FManusEditorModule::UnregisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Manus");
		SettingsModule->UnregisterSettings("Project", "Plugins", "Manus Editor");
	}
}

void FManusEditorModule::RegisterCustomizations()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout(UManusComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FManusComponentDetailCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout(UAnimGraphNode_ManusLiveLinkPose::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FManusAnimGraphNodeDetailCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout(UManusSkeleton::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FManusSkeletonDetailCustomization::MakeInstance));


	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");;
	{
		TSharedPtr<FExtender> ManusToolBarExtender = MakeShareable(new FExtender);

		ManusToolBarExtender->AddToolBarExtension("Compile", EExtensionHook::After, nullptr,
			FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& Builder)
				{
					Builder.AddSeparator();
					Builder.AddToolBarButton(
						FUIAction(FExecuteAction::CreateRaw(this, &FManusEditorModule::OnToolbarButtonClicked)),
						NAME_None,
						LOCTEXT("ManusToolbarButton", "Manus"),
						TAttribute<FText>::Create(&GetToolbarButtonTooltip),
						TAttribute<FSlateIcon>::Create(&GetToolbarButtonIcon),
						EUserInterfaceActionType::ToggleButton
					);
				}));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ManusToolBarExtender);
	}
}

void FManusEditorModule::UnregisterCustomizations()
{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 24
	if (UObjectInitialized() && !IsEngineExitRequested())
#else
	if (UObjectInitialized() && !GIsRequestingExit)
#endif
	{
		FPropertyEditorModule* PropertyEditorModule = FModuleManager::Get().GetModulePtr<FPropertyEditorModule>("PropertyEditor");
		if (PropertyEditorModule)
		{
			PropertyEditorModule->UnregisterCustomClassLayout(UManusComponent::StaticClass()->GetFName());
			PropertyEditorModule->UnregisterCustomClassLayout(UAnimGraphNode_ManusLiveLinkPose::StaticClass()->GetFName());
			PropertyEditorModule->UnregisterCustomClassLayout(UManusSkeleton::StaticClass()->GetFName());
		}

		if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
		{
			FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
			LevelEditor.GetToolBarExtensibilityManager().Reset();
		}
	}
}

void FManusEditorModule::RegisterAssets()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	ManusSkeletonAssetActions = new FManusSkeletonAssetActions;
	AssetTools.RegisterAssetTypeActions(MakeShareable(ManusSkeletonAssetActions));
}

void FManusEditorModule::UnregisterAssets()
{
	FAssetToolsModule* AssetToolsModule = FModuleManager::Get().GetModulePtr<FAssetToolsModule>("AssetTools");
	if (AssetToolsModule)
	{
		AssetToolsModule->Get().UnregisterAssetTypeActions(ManusSkeletonAssetActions->AsShared());
	}
}


FText FManusEditorModule::GetToolbarButtonTooltip()
{
	if (FManusModule::Get().IsActive())
	{
		return LOCTEXT("ManusToolbarButton_Tooltip_Deactivate", "Deactivate Manus tracking");
	}
	else
	{
		return LOCTEXT("ManusToolbarButton_Tooltip_Activate", "Activate Manus tracking");
	}
}

FSlateIcon FManusEditorModule::GetToolbarButtonIcon()
{
	if (FManusModule::Get().IsActive())
	{
		return FSlateIcon(FManusEditorStyle::GetStyleSetName(), "ToolbarIcon.ManusOn");
	}
	else
	{
		return FSlateIcon(FManusEditorStyle::GetStyleSetName(), "ToolbarIcon.ManusOff");
	}
}

void FManusEditorModule::OnToolbarButtonClicked()
{
	FManusModule::Get().SetActive(!FManusModule::Get().IsActive());
}

#undef LOCTEXT_NAMESPACE
