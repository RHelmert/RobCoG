// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2022 Manus

#include "ManusEditorStyle.h"

#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"

#if ENGINE_MAJOR_VERSION == 4 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0 )

#include "EditorStyleSet.h"

#endif

#include "Misc/Paths.h"
#include "Interfaces/IPluginManager.h"

#define PLUGIN_BRUSH(RelativePath,...) FSlateImageBrush(FManusEditorStyle::InContent(RelativePath,".png"),__VA_ARGS__)

FString FManusEditorStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString Content = IPluginManager::Get().FindPlugin(TEXT("Manus"))->GetContentDir();

	return (Content / RelativePath) + Extension;
}

TSharedPtr<FSlateStyleSet> FManusEditorStyle::StyleSet = nullptr;
TSharedPtr<class ISlateStyle> FManusEditorStyle::Get() { return StyleSet; }

FName FManusEditorStyle::GetStyleSetName()
{
	static FName ManusEditorStyleName(TEXT("ManusEditorStyle"));
	return ManusEditorStyleName;
}

BEGIN_FUNCTION_BUILD_OPTIMIZATION

void FManusEditorStyle::Initialize()
{
	if (StyleSet.IsValid()) { return; }

	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon30x30(30.0f, 30.0f);
	const FVector2D Icon90x30(90.0f, 30.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon64x64(64.0f, 64.0f);

	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("Manus"))->GetContentDir());

	StyleSet->Set("ClassIcon.ManusSkeleton", new PLUGIN_BRUSH(TEXT("Icons/Manus_16x"), Icon16x16));
	StyleSet->Set("ClassThumbnail.ManusSkeleton", new PLUGIN_BRUSH(TEXT("Icons/Manus_64x"), Icon64x64));
	StyleSet->Set("ToolbarIcon.ManusOn", new PLUGIN_BRUSH(TEXT("Icons/Manus_On_40x"), Icon40x40)); // todo replace with new manus icon
	StyleSet->Set("ToolbarIcon.ManusOff", new PLUGIN_BRUSH(TEXT("Icons/Manus_Off_40x"), Icon40x40));
	StyleSet->Set("ToolbarIcon.Refresh", new PLUGIN_BRUSH(TEXT("Icons/refreshWhite"), Icon30x30));
	StyleSet->Set("ToolbarIcon.Host", new PLUGIN_BRUSH(TEXT("Icons/Host"), Icon30x30));
	StyleSet->Set("ToolbarIcon.RedLeft", new PLUGIN_BRUSH(TEXT("Icons/left"), Icon30x30));
	StyleSet->Set("ToolbarIcon.dot", new PLUGIN_BRUSH(TEXT("Icons/dot"), Icon30x30));
    StyleSet->Set("ToolbarIcon.disconnect", new PLUGIN_BRUSH(TEXT("Icons/icon_MANUS_Disconnect"), Icon30x30));
    StyleSet->Set("ToolbarIcon.connected", new PLUGIN_BRUSH(TEXT("Icons/icon_MANUS_connected"), Icon30x30));
    StyleSet->Set("ToolbarIcon.GreenDot", new PLUGIN_BRUSH(TEXT("Icons/icon_MANUS_Connected_Dot"), Icon30x30));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

END_FUNCTION_BUILD_OPTIMIZATION

#undef IMAGE_BRUSH

void FManusEditorStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}
