// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#pragma once

#include "CoreTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "ManusEditorUserSettings.generated.h"


/**
 * Editor user settings for Manus plugin.
 */
UCLASS(config= EditorUserSettings, defaultconfig)
class MANUS_API UManusEditorUserSettings : public UObject
{
	GENERATED_BODY()

public:
	UManusEditorUserSettings();

public:
	/** Whether Manus tracking should be active by default. */
	UPROPERTY(config, EditAnywhere, Category = "Manus")
	bool bIsManusActiveByDefault;

	/** Whether Manus should animate the skeletons in the Editor views. */
	UPROPERTY(config, EditAnywhere, Category = "Manus")
	bool bAnimateInEditor;
};
