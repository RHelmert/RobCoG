// Copyright 2015-2022 Manus

#include "ManusLiveLinkSource.h"
#include "Manus.h"
#include "CoreSdk.h"
#include "ManusSettings.h"
#include "ManusBlueprintLibrary.h"
#include "ManusBlueprintTypes.h"
#include "ManusReplicator.h"
#include "ManusComponent.h"
#include "ManusTools.h"
#include "ManusConvert.h"
#include "ManusSkeleton.h"
#include "ManusTypeInitializers.h"


#if WITH_EDITOR
#include "ManusEditorUserSettings.h"
#include <Runtime/Slate/Public/Widgets/Notifications/SNotificationList.h>
#include <Runtime/Slate/Public/Framework/Notifications/NotificationManager.h> 
#endif // WITH_EDITOR

#include "LiveLinkClient.h"
#include "LiveLinkSourceFactory.h"
#include "ILiveLinkClient.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Features/IModularFeatures.h"
#include <regex>

DECLARE_CYCLE_STAT(TEXT("Manus Update Glove Assignments"), STAT_Manus_UpdateGloveAssignments, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("Manus Update Live Link"), STAT_Manus_UpdateLiveLink, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("Manus Update Skeletons Live Link"), STAT_Manus_UpdateSkeletonsLiveLink, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("Manus Update Glove Live Link"), STAT_Manus_UpdateGloveLiveLink, STATGROUP_Manus);

#define LOCTEXT_NAMESPACE "FManusModule"

FManusLiveLinkSource::FManusLiveLinkSource(EManusLiveLinkSourceType InSourceType)
: m_SourceType(InSourceType)
{
}

void FManusLiveLinkSource::Init()
{
	// Init Manus replicated Live Link source
	FLiveLinkClient* t_Client = &IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(FLiveLinkClient::ModularFeatureName);
	if (t_Client)
	{
		t_Client->AddSource(FManusModule::Get().GetLiveLinkSource(m_SourceType));
	}
}

void FManusLiveLinkSource::Destroy()
{
	// pre clean skeletons in case of temp skeletons.
	TArray<FManusLiveLinkUser>& t_ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	for (size_t i = 0; i < t_ManusLiveLinkUsers.Num(); i++)
	{
		UManusSkeleton* t_ManusSkeleton = t_ManusLiveLinkUsers[i].ManusSkeleton;
		if (t_ManusSkeleton)
		{
			t_ManusSkeleton->SetTemporarySkeletonIndex(UINT32_MAX);
		}
	}

	if (m_LiveLinkClient)
	{
		m_LiveLinkClient->RemoveSource(liveLinkSourceGuid);
	}

	// todo more cleanup?
}

bool FManusLiveLinkSource::TickConnection()
{
    // Take care of restarting Core connection when needed
    EManusRet t_ReturnCode = CoreSdk::CheckConnection();

    if (t_ReturnCode != EManusRet::Success)
    {
        FString t_ManusIP = FManusModule::Get().GetManusCoreIP();
        if (t_ManusIP.IsEmpty() ) return false; // no manus core ip set just yet. so lets ignore.

        // Only log the time out one time
        if (!m_bIsConnectionWithCoreTimingOut)
        {
            UE_LOG(LogManus, Warning, TEXT("Connection with Manus Core timed out. Please make sure that Manus Core is up and running."));

            // prepare for reconnect just in case.
            t_ReturnCode = CoreSdk::ShutDown();
            if (t_ReturnCode != EManusRet::Success) return false;
            t_ReturnCode = CoreSdk::Initialize();
            if (t_ReturnCode != EManusRet::Success) return false;
        }

        // Connection with Core is timing out
        m_bIsConnectionWithCoreTimingOut = true;
        m_bIsSkeletonsInitialized = false;

        // check if local or remote
        // since we cannot put a sleep in here but don't want to spam the system to death with reconnects
        // we keep a counter and when it triggers we check again. Not perfect, but not the worst either.
        if (m_ReconnectCounter <= 0)
        {

            m_ReconnectCounter = RECONNECT_RETRY_COUNT_MAX;
            if ((t_ManusIP.Compare("LocalHost") == 0) ||
                (t_ManusIP.Compare("127.0.0.1") == 0))
            {
                t_ReturnCode = CoreSdk::ConnectLocally();
            }
            else
            {
                t_ReturnCode = CoreSdk::ConnectRemotely(t_ManusIP);
            }

            if (t_ReturnCode != EManusRet::Success) return false;
        }
        else
        {
            m_ReconnectCounter--;
            return false;
        }
    }
    else if (m_bIsConnectionWithCoreTimingOut)
    {
        // Connection with Core is back
        m_bIsConnectionWithCoreTimingOut = false;
        UE_LOG(LogManus, Log, TEXT("Connection with Manus Core restored."));

        // Reinitialize Skeletons
        TArray<FManusLiveLinkUser>& t_ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
        InitSkeletons(true, t_ManusLiveLinkUsers);
    }
    return true;
}

