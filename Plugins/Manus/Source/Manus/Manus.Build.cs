// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

// (Un)comment this to control if SteamVR is used for tracking in the plugin.
// The SteamVR plugin should also be disabled in the Unreal Editor settings for this to work.
#define USE_STEAMVR


using UnrealBuildTool;
using System.IO;

public class Manus : ModuleRules
{
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

	private string ThirdPartyPath
	{
		get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
	}

	private string LibraryPath
	{
		get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "Manus", "Lib")); }
	}

	public Manus(ReadOnlyTargetRules Target) : base(Target)
	{
#if UE_4_24_OR_LATER
		DefaultBuildSettings = BuildSettingsVersion.V2;
#endif
        PrivatePCHHeaderFile = "Private/ManusPrivatePCH.h";

        bEnableExceptions = true;

		PublicIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Public")
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
				"Engine",
				"Core",
				"CoreUObject",
				"InputCore",
				"InputDevice",
				"Slate",
				"SlateCore",
				"Projects",
				"HeadMountedDisplay",
#if USE_STEAMVR
				"SteamVR",
#if !UE_4_24_OR_LATER
				"SteamVRController",
#endif
#endif
				"OpenVR",
				"Sockets",
				"Networking",
                "LiveLink",
				"LiveLinkInterface",
			});

#if USE_STEAMVR
		PublicDefinitions.Add("MANUS_PLUGIN_USE_STEAMVR");
#endif

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"UnrealEd"
				}
			);
		}

		LoadManusLib(Target);
	}

	public bool LoadManusLib(ReadOnlyTargetRules Target)
	{
		bool isLibrarySupported = false;

		if ((Target.Platform == UnrealTargetPlatform.Win64))
		{
			isLibrarySupported = true;

			string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
			string ManusDirectory = Path.Combine(BaseDirectory, "ThirdParty", "Manus", "Lib", "Win64");

			// Add the libraries. 
			RuntimeDependencies.Add(Path.Combine(ManusDirectory, "CoreSdkWrapper.dll"));

#if UE_VERSION_BELOW_4_24
			PublicLibraryPaths.Add(ManusDirectory); 
#endif
		}

		return isLibrarySupported;
	}
}
