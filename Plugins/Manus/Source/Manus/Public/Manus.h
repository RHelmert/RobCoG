// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2022 Manus

#pragma once

// Set up a Doxygen group.
/** @addtogroup ManusModule
 *  @{
 */

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "ManusLiveLinkSource.h"
#include "ManusLiveLinkUser.h"
#include "UObject/StrongObjectPtr.h"
#include "Runtime/Launch/Resources/Version.h"
#include "PluginDescriptor.h"

DECLARE_LOG_CATEGORY_EXTERN(LogManus, Log, Log);

DECLARE_STATS_GROUP(TEXT("Manus"), STATGROUP_Manus, STATCAT_Advanced);


/// @brief The main module for the plugin that implements %Manus glove support.
/// most of these are based on the IModuleInterface from unreal. so for more details check the unreal documentation
class MANUS_API FManusModule : public IModuleInterface
{
public:
	/// @brief Singleton-like access to this module's interface.
	// @Returns singleton instance, loading the module on demand if needed
	static inline FManusModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FManusModule>("Manus");
	}

    /// @brief Module startup.
	virtual void StartupModule() override;

    /// @brief Module shutdown.
	virtual void ShutdownModule() override;

    /// @brief Check whether the plugin is currently active.
	bool IsActive() { return bIsActive; }

    /// @brief Set whether the plugin is currently active.
	void SetActive(bool bNewIsActive);

	/// @brief Check whether the plugin is currently connected.
	bool IsConnected();

    /// @brief Called when a new player logs in. spawns a %Manus replicator.
	/// @param GameMode 
	/// @param NewPlayer 
	void OnGameModePostLogin(class AGameModeBase* GameMode, APlayerController* NewPlayer);

    /// @brief Called when a player logs out and destroys %Manus replicator
	void OnGameModeLogout(class AGameModeBase* GameMode, AController* Exiting);

    /// @brief Get the plugin data from the uplugin file.
	/// @param PluginData 
	/// @return true if version is not empty
	static bool GetPluginData(FPluginDescriptor& PluginData);

    /// @brief 
    /// @returns if this is an unreal networked client or not
    bool IsClient();

    /// @brief Set the current session if it is an unreal client.
    void SetAsClient(bool p_IsClient);

    /// @brief set the %Manus Core IP address (usually from the UI to this)
	/// @param p_ManusIP 
    /// @returns true upon succesfully set
	bool SetManusCoreIP(FString p_ManusIP);

    /// @brief Get the currently set %Manus Core host IP.
    /// @returns currently set %Manus Core ip
	FString GetManusCoreIP();

    /// @brief Get a list of all remote %Manus Core hosts. Don't use this when connected.
	/// @param p_Hosts fills all the available hosts in the TArray.
	void GetRemoteHosts(TArray<FString>& p_Hosts);

#if WITH_EDITOR
    /// @brief On PIE began. adjusts %Manus active state
	/// @param InIsSimulating 
	void HandleBeginPIE(const bool InIsSimulating);

    /// @brief On PIE ended. adjusts %Manus active state
	/// @param InIsSimulating 
	void HandleEndPIE(const bool InIsSimulating);