void FManusLiveLinkSource::TickUpdateSkeletons()
{
    TArray<FManusLiveLinkUser>& t_ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;

    // update any temporary skeleton updates
    uint32_t t_TempSkeletonIndex = UINT32_MAX;
    CoreSdk::GetLastModifiedSkeletonIndex(t_TempSkeletonIndex);
    if (t_TempSkeletonIndex != UINT32_MAX)
    {
        // ok find the correct manusskeleton			
        for (size_t i = 0; i < t_ManusLiveLinkUsers.Num(); i++)
        {
            UManusSkeleton* t_ManusSkeleton = t_ManusLiveLinkUsers[i].ManusSkeleton;
            if (t_ManusSkeleton->GetTemporarySkeletonIndex() == t_TempSkeletonIndex)
            {
                // and let it update
                t_ManusSkeleton->RetrieveTemporarySkeleton();
                
                // and indicate that it should be rebuilt.
                t_ManusLiveLinkUsers[i].SkeletonsInitializationRetryCountdown = 1; // enable it to be reloaded.			
                
#if WITH_EDITOR
                // and notify user

                FNotificationInfo Info(LOCTEXT("HotReloadFinished", "Hot reload of Manus Skeleton complete!"));
                Info.FadeInDuration = 0.1f;
                Info.FadeOutDuration = 0.5f;
                Info.ExpireDuration = 5.0f;
                Info.bUseThrobber = false;
                Info.bUseSuccessFailIcons = true;
                Info.bUseLargeFont = true;
                Info.bFireAndForget = false;
                Info.bAllowThrottleWhenFrameRateIsLow = false;
                auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
                NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
                NotificationItem->ExpireAndFadeout();
#endif
                break;
            }
        }
    }
}

void FManusLiveLinkSource::Tick(float p_DeltaTime)
{
	if (m_SourceType == EManusLiveLinkSourceType::Local)
	{
        if (!TickConnection()) return; //continue of true, abort if false.

		// Update Manus Live Link Users
		TArray<FManusLiveLinkUser>& t_ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;

		// Reset replicated data
		m_ReplicatedFrameDataArray.Reset();

        TickUpdateSkeletons();

		InitSkeletons(false, t_ManusLiveLinkUsers);

		// Update Live Link for each Manus Live Link User
		for (int i = 0; i < t_ManusLiveLinkUsers.Num(); i++)
		{
            if (FManusModule::Get().ManusLiveLinkUsers.IsValidIndex(i))
            {
                UpdateLiveLink(p_DeltaTime, i);
            }
		}
		m_bNewLiveLinkClient = false;

		// Remove potential ghost Live Link subjects left there after removing Manus Live Link Users
		int t_ManusLiveLinkUserIndexToRemove = t_ManusLiveLinkUsers.Num();

        // TODO is this valid code? whut ?
		while (m_LiveLinkClient && m_LiveLinkClient->IsSubjectEnabled(FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(t_ManusLiveLinkUserIndexToRemove)))
		{
			FLiveLinkSubjectName t_LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(t_ManusLiveLinkUserIndexToRemove);
			FLiveLinkSubjectKey t_LiveLinkSubjectKey = FLiveLinkSubjectKey(liveLinkSourceGuid, t_LiveLinkSubjectName);
			m_LiveLinkClient->RemoveSubject_AnyThread(t_LiveLinkSubjectKey);
			t_ManusLiveLinkUserIndexToRemove++;
		}
	}
    else // if (m_SourceType == EManusLiveLinkSourceType::Replicated)
    {
        if (FManusModule::Get().IsClient() == false) return;
        //UE_LOG(LogManus, Warning, TEXT("Replicated stuff on client."));
        TArray<FManusLiveLinkUser>& t_ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
        // update the replicated ones
      
        for (int i = 0; i < t_ManusLiveLinkUsers.Num(); i++)
        {
            if (FManusModule::Get().ManusLiveLinkUsers.IsValidIndex(i))
            {
                FManusLiveLinkUser& t_ManusLiveLinkUser = t_ManusLiveLinkUsers[i];
                // do we need to (re)init
                if (t_ManusLiveLinkUser.SkeletonsInitializationRetryCountdown > 0)
                {
                    UManusSkeleton* t_ManusSkeleton = t_ManusLiveLinkUser.ManusSkeleton;

                    if (t_ManusLiveLinkUser.LiveLinkFrame == NULL)
                    {
                        t_ManusLiveLinkUser.LiveLinkFrame = new FLiveLinkFrameDataStruct();
                        t_ManusLiveLinkUser.LiveLinkFrame->InitializeWith(FLiveLinkAnimationFrameData::StaticStruct(), nullptr);
                    }

                    if (t_ManusSkeleton &&
                        t_ManusSkeleton->GetSkeleton())// do we even have a skeleton?
                    {
                        m_bIsSkeletonsInitialized = true;
                        t_ManusLiveLinkUser.SkeletonsInitializationRetryCountdown = 0; // ok we're done 
                    }

                        
                    // recreate livelink if needed
                    t_ManusLiveLinkUser.bShouldUpdateLiveLinkData = false;

                    // Update live link for this Live Link user if some objects are using it
                    if (!t_ManusLiveLinkUser.bShouldUpdateLiveLinkData)
                    {
                        t_ManusLiveLinkUser.bShouldUpdateLiveLinkData |= FManusModule::Get().IsAnyObjectUsingManusLiveLinkUser(i);
                    }

                    // Update Live Link only when necessary
                    if (m_LiveLinkClient && t_ManusLiveLinkUser.bShouldUpdateLiveLinkData)
                    {
                        FLiveLinkSubjectName t_LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(i);
                        FLiveLinkSubjectKey t_LiveLinkSubjectKey = FLiveLinkSubjectKey(liveLinkSourceGuid, t_LiveLinkSubjectName);

                        // Reinit subject when the Live Link client changes
                        if (m_bNewLiveLinkClient ||
                            !m_LiveLinkClient->IsSubjectEnabled(t_LiveLinkSubjectName))
                        {
                            RecreateLiveLinkSubject(t_LiveLinkSubjectKey, i);
                        }
                    }
                }
            }
        }
    }
}

