// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2022 Manus

#pragma once

// Set up a Doxygen group.
/** @addtogroup UManusEditorUserSettings
 *  @{
 */
#include "CoreTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "ManusEditorUserSettings.generated.h"


/// @brief Editor user settings for Manus plugin.
UCLASS(config= EditorUserSettings, defaultconfig)
class MANUS_API UManusEditorUserSettings : public UObject
{
	GENERATED_BODY()

public:
	UManusEditorUserSettings();

public:
	/// @brief Whether Manus tracking should be active by default. 
	UPROPERTY(config, EditAnywhere, Category = "Manus")
	bool bIsManusActiveByDefault;

	/// @brief Whether Manus should animate the skeletons in the Editor views. 
	UPROPERTY(config, EditAnywhere, Category = "Manus")
	bool bAnimateInEditor;
};

// Close the Doxygen group.
/** @} */
