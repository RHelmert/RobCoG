// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2022 Manus

#include "Manus.h"
#include "CoreSdk.h"
#include "ManusLiveLinkSource.h"
#include "ManusReplicator.h"
#include "ManusSettings.h"
#include "ManusEditorUserSettings.h"
#include "ManusComponent.h"

#include "LiveLinkClient.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Features/IModularFeatures.h"
#include "Misc/MessageDialog.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

IMPLEMENT_MODULE(FManusModule, Manus)
DEFINE_LOG_CATEGORY(LogManus);
#define LOCTEXT_NAMESPACE "FManusModule"

// Initialize the module.
void FManusModule::StartupModule()
{
	IModuleInterface::StartupModule();

	// Register Live Link events
	FLiveLinkClient* Client = &IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(FLiveLinkClient::ModularFeatureName);
	if (Client)
	{
		Client->OnLiveLinkSourceRemoved().AddRaw(this, &FManusModule::OnLiveLinkSourceRemoved);
	}

	// Register game mode events
	FGameModeEvents::GameModePostLoginEvent.AddRaw(this, &FManusModule::OnGameModePostLogin);
	FGameModeEvents::GameModeLogoutEvent.AddRaw(this, &FManusModule::OnGameModeLogout);

#if WITH_EDITOR
	// Activate Manus
	if (GIsEditor && GetDefault<UManusEditorUserSettings>()->bIsManusActiveByDefault)
	{
		SetActive(true);
	}

	FPluginDescriptor PluginData;
	GetPluginData(PluginData);

	FString EngineVersion = FString((std::to_string(ENGINE_MAJOR_VERSION) + "." + std::to_string(ENGINE_MINOR_VERSION)).c_str());
	FString PluginEngineVersionCompatibility = PluginData.EngineVersion;

	FText EngineVersionTest = FText::FromString(EngineVersion);

	if (!PluginEngineVersionCompatibility.IsEmpty() && !PluginEngineVersionCompatibility.Contains(EngineVersion, ESearchCase::IgnoreCase, ESearchDir::FromStart))
	{
		UE_LOG(LogManus, Error, TEXT("Plugin version (%s) is not compatible with this engine version (%s)."), *PluginEngineVersionCompatibility, *EngineVersion);
		FText DialogTitle = FText::FromString("Engine version uncompatible");
		FText DialogText = FText::FromString("The supported engine version of the plugin (" + PluginEngineVersionCompatibility + ") and the engine version (" + EngineVersion + ") do not match and might not work properly. Please use the correct plugin with the correct version.");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText, &DialogTitle);
		
		// FPlatformMisc::RequestExit(false);
	}

	// Register PIE/SIE Events
	FEditorDelegates::BeginPIE.AddRaw(this, &FManusModule::HandleBeginPIE);
	FEditorDelegates::EndPIE.AddRaw(this, &FManusModule::HandleEndPIE);
#endif // WITH_EDITOR

	// Standalone initial activation
	if (!GIsEditor)
	{
		SetActive(GetDefault<UManusSettings>()->bUseManusInGame);
	}
    m_IsClient = false;
}

// Shut the module down.
void FManusModule::ShutdownModule()
{
	SetActive(false);

	// Unregister Live Link events
	FLiveLinkClient* Client = &IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(FLiveLinkClient::ModularFeatureName);
	if (Client)
	{
		Client->OnLiveLinkSourceRemoved().RemoveAll(this);
	}

	// Unregister game mode events
	FGameModeEvents::GameModePostLoginEvent.RemoveAll(this);
	FGameModeEvents::GameModeLogoutEvent.RemoveAll(this);

#if WITH_EDITOR
	// Unregister PIE/SIE Events
	FEditorDelegates::BeginPIE.RemoveAll(this);
	FEditorDelegates::EndPIE.RemoveAll(this);
#endif // WITH_EDITOR

	IModuleInterface::ShutdownModule();

}

FString FManusModule::GetManusCoreIP()
{
	return m_ManusIP;
}

void FManusModule::SetAsClient(bool p_IsClient)
{
    m_IsClient = p_IsClient;
}

bool FManusModule::IsClient()
{
    return m_IsClient;
}

bool FManusModule::SetManusCoreIP(FString p_ManusIP )
{
	m_ManusIP = p_ManusIP;
	// NOW we can actually try to connect to manus core

	if (bIsActive)
	{
		EManusRet t_Ret = EManusRet::Error;
        if (p_ManusIP.IsEmpty()) return false; // no connection.

		// check if local or remote
		if ((p_ManusIP.Compare("LocalHost") == 0) ||
			(p_ManusIP.Compare("127.0.0.1") == 0))
		{
			t_Ret = CoreSdk::ConnectLocally();
		}
		else
		{
			t_Ret = CoreSdk::ConnectRemotely(p_ManusIP);
		}
		if (t_Ret == EManusRet::Success)
		{
			// Init Live Link local source
			GetLiveLinkSource(EManusLiveLinkSourceType::Local);
			return true;
		}
	}
	return false;
}