void FManusLiveLinkSource::InitSkeletons(bool p_ResetRetry, TArray<FManusLiveLinkUser>& p_ManusLiveLinkUsers)
{
    //UE_LOG(LogManus, Warning, TEXT("Starting InitSkeletons."));
	int32 t_NumberOfUsers = 0;
	EManusRet t_Ret = CoreSdk::GetNumberOfAvailableUsers(t_NumberOfUsers);
	if (t_Ret != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to get user count."));
		return;
	}
	if (t_NumberOfUsers == 0) return; // this is normal during startup as no landscape data has yet been processed

	// TODO make this more flexible based on the target type,  we may skip this part entirely and just get it via the skeleton.
	TArray<int64> t_IdsOfAvailableUsers;
	t_IdsOfAvailableUsers.Reserve(t_NumberOfUsers);
	t_Ret = CoreSdk::GetIdsOfUsers(t_IdsOfAvailableUsers);
	if (t_Ret != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to get users."));
		return;
	}

	// we can overwrite old skeletons, so set flag to allowit.
	if (p_ResetRetry || !m_bIsSkeletonsInitialized)
	{
		for (int i = 0; i < p_ManusLiveLinkUsers.Num(); i++)
		{
			p_ManusLiveLinkUsers[i].SkeletonsInitializationRetryCountdown = 1; // enable it to be reloaded.
		}
	}
	// now init and connect the skeletons.
	for (int i = 0; i < p_ManusLiveLinkUsers.Num(); i++)
	{
		if (p_ManusLiveLinkUsers[i].SkeletonsInitializationRetryCountdown > 0)
		{
			p_ManusLiveLinkUsers[i].SkeletonsInitializationRetryCountdown--;
			if (p_ManusLiveLinkUsers[i].SkeletonsInitializationRetryCountdown <= 0)
			{
				InitSkeletonsForManusLiveLinkUser(i, t_IdsOfAvailableUsers[0], p_ResetRetry); // todo the users must be setup better. for now we just use 1 user.				
			}
		}
	}
}

TStatId FManusLiveLinkSource::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FManusLiveLinkSource, STATGROUP_Tickables);
}

void FManusLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	m_LiveLinkClient = InClient;
	liveLinkSourceGuid = InSourceGuid;
	m_bNewLiveLinkClient = true;

	if (ULiveLinkSourceSettings* Settings = static_cast<FLiveLinkClient*>(InClient)->GetSourceSettings(liveLinkSourceGuid))
	{
		Settings->ConnectionString = FString();
	}

	if (m_SourceType == EManusLiveLinkSourceType::Local)
	{
		SetBufferOffset(GetDefault<UManusSettings>()->TrackingSmoothing);
	}
	else if (m_SourceType == EManusLiveLinkSourceType::Replicated)
	{
		SetBufferOffset(GetDefault<UManusSettings>()->DefaultReplicationOffsetTime);
	}
}

bool FManusLiveLinkSource::IsSourceStillValid() const
{
	return m_LiveLinkClient != nullptr;
}

