// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#pragma once

#include "Styling/ISlateStyle.h"
#include "Styling/SlateStyle.h"

class FManusEditorStyle
{	
public:
	static void Initialize();
	static void Shutdown();

	static TSharedPtr<class ISlateStyle> Get();

	static FName GetStyleSetName();

private:
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);

	static TSharedPtr<class FSlateStyleSet> StyleSet;
};
