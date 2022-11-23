// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#pragma once

#include "ManusEditorStyle.h"
#include "Modules/ModuleInterface.h"
#include "Features/IModularFeature.h"
#include "Textures/SlateIcon.h"
#include "Runtime/Launch/Resources/Version.h"

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

	static FText GetToolbarButtonTooltip();
	static FSlateIcon GetToolbarButtonIcon();
	void OnToolbarButtonClicked();

private:
	class FManusSkeletonAssetActions* ManusSkeletonAssetActions;
};