bool FManusLiveLinkSource::RequestSourceShutdown()
{
	m_LiveLinkClient = nullptr;
	liveLinkSourceGuid.Invalidate();
	return true;
}

FText FManusLiveLinkSource::GetSourceMachineName() const
{
	return FText().FromString(FPlatformProcess::ComputerName());
}

FText FManusLiveLinkSource::GetSourceStatus() const
{
	return LOCTEXT("ManusLiveLinkStatus", "Active");
}

FText FManusLiveLinkSource::GetSourceType() const
{
	FText t_SourceTypeName;
	switch (m_SourceType)
	{
		case EManusLiveLinkSourceType::Local:		t_SourceTypeName = FText::FromString("Manus");					break;
		case EManusLiveLinkSourceType::Replicated:	t_SourceTypeName = FText::FromString("Manus Replicated");		break;
	}
	return t_SourceTypeName;
}

void FManusLiveLinkSource::SetBufferOffset(float p_Offset)
{
	FLiveLinkClient* t_Client = &IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(FLiveLinkClient::ModularFeatureName);
	if (t_Client)
	{
		if (ULiveLinkSourceSettings* Settings = t_Client->GetSourceSettings(liveLinkSourceGuid))
		{
			Settings->BufferSettings.EngineTimeOffset = p_Offset;
			Settings->BufferSettings.MaxNumberOfFrameToBuffered = FMath::Max(10, (int)(p_Offset * 90));
		}
	}
}

FName FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(const UManusComponent* p_ManusComponent)
{
	if (p_ManusComponent)
	{
        int t_ManusLiveLinkUserIndex = INDEX_NONE;
        if (p_ManusComponent->ManusSkeleton != NULL)
        {
            t_ManusLiveLinkUserIndex = FManusModule::Get().GetManusLiveLinkUserIndex(p_ManusComponent->ManusSkeleton->TargetUserIndexData.userIndex, p_ManusComponent->ManusSkeleton);
        }
		if (t_ManusLiveLinkUserIndex != INDEX_NONE)
		{
			if (p_ManusComponent->IsLocallyOwned())
			{
				return GetManusLiveLinkUserLiveLinkSubjectName(t_ManusLiveLinkUserIndex);
			}
			else
			{
				return GetManusLiveLinkUserLiveLinkSubjectName(t_ManusLiveLinkUserIndex, p_ManusComponent->ManusReplicatorId);
			}
		}
	}
	return GetManusLiveLinkUserLiveLinkSubjectName(INDEX_NONE);
}

FName FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(int p_ManusLiveLinkUserIndex, int32 p_ReplicatorId)
{
	if (p_ManusLiveLinkUserIndex < 0 || p_ManusLiveLinkUserIndex > 99)
	{
		UE_LOG(LogManus, Warning, TEXT("The Manus Live Link User index is incorrect. The Manus Live Link User Live Link subject name will be set to None."));
		return FName(TEXT("None"));
	}

	bool t_IsReplicated = (p_ReplicatorId != 0);

	FString t_Name = t_IsReplicated ? "ManusReplicatedUser_" : "ManusLiveLinkUser_";
	if (t_IsReplicated)
	{
		t_Name += FString::FromInt(p_ReplicatorId);
		t_Name += "_";
	}
	if (p_ManusLiveLinkUserIndex < 10)
	{
		t_Name += "0";
	}
	t_Name += FString::FromInt(p_ManusLiveLinkUserIndex);

	return FName(*t_Name);
}