#endif // WITH_EDITOR

    /// @brief Get the Live Link source.
	/// @param SourceType 
    /// @Returns the Live Link source.
	virtual TSharedPtr<ILiveLinkSource> GetLiveLinkSource(EManusLiveLinkSourceType SourceType)
	{
		if (IsActive() && !LiveLinkSources[(int)SourceType].IsValid())
		{
			LiveLinkSources[(int)SourceType] = TSharedPtr<FManusLiveLinkSource>(new FManusLiveLinkSource(SourceType));
			LiveLinkSources[(int)SourceType]->Init();
		}
		return LiveLinkSources[(int)SourceType];
	}

    /// @brief Get whether the Live Link source is valid.
	/// @param SourceType 
    /// @Returns whether the Live Link source is valid.
	virtual bool IsLiveLinkSourceValid(EManusLiveLinkSourceType SourceType)
	{
		return LiveLinkSources[(int)SourceType].IsValid();
	}

    /// @brief Get the Live Link source.
	/// @param SourceGuid 
    /// @Returns the Live Link source.
	void OnLiveLinkSourceRemoved(FGuid SourceGuid)
	{
		for (int i = 0; i < (int)EManusLiveLinkSourceType::Max; i++)
		{
			if (LiveLinkSources[i] && LiveLinkSources[i].IsValid() && LiveLinkSources[i]->liveLinkSourceGuid == SourceGuid)
			{
				LiveLinkSources[i].Reset();
			}
		}
	}

    /// @brief Returns the %Manus Live Link User index of the given pair of ManusDashboardUserIndex and ManusSkeleton.
	/// @param ManusDashboardUserIndex 
	/// @param ManusSkeleton 
	/// @return user index
	int GetManusLiveLinkUserIndex(int ManusDashboardUserIndex, class UManusSkeleton* ManusSkeleton);

    /// @brief Returns the %Manus Live Link User at the given index.
	/// @param Index 
	/// @return FManusLiveLinkUser found or INDEX_NONE if not found.
	FManusLiveLinkUser& GetManusLiveLinkUser(int Index);

    /// @brief Adds a new object to the list of objects using a %Manus Live Link user based upon the dashboarduserindex and manusskeleton.
	/// @param ManusDashboardUserIndex 
	/// @param ManusSkeleton 
	/// @param Object 
	void AddObjectUsingManusLiveLinkUser(int ManusDashboardUserIndex, class UManusSkeleton* ManusSkeleton, UObject* Object);

    /// @brief Removes an object from the list of objects using a %Manus Live Link user based upon the dashboarduserindex and manusskeleton.
	/// @param ManusDashboardUserIndex 
	/// @param ManusSkeleton 
	/// @param Object 
	void RemoveObjectUsingManusLiveLinkUser(int ManusDashboardUserIndex, class UManusSkeleton* ManusSkeleton, UObject* Object);

    /// @brief Returns whether there is any object using the %Manus Live Link user.
	/// @param ManusLiveLinkUserIndex 
	/// @return true if used.
	bool IsAnyObjectUsingManusLiveLinkUser(int ManusLiveLinkUserIndex);

    /// @brief Returns whether there is any replicating object using the %Manus Live Link user.
	/// @param ManusLiveLinkUserIndex 
	/// @return true if used
	bool IsAnyReplicatingObjectUsingManusLiveLinkUser(int ManusLiveLinkUserIndex);

    /// @brief This function is for the Demo only to indicate trackers are being used.
    /// @param p_UseTrackers 
    void SetGlovesUsingTrackers(bool p_UseTrackers);

    /// @brief This function is for the Demo only
    /// @return true if using trackers.
    bool GetGlovesUsingTrackers();

public:
    /// @brief The map of the %Manus Replicators 
	TMap<class AController*, class AManusReplicator*> Replicators;

    /// @brief The %Manus Live Link Users. 
	TArray<FManusLiveLinkUser> ManusLiveLinkUsers;

private:
    /// @brief is %Manus SDK setup
	bool bIsActive = false;
	/// @brief is %Manus SDK connected to a Host
	bool bIsConnected = false;
    /// @brief for demo purposes only. 
    bool m_UseTrackersForGloves = false;
    /// @brief current list of %Manus Core hosts found
	TArray<FString> m_Hosts;
    /// @brief current set %Manus Core host ip.
	FString m_ManusIP;
    /// @brief indicates wether it is a network client or not.
    bool m_IsClient;

#if WITH_EDITOR
	bool bWasActiveBeforePIE = false;
#endif // WITH_EDITOR

    /// @brief The Live Link sources. 
	TSharedPtr<FManusLiveLinkSource> LiveLinkSources[(int)EManusLiveLinkSourceType::Max];
};

// Close the Doxygen group.
/** @} */
