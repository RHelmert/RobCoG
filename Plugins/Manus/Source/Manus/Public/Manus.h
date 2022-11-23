// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "ManusLiveLinkSource.h"
#include "ManusLiveLinkUser.h"
#include "ManusTrackingManager.h"
#include "UObject/StrongObjectPtr.h"
#include "Runtime/Launch/Resources/Version.h"
#include "PluginDescriptor.h"

DECLARE_LOG_CATEGORY_EXTERN(LogManus, Log, Log);

DECLARE_STATS_GROUP(TEXT("Manus"), STATGROUP_Manus, STATCAT_Advanced);


/**
 * The main module for the plugin that implements Manus glove support.
 */
class MANUS_API FManusModule : public IModuleInterface
{
public:
	/**
	 * Singleton-like access to this module's interface.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FManusModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FManusModule>("Manus");
	}

	/**
	 * Module startup.
	 */
	virtual void StartupModule() override;

	/**
	 * Module shutdown.
	 */
	virtual void ShutdownModule() override;

	/**
	 * Check whether the plugin is currently active.
	 */
	bool IsActive() { return bIsActive; }

	/**
	 * Set whether the plugin is currently active.
	 */
	void SetActive(bool bNewIsActive);

	/**
	 * Called when a new player logs in.
	 */
	void OnGameModePostLogin(class AGameModeBase* GameMode, APlayerController* NewPlayer);

	/**
	 * Called when a player logs out.
	 */
	void OnGameModeLogout(class AGameModeBase* GameMode, AController* Exiting);

	/**
	 * Get the data from the uplugin file.
	 */
	static bool GetPluginData(FPluginDescriptor& PluginData);

#if WITH_EDITOR
	/** On PIE began. */
	void HandleBeginPIE(const bool InIsSimulating);

	/** On PIE ended. */
	void HandleEndPIE(const bool InIsSimulating);
#endif // WITH_EDITOR

	/**
	 * Get the Live Link source.
	 *
	 * @return Returns the Live Link source.
	 */
	virtual TSharedPtr<ILiveLinkSource> GetLiveLinkSource(EManusLiveLinkSourceType SourceType)
	{
		if (IsActive() && !LiveLinkSources[(int)SourceType].IsValid())
		{
			LiveLinkSources[(int)SourceType] = TSharedPtr<FManusLiveLinkSource>(new FManusLiveLinkSource(SourceType));
			LiveLinkSources[(int)SourceType]->Init();
		}
		return LiveLinkSources[(int)SourceType];
	}

	/**
	 * Get whether the Live Link source is valid.
	 *
	 * @return Returns whether the Live Link source is valid.
	 */
	virtual bool IsLiveLinkSourceValid(EManusLiveLinkSourceType SourceType)
	{
		return LiveLinkSources[(int)SourceType].IsValid();
	}

	/**
	 * Get the Live Link source.
	 *
	 * @return Returns the Live Link source.
	 */
	void OnLiveLinkSourceRemoved(FGuid SourceGuid)
	{
		for (int i = 0; i < (int)EManusLiveLinkSourceType::Max; i++)
		{
			if (LiveLinkSources[i] && LiveLinkSources[i].IsValid() && LiveLinkSources[i]->LiveLinkSourceGuid == SourceGuid)
			{
				LiveLinkSources[i].Reset();
			}
		}
	}

	/**
	 * Get the Tracking Manager.
	 *
	 * @return Returns the Tracking Manager.
	 */
	virtual UManusTrackingManager* GetTrackingManager()
	{
		if (!TrackingManager)
		{
			TrackingManager = TStrongObjectPtr<UManusTrackingManager>(NewObject<UManusTrackingManager>(GetTransientPackage(), UManusTrackingManager::StaticClass()));
			TrackingManager->Initialize();
		}
		return TrackingManager.Get();
	}

	/**
	* Returns the Manus Live Link User index of the given pair of ManusDashboardUserIndex and ManusSkeleton.
	* @return The Manus Live Link User index of the given pair of ManusDashboardUserIndex and ManusSkeleton.
	*/
	int GetManusLiveLinkUserIndex(int ManusDashboardUserIndex, class UManusSkeleton* ManusSkeleton);

	/**
	* Returns the Manus Live Link User at the given index.
	* @return The Manus Live Link User at the given index.
	*/
	FManusLiveLinkUser& GetManusLiveLinkUser(int Index);

	/**
	* Adds a new object to the list of objects using a Manus Live Link user.
	*/
	void AddObjectUsingManusLiveLinkUser(int ManusDashboardUserIndex, class UManusSkeleton* ManusSkeleton, UObject* Object);

	/**
	* Removes an object from the list of objects using a Manus Live Link user.
	*/
	void RemoveObjectUsingManusLiveLinkUser(int ManusDashboardUserIndex, class UManusSkeleton* ManusSkeleton, UObject* Object);

	/**
	* Returns whether there is any object using the Manus Live Link user.
	*/
	bool IsAnyObjectUsingManusLiveLinkUser(int ManusLiveLinkUserIndex);

	/**
	* Returns whether there is any replicating object using the Manus Live Link user.
	*/
	bool IsAnyReplicatingObjectUsingManusLiveLinkUser(int ManusLiveLinkUserIndex);

public:
	/** The map of the Manus Replicators */
	TMap<class AController*, class AManusReplicator*> Replicators;

	/** The Manus Live Link Users. */
	TArray<FManusLiveLinkUser> ManusLiveLinkUsers;

private:
	bool bIsActive = false;

#if WITH_EDITOR
	bool bWasActiveBeforePIE = false;
#endif // WITH_EDITOR

	/** The Live Link sources. */
	TSharedPtr<FManusLiveLinkSource> LiveLinkSources[(int)EManusLiveLinkSourceType::Max];

	/** The Tracking manager. */
	TStrongObjectPtr<UManusTrackingManager> TrackingManager;
};