void FManusLiveLinkSource::ReplicateLiveLink(class AManusReplicator* p_Replicator)
{
#if ENGINE_MAJOR_VERSION == 5 
	if (IsValid(p_Replicator)) // (!p_Replicator->IsPendingKill()) // ispendingkill is beyond deprecated and will block future compiling.
#else
	if (!p_Replicator->IsPendingKill()) 
#endif
	{
		// For each Manus Live Link User
		for (int i = 0; i < p_Replicator->ReplicatedData.ReplicatedFrameDataArray.Num(); i++)
		{
			int t_ManusLiveLinkUserIndex = FManusModule::Get().GetManusLiveLinkUserIndex(p_Replicator->ReplicatedData.ReplicatedFrameDataArray[i].ManusDashboardUserIndex, p_Replicator->ReplicatedData.ReplicatedFrameDataArray[i].ManusSkeleton);

			// Create or recreate subject (if needed)
			FLiveLinkSubjectName t_LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(t_ManusLiveLinkUserIndex, p_Replicator->ReplicatorId);
			FLiveLinkSubjectKey t_LiveLinkSubjectKey = FLiveLinkSubjectKey(liveLinkSourceGuid, t_LiveLinkSubjectName);
			if (!m_LiveLinkClient->IsSubjectEnabled(t_LiveLinkSubjectName))
			{
				RecreateLiveLinkSubject(t_LiveLinkSubjectKey, t_ManusLiveLinkUserIndex);
			}

			// Update Frame
			FManusLiveLinkUser& t_LiveLinkUser = FManusModule::Get().GetManusLiveLinkUser(t_ManusLiveLinkUserIndex);

			FLiveLinkAnimationFrameData* t_LiveLinkFramePtr = t_LiveLinkUser.LiveLinkFrame->Cast<FLiveLinkAnimationFrameData>();
			t_LiveLinkFramePtr->WorldTime = FPlatformTime::Seconds();
			t_LiveLinkFramePtr->MetaData.SceneTime = FApp::GetCurrentFrameTime().Get(FQualifiedFrameTime());

			// Update the transforms from the replicated data
			AManusReplicator::DecompressReplicatedFrameData(*t_LiveLinkFramePtr, p_Replicator->ReplicatedData.ReplicatedFrameDataArray[i]);

			// Copy the data locally and share it with the LiveLink client
			FLiveLinkFrameDataStruct t_NewLiveLinkFrame;
			t_NewLiveLinkFrame.InitializeWith(*t_LiveLinkUser.LiveLinkFrame);
			m_LiveLinkClient->PushSubjectFrameData_AnyThread(t_LiveLinkSubjectKey, MoveTemp(t_NewLiveLinkFrame));
		}

		// Update replicated data buffer offset according to current ping
		const UManusSettings* t_ManusSettings = GetDefault<UManusSettings>();
		if (t_ManusSettings && t_ManusSettings->bUpdateReplicationOffsetTimeUsingPing)
		{
			FLiveLinkClient* t_Client = &IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(FLiveLinkClient::ModularFeatureName);
			if (t_Client)
			{
				if (ULiveLinkSourceSettings* Settings = t_Client->GetSourceSettings(liveLinkSourceGuid))
				{
					if (APlayerController* t_PlayerController = p_Replicator->GetWorld()->GetFirstPlayerController())
					{
						if (APlayerState* PlayerState = t_PlayerController->PlayerState)
						{
							float t_DesiredOffset = PlayerState->ExactPing * 0.001f * t_ManusSettings->ReplicationOffsetTimePingMultiplier + t_ManusSettings->ReplicationOffsetTimePingExtraTime;
							SetBufferOffset(Settings->BufferSettings.EngineTimeOffset * 0.995f + t_DesiredOffset * 0.005f);
						}
					}
				}
			}
		}
	}
}

void FManusLiveLinkSource::StopReplicatingLiveLink(class AManusReplicator* p_Replicator)
{
	int t_ManusLiveLinkUserIndexToRemove = 0;
	while (m_LiveLinkClient && m_LiveLinkClient->IsSubjectEnabled(FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(t_ManusLiveLinkUserIndexToRemove, p_Replicator->ReplicatorId)))
	{
		FLiveLinkSubjectName t_LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(t_ManusLiveLinkUserIndexToRemove, p_Replicator->ReplicatorId);
		FLiveLinkSubjectKey t_LiveLinkSubjectKey = FLiveLinkSubjectKey(liveLinkSourceGuid, t_LiveLinkSubjectName);
		m_LiveLinkClient->RemoveSubject_AnyThread(t_LiveLinkSubjectKey);
		t_ManusLiveLinkUserIndexToRemove++;
	}
}

