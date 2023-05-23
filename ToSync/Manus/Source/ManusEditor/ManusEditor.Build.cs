// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

using UnrealBuildTool;
using System.IO;

public class ManusEditor : ModuleRules
{
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

	private string ThirdPartyPath
	{
		get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
	}

	private string RuntimeModulePath
	{
		get { return Path.GetFullPath(Path.Combine(ModulePath, "../Manus/")); }
	}

	public ManusEditor(ReadOnlyTargetRules Target) : base(Target)
    {
#if UE_4_24_OR_LATER
		DefaultBuildSettings = BuildSettingsVersion.V2;
#endif
        PrivatePCHHeaderFile = "Private/ManusEditorPrivatePCH.h";

        PublicIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Public"),
				Path.Combine(RuntimeModulePath, "Public")
			} );

		PrivateIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Private"),
				Path.Combine(ThirdPartyPath, "Manus", "Include")
			} );

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"InputCore",
				"AnimGraph",
				"BlueprintGraph",
				"UnrealEd",
				"Slate",
				"DetailCustomizations",
				"SlateCore",
				"LevelEditor",
				"PropertyEditor",
				"Manus"
			} );

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Projects"
			} );
	}
}