void FManusModule::GetRemoteHosts(TArray<FString>& p_Hosts)
{
	m_Hosts.Reset();
	CoreSdk::FindRemoteHosts(m_Hosts);

	p_Hosts.Insert(m_Hosts,0);
}

void FManusModule::SetActive(bool bNewIsActive)
{
	if (bIsActive != bNewIsActive)
	{
		bIsActive = bNewIsActive;
		if (bIsActive)
		{
			UE_LOG(LogManus, Log, TEXT("Manus SDK activated."));

			// Init Core SDK
			CoreSdk::Initialize();
		}
		else
		{
			UE_LOG(LogManus, Log, TEXT("Manus SDK deactivated."));

			// Shut Core SDK down
			CoreSdk::ShutDown();
			m_ManusIP = "";
			// Invalidate Live Link sources
			if (LiveLinkSources[(int)EManusLiveLinkSourceType::Local])
			{
				LiveLinkSources[(int)EManusLiveLinkSourceType::Local]->Destroy();
				LiveLinkSources[(int)EManusLiveLinkSourceType::Local].Reset();
			}
		}
	}
}

void FManusModule::OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
    GetLiveLinkSource(EManusLiveLinkSourceType::Replicated); // we always need the replicants!
    
	// Spawn Manus replicator
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = NewPlayer;
	AManusReplicator* Replicator = GameMode->GetWorld()->SpawnActor<AManusReplicator>(ActorSpawnParameters);

	Replicator->ReplicatorId = NewPlayer->PlayerState->GetPlayerId();
	Replicators.Add(NewPlayer, Replicator);
}

void FManusModule::OnGameModeLogout(AGameModeBase* GameMode, AController* Exiting)
{
	// Destroy Manus replicator
	AManusReplicator** Replicator = Replicators.Find(Exiting);
	if (Replicator)
	{
		(*Replicator)->Destroy();
		Replicators.Remove(Exiting);
	}

    if (LiveLinkSources[(int)EManusLiveLinkSourceType::Replicated])
    {
        LiveLinkSources[(int)EManusLiveLinkSourceType::Replicated]->Destroy();
        LiveLinkSources[(int)EManusLiveLinkSourceType::Replicated].Reset();
    }
}