void FManusLiveLinkSource::UpdateLiveLink(float p_DeltaTime, int p_ManusLiveLinkUserIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_Manus_UpdateLiveLink);

	check(IsInGameThread());

    //UE_LOG(LogManus, Warning, TEXT("updating livelink."));

	TArray<FManusLiveLinkUser>& t_ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].bShouldUpdateLiveLinkData = false;

	// Update live link for this Live Link user if some objects are using it
	if (!t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].bShouldUpdateLiveLinkData)
	{
		t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].bShouldUpdateLiveLinkData |= FManusModule::Get().IsAnyObjectUsingManusLiveLinkUser(p_ManusLiveLinkUserIndex);
	}

	// Update Live Link only when necessary
	if (m_LiveLinkClient && t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].bShouldUpdateLiveLinkData)
	{
		FLiveLinkSubjectName t_LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(p_ManusLiveLinkUserIndex);
		FLiveLinkSubjectKey t_LiveLinkSubjectKey = FLiveLinkSubjectKey(liveLinkSourceGuid, t_LiveLinkSubjectName);

		// Reinit subject when the Live Link client changes
		if (m_bNewLiveLinkClient || 
			!m_LiveLinkClient->IsSubjectEnabled(t_LiveLinkSubjectName) )
		{
            //UE_LOG(LogManus, Warning, TEXT("updating livelink. A"));
			RecreateLiveLinkSubject(t_LiveLinkSubjectKey, p_ManusLiveLinkUserIndex);
		}

		// Update Frame
		if (t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame == NULL) return;
		FLiveLinkAnimationFrameData* t_LiveLinkFramePtr = t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame->Cast<FLiveLinkAnimationFrameData>();
		if (!t_LiveLinkFramePtr) return;

        //UE_LOG(LogManus, Warning, TEXT("updating livelink. B"));

		// Update the transforms for each subject from tracking data
        uint64 LastUpdateTime = 0;
		
		UManusSkeleton* t_ManusSkeleton = t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].ManusSkeleton;
		if (t_ManusSkeleton->ManusSkeletonId == 0)  
		{
			return; // nothing to do here. so don't bother.
		}

		if (!UpdateSkeletonsLiveLinkTransforms(p_ManusLiveLinkUserIndex, t_LiveLinkFramePtr->Transforms, LastUpdateTime))
		{
			return; // there was no data.
		}		
        
		// UE_LOG(LogManus, Warning, TEXT("updating livelink. C"));
		t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].ManusDataLastUpdateTime = LastUpdateTime;

		// Frame time
		t_LiveLinkFramePtr->WorldTime = FPlatformTime::Seconds();
		t_LiveLinkFramePtr->MetaData.SceneTime = FApp::GetCurrentFrameTime().Get(FQualifiedFrameTime());

		// Copy the data locally and share it with the LiveLink client
		FLiveLinkFrameDataStruct NewLiveLinkFrame;
		NewLiveLinkFrame.InitializeWith(*t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame);
		m_LiveLinkClient->PushSubjectFrameData_AnyThread(t_LiveLinkSubjectKey, MoveTemp(NewLiveLinkFrame));
            
        // Copy the transforms locally to replicate them (if necessary)
        if (FManusModule::Get().IsAnyReplicatingObjectUsingManusLiveLinkUser(p_ManusLiveLinkUserIndex))
        {
            int t_ReplicatedFrameDataArrayIndex = m_ReplicatedFrameDataArray.AddDefaulted();
            m_ReplicatedFrameDataArray[t_ReplicatedFrameDataArrayIndex].ManusDashboardUserIndex = t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].ManusDashboardUserIndex;
            m_ReplicatedFrameDataArray[t_ReplicatedFrameDataArrayIndex].ManusSkeleton = t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].ManusSkeleton;

            AManusReplicator::CompressReplicatedFrameData(*t_LiveLinkFramePtr, m_ReplicatedFrameDataArray[t_ReplicatedFrameDataArrayIndex]);
        }
	}
}

void FManusLiveLinkSource::RecreateLiveLinkSubject(FLiveLinkSubjectKey& p_LiveLinkSubjectKey, int p_ManusLiveLinkUserIndex)
{
	m_LiveLinkClient->RemoveSubject_AnyThread(p_LiveLinkSubjectKey);

	
	TArray<FManusLiveLinkUser>& t_ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	UManusSkeleton* ManusSkeleton = t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].ManusSkeleton;

	if ((ManusSkeleton != NULL) &&
		(ManusSkeleton->GetSkeleton()!=NULL))
	{
		// Skeleton data
		FLiveLinkStaticDataStruct SkeletalData(FLiveLinkSkeletonStaticData::StaticStruct());
		FLiveLinkSkeletonStaticData* SkeletonDataPtr = SkeletalData.Cast<FLiveLinkSkeletonStaticData>();

		// Frame
		t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame = new FLiveLinkFrameDataStruct(); // todo cleanup too.
		t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame->InitializeWith(FLiveLinkAnimationFrameData::StaticStruct(), nullptr);

		FLiveLinkAnimationFrameData* t_LiveLinkFramePtr = t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame->Cast<FLiveLinkAnimationFrameData>();

		const TArray<FMeshBoneInfo>& t_BoneInfo = ManusSkeleton->GetRawRefBoneInfo();
		SkeletonDataPtr->BoneNames.Reset();
		SkeletonDataPtr->BoneParents.Reset();
		t_LiveLinkFramePtr->Transforms.Reset();

		for (int i = 0; i < t_BoneInfo.Num(); i++)
		{
			SkeletonDataPtr->BoneNames.Add(FName(t_BoneInfo[i].Name)); // the actual string content is not used, but the strings must be allocated or unreal flips.
			SkeletonDataPtr->BoneParents.Add(i);

			t_LiveLinkFramePtr->Transforms.Add(FTransform::Identity);
		}
		m_LiveLinkClient->PushSubjectStaticData_AnyThread(p_LiveLinkSubjectKey, ULiveLinkAnimationRole::StaticClass(), MoveTemp(SkeletalData));
	}
}

