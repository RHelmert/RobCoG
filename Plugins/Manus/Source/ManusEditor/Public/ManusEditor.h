// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2022 Manus

#pragma once

#include "ManusEditorStyle.h"
#include "Modules/ModuleInterface.h"
#include "Features/IModularFeature.h"
#include "Textures/SlateIcon.h"
#include "Runtime/Launch/Resources/Version.h"

#include "Runtime/SlateCore/Public/Widgets/SWidget.h"

DECLARE_LOG_CATEGORY_EXTERN(LogManusEditor, All, All);


/**
 * The Manus plugin module for use in editor builds.
 * It is used for editor-only code, like part of the ManusGlove anim node, and
 * gets excluded in cooked builds.
 */
class FManusEditorModule : public IModuleInterface, public IModularFeature
{
public:
	////////////////////////////////////////////////////////////////////////////
	// overrides

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterSettings();
	void UnregisterSettings();
	void RegisterCustomizations();
	void UnregisterCustomizations();
	void RegisterAssets();
	void UnregisterAssets();


private:
	static FSlateIcon GetToolbarHostsButtonIcon();
	void OnToggleConnectClicked();
    void SwitchHost(FString p_NewHost);
	void OnToolbarRefreshHostsButtonClicked();
	TSharedRef<SWidget> FillToolbarComboButton(TSharedPtr<class FUICommandList> Commands);
	
	class FManusSkeletonAssetActions* ManusSkeletonAssetActions;
	TSharedPtr<class FUICommandList> PluginCommands;
	TArray<FString> m_HostValues;
    TArray<FString> m_CleanedHostValues;
	FString m_CurrentHostSelected;

    int m_LocalIndex;
};