int FManusModule::GetManusLiveLinkUserIndex(int ManusDashboardUserIndex, class UManusSkeleton* ManusSkeleton)
{
	for (int i = 0; i < ManusLiveLinkUsers.Num(); i++)
	{
		if (ManusLiveLinkUsers[i].ManusDashboardUserIndex == ManusDashboardUserIndex && ManusLiveLinkUsers[i].ManusSkeleton == ManusSkeleton)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

FManusLiveLinkUser& FManusModule::GetManusLiveLinkUser(int Index)
{
	check(ManusLiveLinkUsers.IsValidIndex(Index));
	return ManusLiveLinkUsers[Index];
}

void FManusModule::AddObjectUsingManusLiveLinkUser(int ManusDashboardUserIndex, class UManusSkeleton* ManusSkeleton, UObject* Object)
{
	if (ManusSkeleton)
	{
		int Index = GetManusLiveLinkUserIndex(ManusDashboardUserIndex, ManusSkeleton);
		if (Index == INDEX_NONE)
		{
			Index = ManusLiveLinkUsers.AddDefaulted();
			ManusLiveLinkUsers[Index].ManusDashboardUserIndex = ManusDashboardUserIndex; // todo get a manus user ID in here instead of an index.
			ManusLiveLinkUsers[Index].ManusSkeleton = ManusSkeleton;
			ManusLiveLinkUsers[Index].SkeletonsInitializationRetryCountdown = 1;
		}
		ManusLiveLinkUsers[Index].ObjectsUsingUser.AddUnique(Object);
	}
}

void FManusModule::RemoveObjectUsingManusLiveLinkUser(int ManusDashboardUserIndex, class UManusSkeleton* ManusSkeleton, UObject* Object)
{
	int Index = GetManusLiveLinkUserIndex(ManusDashboardUserIndex, ManusSkeleton);
	if (Index != INDEX_NONE)
	{
		ManusLiveLinkUsers[Index].ObjectsUsingUser.Remove(Object);
		if (ManusLiveLinkUsers[Index].ObjectsUsingUser.Num() == 0)
		{
			ManusLiveLinkUsers.RemoveAt(Index);
		}
	}
}

bool FManusModule::IsAnyObjectUsingManusLiveLinkUser(int ManusLiveLinkUserIndex)
{
	if (ManusLiveLinkUsers.IsValidIndex(ManusLiveLinkUserIndex))
	{
		for (int i = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ObjectsUsingUser.Num() - 1; i >= 0; i--)
		{
			if (ManusLiveLinkUsers[ManusLiveLinkUserIndex].ObjectsUsingUser[i].IsValid())
			{
				bool bIsObjectRelevant = true;
#if WITH_EDITOR
				// Ignore Editor objects if bAnimateInEditor is false
				if (!GetDefault<UManusEditorUserSettings>()->bAnimateInEditor)
				{
					UWorld* ObjectWorld = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ObjectsUsingUser[i]->GetWorld();
					if (!ObjectWorld || ObjectWorld->WorldType == EWorldType::Editor || ObjectWorld->WorldType == EWorldType::EditorPreview || ObjectWorld->WorldType == EWorldType::Inactive)
					{
						bIsObjectRelevant = false;
					}
				}
#endif // WITH_EDITOR

				if (bIsObjectRelevant)
				{
					return true;
				}
			}
			else
			{
				ManusLiveLinkUsers[ManusLiveLinkUserIndex].ObjectsUsingUser.RemoveAt(i, 1, false);
			}
		}
	}
	return false;
}

bool FManusModule::IsAnyReplicatingObjectUsingManusLiveLinkUser(int ManusLiveLinkUserIndex)
{
	if (ManusLiveLinkUsers.IsValidIndex(ManusLiveLinkUserIndex))
	{
		for (int i = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ObjectsUsingUser.Num() - 1; i >= 0; i--)
		{
			if (ManusLiveLinkUsers[ManusLiveLinkUserIndex].ObjectsUsingUser[i].IsValid())
			{
				TWeakObjectPtr<UObject> Object = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ObjectsUsingUser[i];

#if WITH_EDITOR
				// Ignore Editor objects
				UWorld* ObjectWorld = Object->GetWorld();
				if (!ObjectWorld || ObjectWorld->WorldType == EWorldType::Editor || ObjectWorld->WorldType == EWorldType::EditorPreview || ObjectWorld->WorldType == EWorldType::Inactive)
				{
					continue;
				}
#endif // WITH_EDITOR

				// Check if the object is replicating
				if (UManusComponent* ManusComponentUsingManusLiveLinkUser = Cast<UManusComponent>(Object.Get()))
				{
					return ManusComponentUsingManusLiveLinkUser->GetIsReplicated() && ManusComponentUsingManusLiveLinkUser->GetOwner() && ManusComponentUsingManusLiveLinkUser->GetOwner()->GetIsReplicated();
				}
			}
			else
			{
				ManusLiveLinkUsers[ManusLiveLinkUserIndex].ObjectsUsingUser.RemoveAt(i, 1, false);
			}
		}
	}
	return false;
}

bool FManusModule::GetPluginData(FPluginDescriptor& PluginData)
{
	FText PluginDescriptorError;

	FString EnginePluginFile = FPaths::ProjectPluginsDir() + "Manus/" + "Manus.uplugin";
	FString ProjectPluginFile = FPaths::ProjectPluginsDir() + "Manus/" + "Manus.uplugin";
	FString EnterprisePluginFile = FPaths::ProjectPluginsDir() + "Manus/" + "Manus.uplugin";

	if (FPaths::FileExists(EnginePluginFile))
	{
		PluginData.Load(EnginePluginFile, PluginDescriptorError);
	}
	else if (FPaths::FileExists(ProjectPluginFile))
	{
		PluginData.Load(ProjectPluginFile, PluginDescriptorError);
	}
	else if (FPaths::FileExists(EnterprisePluginFile))
	{
		PluginData.Load(EnterprisePluginFile, PluginDescriptorError);
	}

	return !PluginData.VersionName.IsEmpty();
}

#if WITH_EDITOR

void FManusModule::HandleBeginPIE(const bool InIsSimulating)
{
	bWasActiveBeforePIE = bIsActive;
	SetActive(GetDefault<UManusSettings>()->bUseManusInGame);
}

void FManusModule::HandleEndPIE(const bool InIsSimulating)
{
	SetActive(bWasActiveBeforePIE);
}

#endif	// WITH_EDITOR

void FManusModule::SetGlovesUsingTrackers(bool p_UseTrackers)
{
    m_UseTrackersForGloves = p_UseTrackers;
}

bool FManusModule::GetGlovesUsingTrackers()
{
    return m_UseTrackersForGloves;
}

#undef LOCTEXT_NAMESPACE