bool FManusLiveLinkSource::UpdateSkeletonsLiveLinkTransforms(int p_ManusLiveLinkUserIndex, TArray<FTransform>& p_OutTransforms, uint64& p_LastUpdateTime)
{
	SCOPE_CYCLE_COUNTER(STAT_Manus_UpdateSkeletonsLiveLink);
    if (p_OutTransforms.Num() == 0) return false;
    //UE_LOG(LogManus, Warning, TEXT("UpdateSkeletonsLiveLinkTransforms."));

	p_LastUpdateTime = 0;

	UManusSkeleton* t_ManusSkeleton = nullptr;
	uint32_t SkeletonId = 0;
	EBoneAxis SkeletonStretchAxis = EBoneAxis::BA_X;
	const TArray<FManusLiveLinkUser>& t_ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	if (t_ManusLiveLinkUsers.IsValidIndex(p_ManusLiveLinkUserIndex))
	{
		t_ManusSkeleton = t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].ManusSkeleton;
				
		SkeletonId = t_ManusSkeleton->ManusSkeletonId;
	}

	if (SkeletonId > 0 && t_ManusSkeleton && t_ManusSkeleton->SkeletalMesh)
	{
        const TArray<FMeshBoneInfo>& t_BoneInfo = t_ManusSkeleton->GetRawRefBoneInfo();

		FManusMetaSkeleton t_SkeletonData;
		if (UManusBlueprintLibrary::GetSkeletonData(SkeletonId, t_SkeletonData) == EManusRet::Success) 
		{
			p_LastUpdateTime = (uint64)t_SkeletonData.LastUpdateTime; // this is where we will get a int64 to uin64 issue. but unreal blueprints are bad.
		
			TArray<FVector> LocalScales;

			for (int BoneIndex = 0; BoneIndex < (int)t_SkeletonData.Bones.Num(); BoneIndex++)
			{
                for (size_t i = 0; i < t_BoneInfo.Num(); i++)
				{
					if (t_ManusSkeleton->NodesSetupMap[t_BoneInfo[i].Name].Id == t_SkeletonData.Bones[BoneIndex].BoneId) // there are moments where these lists are not identical. so we got to check it. but most of the time they do match....(so far metahumans surprised us)
                    {
						LocalScales.Add(FVector::OneVector);
                        
						p_OutTransforms[i] = t_SkeletonData.Bones[BoneIndex].Transform;

						FQuat Orientation = p_OutTransforms[i].GetRotation();
						Orientation.Normalize();
                        
						p_OutTransforms[i].SetRotation(Orientation);
                        
                        //t_Keys.RemoveAt(i,1,false); // makes the list smaller to look through, but do not resize yet, as thats unnecessary mem reallocation
						// the above commented line can actually be slower, due to the move actions. so.... skipping for now.

						break;
					}
				}
			}

			return true;
		}
		else
		{
            //UE_LOG(LogManus, Warning, TEXT("UpdateSkeletonsLiveLinkTransforms we dont got skeleton data. %u"), SkeletonId);
            // Zero the scale to let the anim node know that there was no valid data
			for (int BoneIndex = 0; BoneIndex < (int)t_BoneInfo.Num(); BoneIndex++)
			{
				p_OutTransforms[BoneIndex].SetScale3D(FVector::ZeroVector);
			}
			return false;
		}
	}
	return false;
}

void FManusLiveLinkSource::InitSkeletonsForManusLiveLinkUser(int p_ManusLiveLinkUserIndex, uint32_t p_ManusUserId, bool ClearTempValues)
{
    UE_LOG(LogManus, Warning, TEXT("InitSkeletonsForManusLiveLinkUser."));

	TArray<FManusLiveLinkUser>& t_ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	if (t_ManusLiveLinkUsers.IsValidIndex(p_ManusLiveLinkUserIndex))
	{
		UManusSkeleton* t_ManusSkeleton = t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].ManusSkeleton;

		if (t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame == NULL)
		{
			t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame = new FLiveLinkFrameDataStruct();
            t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame->InitializeWith(FLiveLinkAnimationFrameData::StaticStruct(), nullptr);
		}

		if (t_ManusSkeleton && 
			t_ManusSkeleton->GetSkeleton())// do we even have a skeleton?
		{
            m_bIsSkeletonsInitialized = true;
            t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].SkeletonsInitializationRetryCountdown = 0; // ok we're trying to init it once. 

			// first check if its a hand or a mannequin , anything else we don't support . 
			if (t_ManusSkeleton->SkeletalMesh )
			{
				if (t_ManusSkeleton->SkeletonType == EManusSkeletonType::Invalid)
				{ 
					// cleanup
					delete t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame;
					t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame = NULL;
					UE_LOG(LogManus, Log, TEXT("skeleton %s did not have a skeleton type defined."), *t_ManusSkeleton->GetSkeleton()->GetName());
					return; // not a hand or a body. so do not process it.											
				}
			}

			UE_LOG(LogManus, Log, TEXT("Initializing Manus Skeletons for skeleton %s."), *t_ManusSkeleton->GetSkeleton()->GetName());

			// todo this is still a placeholder and we need to make it more robust. but for most cases this is fine.
			if (t_ManusSkeleton->TargetType == EManusSkeletonTargetType::UserData && t_ManusSkeleton->TargetUserData.userID == 0)
			{
				t_ManusSkeleton->TargetUserData.userID = p_ManusUserId; 
			}

			uint32_t  t_SkeletonSetupIndex = 0;
			
			if (t_ManusSkeleton->SetupSkeleton(t_SkeletonSetupIndex) != EManusRet::Success)
			{
				UE_LOG(LogManus, Error, TEXT("Failed to Create Skeleton Setup. %s."), *t_ManusSkeleton->GetSkeleton()->GetName());
				// cleanup
				delete t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame;
				t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame = NULL;
				return;
			}

			if (t_ManusSkeleton->NodesSetupMap.Num() > 0) // check if we already have nodes.
			{
				if (t_ManusSkeleton->LoadExistingNodes(t_SkeletonSetupIndex) != true)
				{
					UE_LOG(LogManus, Error, TEXT("Failed to Add Node To Skeleton Setup. %s."), *t_ManusSkeleton->GetSkeleton()->GetName());
					// cleanup
					delete t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame;
					t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame = NULL;
					t_ManusSkeleton->ClearSetupSkeleton(t_SkeletonSetupIndex);
					return;
				}
			}
			else
			{
				if (t_ManusSkeleton->LoadNewNodes(t_SkeletonSetupIndex) != true)
				{
					UE_LOG(LogManus, Error, TEXT("Failed to Add Node To Skeleton Setup. %s."), *t_ManusSkeleton->GetSkeleton()->GetName());
					// cleanup
					delete t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame;
					t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame = NULL;
					t_ManusSkeleton->ClearSetupSkeleton(t_SkeletonSetupIndex);
					return;
				}
			}

			if (t_ManusSkeleton->NodesSetupMap.Num() == 0)
			{
				UE_LOG(LogManus, Error, TEXT("Failed to Create Skeleton Setup. %s has no nodes."), *t_ManusSkeleton->GetSkeleton()->GetName());
				// cleanup
				delete t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame;
				t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame = NULL;
				t_ManusSkeleton->ClearSetupSkeleton(t_SkeletonSetupIndex);
				return; // if the asset is malformed or such and we don't have nodes. welp its game over for this asset.
			}

			if (t_ManusSkeleton->ChainsIndexMap.Num() != 0) 
			{ 
				// load the existing chains up
				t_ManusSkeleton->LoadChains(t_SkeletonSetupIndex, FManusModule::Get().GetGlovesUsingTrackers());
			}
			else // we no longer autoallocate. our assets come pre loaded with chains now. so if there is nothing. it is bad.
			{
				UE_LOG(LogManus, Error, TEXT("Failed to Create Skeleton Setup. %s has no chains."), *t_ManusSkeleton->GetSkeleton()->GetName());
				// cleanup
				delete t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame;
				t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame = NULL;
				t_ManusSkeleton->ClearSetupSkeleton(t_SkeletonSetupIndex);
				return; // if the asset is malformed or such and we don't have chains. welp its game over for this asset.
			}
						
			// and load the skeleton.
			uint32_t t_ID = 0;
			if (t_ManusSkeleton->LoadSkeleton(t_SkeletonSetupIndex, t_ID) != true || t_ID == 0)
			{
				UE_LOG(LogManus, Error, TEXT("Failed to load Skeleton Setup. %s."), *t_ManusSkeleton->GetSkeleton()->GetName());
				delete t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame;
				t_ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].LiveLinkFrame = NULL;
				t_ManusSkeleton->ClearSetupSkeleton(t_SkeletonSetupIndex);
				return;
			}
            UE_LOG(LogManus, Warning, TEXT("successfully initialized skeleton. %s."), *t_ManusSkeleton->GetSkeleton()->GetName());
			t_ManusSkeleton->ManusSkeletonId = t_ID;
		}
        else
        {
            UE_LOG(LogManus, Warning, TEXT("InitSkeletonsForManusLiveLinkUser failed. No mesh found."));
        }
	}
    else
    {
        UE_LOG(LogManus, Warning, TEXT("InitSkeletonsForManusLiveLinkUser failed. No valid LiveLinkUser found."));
    }
}

#undef LOCTEXT_NAMESPACE
