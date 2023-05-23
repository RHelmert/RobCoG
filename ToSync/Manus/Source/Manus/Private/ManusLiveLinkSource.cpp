// Copyright 2015-2020 Manus

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
#if WITH_EDITOR
#include "ManusEditorUserSettings.h"
#endif // WITH_EDITOR

#include "LiveLinkClient.h"
#include "LiveLinkSourceFactory.h"
#include "ILiveLinkClient.h"
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#else
#include "LiveLinkTypes.h"
#endif
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Features/IModularFeatures.h"

DECLARE_CYCLE_STAT(TEXT("Manus Update Glove Assignments"), STAT_Manus_UpdateGloveAssignments, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("Manus Update Live Link"), STAT_Manus_UpdateLiveLink, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("Manus Update Polygon Live Link"), STAT_Manus_UpdatePolygonLiveLink, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("Manus Update Glove Live Link"), STAT_Manus_UpdateGloveLiveLink, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("Manus Gesture Detection"), STAT_Manus_GestureDetection, STATGROUP_Manus);

#define LOCTEXT_NAMESPACE "FManusModule"

FManusLiveLinkSource::FManusLiveLinkSource(EManusLiveLinkSourceType InSourceType)
: SourceType(InSourceType)
{
}

void FManusLiveLinkSource::Init()
{
	// Init Manus replicated Live Link source
	FLiveLinkClient* Client = &IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(FLiveLinkClient::ModularFeatureName);
	if (Client)
	{
		Client->AddSource(FManusModule::Get().GetLiveLinkSource(SourceType));
	}

	// Init Polygon
	if (SourceType == EManusLiveLinkSourceType::Local)
	{
		InitPolygonForAllManusLiveLinkUsers(true);
	}
}

void FManusLiveLinkSource::Destroy()
{
	if (LiveLinkClient)
	{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 24
		LiveLinkClient->RemoveSource(LiveLinkSourceGuid);
#else
		LiveLinkClient->RemoveSource(FManusModule::Get().GetLiveLinkSource(SourceType));
#endif
	}
}

void FManusLiveLinkSource::Tick(float DeltaTime)
{
	if (SourceType == EManusLiveLinkSourceType::Local)
	{
		// Take care of restarting Core connection when needed
		EManusRet CoreSdkStatus = CoreSdk::CheckConnection();
		if (CoreSdkStatus != EManusRet::Success)
		{
			// Only log the time out one time
			if (!bIsConnectionWithCoreTimingOut)
			{
				UE_LOG(LogManus, Warning, TEXT("Connection with Manus Core timed out. Please make sure that Manus Core is up and running."));
			}

			// Connection with Core is timing out
			bIsConnectionWithCoreTimingOut = true;
		}
		else if (bIsConnectionWithCoreTimingOut)
		{
			// Connection with Core is back
			bIsConnectionWithCoreTimingOut = false;
			UE_LOG(LogManus, Log, TEXT("Connection with Manus Core restored."));

			// Reinitialize Polygon
			InitPolygonForAllManusLiveLinkUsers(true);
		}

#if WITH_EDITOR
		// Tick skeleton previews
		TickAllHandPreviews(DeltaTime);
#endif // WITH_EDITOR

		// Update Manus Live Link Users
		TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;

		// Update the Manus Dashboard Users Glove assignments
		UpdateManusDashboardUsersGloveAssignments(DeltaTime);

		// Reset replicated data
		ReplicatedFrameDataArray.Reset();

		// Update Live Link for each Manus Live Link User
		for (int i = 0; i < ManusLiveLinkUsers.Num(); i++)
		{
			UpdateLiveLink(DeltaTime, i);
		}
		bNewLiveLinkClient = false;

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
		// Remove potential ghost Live Link subjects left there after removing Manus Live Link Users
		int ManusLiveLinkUserIndexToRemove = ManusLiveLinkUsers.Num();
		while (LiveLinkClient && LiveLinkClient->IsSubjectEnabled(FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndexToRemove)))
		{
			FLiveLinkSubjectName LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndexToRemove);
			FLiveLinkSubjectKey LiveLinkSubjectKey = FLiveLinkSubjectKey(LiveLinkSourceGuid, LiveLinkSubjectName);
			LiveLinkClient->RemoveSubject_AnyThread(LiveLinkSubjectKey);
			ManusLiveLinkUserIndexToRemove++;
		}
#else
		int ManusLiveLinkUserIndexToRemove = ManusLiveLinkUsers.Num();
		while (LiveLinkClient && IsSubjectEnabled(FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndexToRemove)))
		{
			FName LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndexToRemove);
			LiveLinkClient->ClearSubject(LiveLinkSubjectName);
			ManusLiveLinkUserIndexToRemove++;
		}
#endif

		// Make sure Polygon is initialized properly for each user
		if (CoreSdkStatus == EManusRet::Success)
		{
			for (int i = 0; i < ManusLiveLinkUsers.Num(); i++)
			{
				if (ManusLiveLinkUsers[i].PolygonInitializationRetryCountdown > 0)
				{
					ManusLiveLinkUsers[i].PolygonInitializationRetryCountdown--;
					if (ManusLiveLinkUsers[i].PolygonInitializationRetryCountdown <= 0)
					{
						InitPolygonForManusLiveLinkUser(i, false);
					}
				}
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
	LiveLinkClient = InClient;
	LiveLinkSourceGuid = InSourceGuid;
	bNewLiveLinkClient = true;

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
	if (ULiveLinkSourceSettings* Settings = static_cast<FLiveLinkClient*>(InClient)->GetSourceSettings(LiveLinkSourceGuid))
	{
		Settings->ConnectionString = FString();
	}

	if (SourceType == EManusLiveLinkSourceType::Local)
	{
		SetBufferOffset(GetDefault<UManusSettings>()->TrackingSmoothing);
	}
	else if (SourceType == EManusLiveLinkSourceType::Replicated)
	{
		SetBufferOffset(GetDefault<UManusSettings>()->DefaultReplicationOffsetTime);
	}
#endif
}

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
bool FManusLiveLinkSource::IsSourceStillValid() const
#else
bool FManusLiveLinkSource::IsSourceStillValid()
#endif
{
	return LiveLinkClient != nullptr;
}

bool FManusLiveLinkSource::RequestSourceShutdown()
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 23
	FManusModule::Get().OnLiveLinkSourceRemoved(LiveLinkSourceGuid);
#endif
	LiveLinkClient = nullptr;
	LiveLinkSourceGuid.Invalidate();
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
	FText SourceTypeName;
	switch (SourceType)
	{
	case EManusLiveLinkSourceType::Local:		SourceTypeName = FText::FromString("Manus");				break;
	case EManusLiveLinkSourceType::Replicated:	SourceTypeName = FText::FromString("Manus Replicated");		break;
	}
	return SourceTypeName;
}

void FManusLiveLinkSource::SetBufferOffset(float Offset)
{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
	FLiveLinkClient* Client = &IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(FLiveLinkClient::ModularFeatureName);
	if (Client)
	{
		if (ULiveLinkSourceSettings* Settings = Client->GetSourceSettings(LiveLinkSourceGuid))
		{
			Settings->BufferSettings.EngineTimeOffset = Offset;
			Settings->BufferSettings.MaxNumberOfFrameToBuffered = FMath::Max(10, (int)(Offset * 90));
		}
	}
#endif
}

FName FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(const UManusComponent* ManusComponent)
{
	if (ManusComponent)
	{
		int ManusLiveLinkUserIndex = FManusModule::Get().GetManusLiveLinkUserIndex(ManusComponent->ManusDashboardUserIndex, ManusComponent->ManusSkeleton);
		if (ManusLiveLinkUserIndex != INDEX_NONE)
		{
			if (ManusComponent->IsLocallyOwned())
			{
				return GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndex);
			}
			else
			{
				return GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndex, ManusComponent->ManusReplicatorId);
			}
		}
	}
	return GetManusLiveLinkUserLiveLinkSubjectName(INDEX_NONE);
}

FName FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(int ManusLiveLinkUserIndex, int32 ReplicatorId)
{
	if (ManusLiveLinkUserIndex < 0 || ManusLiveLinkUserIndex > 99)
	{
		UE_LOG(LogManus, Warning, TEXT("The Manus Live Link User index is incorrect. The Manus Live Link User Live Link subject name will be set to None."));
		return FName(TEXT("None"));
	}

	bool IsReplicated = (ReplicatorId != 0);

	FString Name = IsReplicated ? "ManusReplicatedUser_" : "ManusLiveLinkUser_";
	if (IsReplicated)
	{
		Name += FString::FromInt(ReplicatorId);
		Name += "_";
	}
	if (ManusLiveLinkUserIndex < 10)
	{
		Name += "0";
	}
	Name += FString::FromInt(ManusLiveLinkUserIndex);

	return FName(*Name);
}

void FManusLiveLinkSource::ReplicateLiveLink(class AManusReplicator* Replicator)
{
	if (!Replicator->IsPendingKill())
	{
		// For each Manus Live Link User
		for (int i = 0; i < Replicator->ReplicatedData.ReplicatedFrameDataArray.Num(); i++)
		{
			int ManusLiveLinkUserIndex = FManusModule::Get().GetManusLiveLinkUserIndex(Replicator->ReplicatedData.ReplicatedFrameDataArray[i].ManusDashboardUserIndex, Replicator->ReplicatedData.ReplicatedFrameDataArray[i].ManusSkeleton);

			// Create or recreate subject (if needed)
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
			FLiveLinkSubjectName LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndex, Replicator->ReplicatorId);
			FLiveLinkSubjectKey LiveLinkSubjectKey = FLiveLinkSubjectKey(LiveLinkSourceGuid, LiveLinkSubjectName);
			if (!LiveLinkClient->IsSubjectEnabled(LiveLinkSubjectName))
			{
				RecreateLiveLinkSubject(LiveLinkSubjectKey);
			}

			// Update Frame
			FLiveLinkAnimationFrameData* LiveLinkFramePtr = LiveLinkFrame.Cast<FLiveLinkAnimationFrameData>();
			LiveLinkFramePtr->WorldTime = FPlatformTime::Seconds();
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
			LiveLinkFramePtr->MetaData.SceneTime = FApp::GetCurrentFrameTime().Get(FQualifiedFrameTime());
#else
			LiveLinkFramePtr->MetaData.SceneTime = FQualifiedFrameTime(FApp::GetTimecode(), FApp::GetTimecodeFrameRate());
#endif

			// Update the transforms from the replicated data
			AManusReplicator::DecompressReplicatedFrameData(*LiveLinkFramePtr, Replicator->ReplicatedData.ReplicatedFrameDataArray[i]);

			// Copy the data locally and share it with the LiveLink client
			FLiveLinkFrameDataStruct NewLiveLinkFrame;
			NewLiveLinkFrame.InitializeWith(LiveLinkFrame);
			LiveLinkClient->PushSubjectFrameData_AnyThread(LiveLinkSubjectKey, MoveTemp(NewLiveLinkFrame));

#else

			FName LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndex, Replicator->ReplicatorId);
			FLiveLinkSubjectKey LiveLinkSubjectKey = FLiveLinkSubjectKey(LiveLinkSubjectName, LiveLinkSourceGuid);
			if (!IsSubjectEnabled(LiveLinkSubjectName))
			{
				RecreateLiveLinkSubject(LiveLinkSubjectKey);
			}

			// Update Frame
			LiveLinkFrame.WorldTime = FPlatformTime::Seconds();
			LiveLinkFrame.MetaData.SceneTime = FQualifiedFrameTime(FApp::GetTimecode(), FApp::GetTimecodeFrameRate());

			// Update the transforms from the replicated data
			AManusReplicator::DecompressReplicatedFrameData(LiveLinkFrame, Replicator->ReplicatedData.ReplicatedFrameDataArray[i]);

			// Copy the data locally and share it with the LiveLink client
			FLiveLinkFrameData NewLiveLinkFrame;
			NewLiveLinkFrame.CurveElements = LiveLinkFrame.CurveElements;
			NewLiveLinkFrame.MetaData = LiveLinkFrame.MetaData;
			NewLiveLinkFrame.Transforms = LiveLinkFrame.Transforms;
			NewLiveLinkFrame.WorldTime = LiveLinkFrame.WorldTime;
			LiveLinkClient->PushSubjectData(LiveLinkSourceGuid, LiveLinkSubjectName, NewLiveLinkFrame);
#endif
		}

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
		// Update replicated data buffer offset according to current ping
		const UManusSettings* ManusSettings = GetDefault<UManusSettings>();
		if (ManusSettings && ManusSettings->bUpdateReplicationOffsetTimeUsingPing)
		{
			FLiveLinkClient* Client = &IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(FLiveLinkClient::ModularFeatureName);
			if (Client)
			{
				if (ULiveLinkSourceSettings* Settings = Client->GetSourceSettings(LiveLinkSourceGuid))
				{
					if (APlayerController* PlayerController = Replicator->GetWorld()->GetFirstPlayerController())
					{
						if (APlayerState* PlayerState = PlayerController->PlayerState)
						{
							float DesiredOffset = PlayerState->ExactPing * 0.001f * ManusSettings->ReplicationOffsetTimePingMultiplier + ManusSettings->ReplicationOffsetTimePingExtraTime;
							SetBufferOffset(Settings->BufferSettings.EngineTimeOffset * 0.995f + DesiredOffset * 0.005f);
						}
					}
				}
			}
		}
#endif
	}
}

void FManusLiveLinkSource::StopReplicatingLiveLink(class AManusReplicator* Replicator)
{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
	int ManusLiveLinkUserIndexToRemove = 0;
	while (LiveLinkClient && LiveLinkClient->IsSubjectEnabled(FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndexToRemove, Replicator->ReplicatorId)))
	{
		FLiveLinkSubjectName LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndexToRemove, Replicator->ReplicatorId);
		FLiveLinkSubjectKey LiveLinkSubjectKey = FLiveLinkSubjectKey(LiveLinkSourceGuid, LiveLinkSubjectName);
		LiveLinkClient->RemoveSubject_AnyThread(LiveLinkSubjectKey);
		ManusLiveLinkUserIndexToRemove++;
	}
#else
	int ManusLiveLinkUserIndexToRemove = 0;
	while (LiveLinkClient && IsSubjectEnabled(FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndexToRemove, Replicator->ReplicatorId)))
	{
		FName LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndexToRemove);
		LiveLinkClient->ClearSubject(LiveLinkSubjectName);
		ManusLiveLinkUserIndexToRemove++;
	}
#endif
}

void FManusLiveLinkSource::UpdateManusDashboardUsersGloveAssignments(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_Manus_UpdateGloveAssignments);

	if (ManusDashboardUserGloveAssignmentUpdateQueue.Num() == 0)
	{
		// No queued update - Update timer
		ManusDashboardUserGloveAssignmentUpdateTimer = FMath::Max(0.0f, ManusDashboardUserGloveAssignmentUpdateTimer - DeltaTime);
		
		// When it's time to update
		if (ManusDashboardUserGloveAssignmentUpdateTimer <= 0.0f)
		{
			// Update the cache with the indices of the Manus Dashboard Users we are currently using
			const TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;

			// Remove Manus Dashboard User indices we don't use anymore
			for (auto Itr = ManusDashboardUserGloveAssignmentCache.CreateIterator(); Itr; ++Itr)
			{
				bool bIsStillInUse = false;
				for (int i = 0; i < ManusLiveLinkUsers.Num() && !bIsStillInUse; i++)
				{
					if (Itr->Key == ManusLiveLinkUsers[i].ManusDashboardUserIndex)
					{
						bIsStillInUse = true;
					}
				}
				if (!bIsStillInUse)
				{
					Itr.RemoveCurrent();
				}
			}

			// Add new Manus Dashboard User indices
			for (int i = 0; i < ManusLiveLinkUsers.Num(); i++)
			{
				ManusDashboardUserGloveAssignmentCache.FindOrAdd(ManusLiveLinkUsers[i].ManusDashboardUserIndex);
			}

			// Queue the Manus Dashboard User indices to update
			ManusDashboardUserGloveAssignmentUpdateQueue.Reset();
			for (auto Itr = ManusDashboardUserGloveAssignmentCache.CreateIterator(); Itr; ++Itr)
			{
				ManusDashboardUserGloveAssignmentUpdateQueue.Add(Itr->Key); // Left Glove
				ManusDashboardUserGloveAssignmentUpdateQueue.Add(Itr->Key + 0.5f); // Right Glove
			}
		}
	}
	else
	{
		// Do only one update at a time
		float ManusDashboardUserAssignmentToUpdate = ManusDashboardUserGloveAssignmentUpdateQueue.Pop(false);
		int ManusDashboardUserIndex = (int)ManusDashboardUserAssignmentToUpdate;
		EManusHandType HandType = (ManusDashboardUserAssignmentToUpdate - ManusDashboardUserIndex > 0.0f) ? EManusHandType::Left : EManusHandType::Right;
		FManusGloveAssignment* AssignmentPtr = ManusDashboardUserGloveAssignmentCache.Find(ManusDashboardUserIndex);
		if (AssignmentPtr)
		{
			CoreSdk::GetGloveIdOfUser_UsingUserIndex(ManusDashboardUserIndex, HandType, AssignmentPtr->GloveIds[(int)HandType]);
		}
	}

	// Reset update timer
	if (ManusDashboardUserGloveAssignmentUpdateQueue.Num() == 0 && ManusDashboardUserGloveAssignmentUpdateTimer <= 0.0f)
	{
		ManusDashboardUserGloveAssignmentUpdateTimer = GetDefault<UManusSettings>()->ManusDashboardUserGloveAssignmentUpdateFrequency;
	}
}

int64 FManusLiveLinkSource::GetCachedGloveAssignment(int ManusDashboardUserIndex, EManusHandType HandType)
{
	FManusGloveAssignment* AssignmentPtr = ManusDashboardUserGloveAssignmentCache.Find(ManusDashboardUserIndex);
	if (AssignmentPtr)
	{
		return AssignmentPtr->GloveIds[(int)HandType];
	}
	return 0;
}

void FManusLiveLinkSource::UpdateLiveLink(float DeltaTime, int ManusLiveLinkUserIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_Manus_UpdateLiveLink);

	check(IsInGameThread());

	TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	ManusLiveLinkUsers[ManusLiveLinkUserIndex].bShouldUpdateLiveLinkData = false;

#if WITH_EDITORONLY_DATA
	// Update live link for this Live Link user during preview
	ManusLiveLinkUsers[ManusLiveLinkUserIndex].bShouldUpdateLiveLinkData |= ManusLiveLinkUsers[ManusLiveLinkUserIndex].bIsHandPreviewOnGoing;
#endif // WITH_EDITORONLY_DATA

	// Update live link for this Live Link user if some objects are using it
	if (!ManusLiveLinkUsers[ManusLiveLinkUserIndex].bShouldUpdateLiveLinkData)
	{
		ManusLiveLinkUsers[ManusLiveLinkUserIndex].bShouldUpdateLiveLinkData |= FManusModule::Get().IsAnyObjectUsingManusLiveLinkUser(ManusLiveLinkUserIndex);
	}

	// Update Live Link only when necessary
	if (LiveLinkClient && ManusLiveLinkUsers[ManusLiveLinkUserIndex].bShouldUpdateLiveLinkData)
	{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
		FLiveLinkSubjectName LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndex);
		FLiveLinkSubjectKey LiveLinkSubjectKey = FLiveLinkSubjectKey(LiveLinkSourceGuid, LiveLinkSubjectName);

		// Reinit subject when the Live Link client changes
		if (bNewLiveLinkClient || !LiveLinkClient->IsSubjectEnabled(LiveLinkSubjectName))
		{
			RecreateLiveLinkSubject(LiveLinkSubjectKey);
		}
#else
		FName LiveLinkSubjectName = FManusLiveLinkSource::GetManusLiveLinkUserLiveLinkSubjectName(ManusLiveLinkUserIndex);
		FLiveLinkSubjectKey LiveLinkSubjectKey = FLiveLinkSubjectKey(LiveLinkSubjectName, LiveLinkSourceGuid);

		// Reinit subject when the Live Link client changes
		if (bNewLiveLinkClient || !IsSubjectEnabled(LiveLinkSubjectName))
		{
			RecreateLiveLinkSubject(LiveLinkSubjectKey);
		}
#endif

		// Update Frame
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
		FLiveLinkAnimationFrameData* LiveLinkFramePtr = LiveLinkFrame.Cast<FLiveLinkAnimationFrameData>();
#endif

		// Update the transforms for each subject from tracking data
		int64 LastUpdateTime = 0, LastUpdateTimeAll = 0;
		bool ForceUpdate = false, ForceUpdateAll = false;

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
		UpdateGloveLiveLinkTransforms(DeltaTime, ManusLiveLinkUserIndex, EManusHandType::Left, LiveLinkFramePtr->Transforms, LastUpdateTime, ForceUpdate);
#else
		UpdateGloveLiveLinkTransforms(DeltaTime, ManusLiveLinkUserIndex, EManusHandType::Left, LiveLinkFrame.Transforms, LastUpdateTime, ForceUpdate);
#endif
		LastUpdateTimeAll = FMath::Max(LastUpdateTimeAll, LastUpdateTime);
		ForceUpdateAll |= ForceUpdate;

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
		UpdateGloveLiveLinkTransforms(DeltaTime, ManusLiveLinkUserIndex, EManusHandType::Right, LiveLinkFramePtr->Transforms, LastUpdateTime, ForceUpdate);
#else
		UpdateGloveLiveLinkTransforms(DeltaTime, ManusLiveLinkUserIndex, EManusHandType::Right, LiveLinkFrame.Transforms, LastUpdateTime, ForceUpdate);
#endif
		LastUpdateTimeAll = FMath::Max(LastUpdateTimeAll, LastUpdateTime);
		ForceUpdateAll |= ForceUpdate;

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
		UpdatePolygonLiveLinkTransforms(ManusLiveLinkUserIndex, LiveLinkFramePtr->Transforms, LastUpdateTime);
#else
		UpdatePolygonLiveLinkTransforms(ManusLiveLinkUserIndex, LiveLinkFrame.Transforms, LastUpdateTime);
#endif
		LastUpdateTimeAll = FMath::Max(LastUpdateTimeAll, LastUpdateTime);

		if (LastUpdateTimeAll > ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusDataLastUpdateTime || ForceUpdateAll)
		{
			ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusDataLastUpdateTime = LastUpdateTimeAll;

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
			// Frame time
			LiveLinkFramePtr->WorldTime = FPlatformTime::Seconds();
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
			LiveLinkFramePtr->MetaData.SceneTime = FApp::GetCurrentFrameTime().Get(FQualifiedFrameTime());
#else
			LiveLinkFramePtr->MetaData.SceneTime = FQualifiedFrameTime(FApp::GetTimecode(), FApp::GetTimecodeFrameRate());
#endif

			// Copy the data locally and share it with the LiveLink client
			FLiveLinkFrameDataStruct NewLiveLinkFrame;
			NewLiveLinkFrame.InitializeWith(LiveLinkFrame);
			LiveLinkClient->PushSubjectFrameData_AnyThread(LiveLinkSubjectKey, MoveTemp(NewLiveLinkFrame));

			// Copy the transforms locally to replicate them (if necessary)
			if (FManusModule::Get().IsAnyReplicatingObjectUsingManusLiveLinkUser(ManusLiveLinkUserIndex))
			{
				int ReplicatedFrameDataArrayIndex = ReplicatedFrameDataArray.AddDefaulted();
				ReplicatedFrameDataArray[ReplicatedFrameDataArrayIndex].ManusDashboardUserIndex = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusDashboardUserIndex;
				ReplicatedFrameDataArray[ReplicatedFrameDataArrayIndex].ManusSkeleton = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusSkeleton;


				AManusReplicator::CompressReplicatedFrameData(*LiveLinkFramePtr, ReplicatedFrameDataArray[ReplicatedFrameDataArrayIndex]);
			}
#else
			// Frame time
			LiveLinkFrame.WorldTime = FPlatformTime::Seconds();
			LiveLinkFrame.MetaData.SceneTime = FQualifiedFrameTime(FApp::GetTimecode(), FApp::GetTimecodeFrameRate());

			// Copy the data locally and share it with the LiveLink client
			FLiveLinkFrameData NewLiveLinkFrame;
			NewLiveLinkFrame.CurveElements = LiveLinkFrame.CurveElements;
			NewLiveLinkFrame.MetaData = LiveLinkFrame.MetaData;
			NewLiveLinkFrame.Transforms = LiveLinkFrame.Transforms;
			NewLiveLinkFrame.WorldTime = LiveLinkFrame.WorldTime;
			LiveLinkClient->PushSubjectData(LiveLinkSourceGuid, LiveLinkSubjectName, NewLiveLinkFrame);

			// Copy the transforms locally to replicate them (if necessary)
			if (FManusModule::Get().IsAnyReplicatingObjectUsingManusLiveLinkUser(ManusLiveLinkUserIndex))
			{
				int ReplicatedFrameDataArrayIndex = ReplicatedFrameDataArray.AddDefaulted();
				ReplicatedFrameDataArray[ReplicatedFrameDataArrayIndex].ManusDashboardUserIndex = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusDashboardUserIndex;
				ReplicatedFrameDataArray[ReplicatedFrameDataArrayIndex].ManusSkeleton = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusSkeleton;
				AManusReplicator::CompressReplicatedFrameData(LiveLinkFrame, ReplicatedFrameDataArray[ReplicatedFrameDataArrayIndex]);
			}
#endif
		}
	}
}

void FManusLiveLinkSource::RecreateLiveLinkSubject(FLiveLinkSubjectKey& LiveLinkSubjectKey)
{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
	LiveLinkClient->RemoveSubject_AnyThread(LiveLinkSubjectKey);

	// Skeleton data
	FLiveLinkStaticDataStruct SkeletalData(FLiveLinkSkeletonStaticData::StaticStruct());
	FLiveLinkSkeletonStaticData* SkeletonDataPtr = SkeletalData.Cast<FLiveLinkSkeletonStaticData>();
	SetupLiveLinkData(*SkeletonDataPtr);

	// Frame
	LiveLinkFrame.InitializeWith(FLiveLinkAnimationFrameData::StaticStruct(), nullptr);
	FLiveLinkAnimationFrameData* LiveLinkFramePtr = LiveLinkFrame.Cast<FLiveLinkAnimationFrameData>();
	for (int i = 0; i < SkeletonDataPtr->BoneNames.Num(); ++i)
	{
		LiveLinkFramePtr->Transforms.Add(FTransform::Identity);
	}

	LiveLinkClient->PushSubjectStaticData_AnyThread(LiveLinkSubjectKey, ULiveLinkAnimationRole::StaticClass(), MoveTemp(SkeletalData));
#else
	LiveLinkClient->ClearSubject(LiveLinkSubjectKey.SubjectName);
	LiveLinkClient->ClearSubjectsFrames(LiveLinkSubjectKey.SubjectName);

	// Skeleton data
	FLiveLinkRefSkeleton SkeletonData;
	SetupLiveLinkData(SkeletonData);

	// Frame
	LiveLinkFrame.Transforms.Reset();
	for (int i = 0; i < SkeletonData.GetBoneNames().Num(); ++i)
	{
		LiveLinkFrame.Transforms.Add(FTransform::Identity);
	}

	LiveLinkClient->PushSubjectSkeleton(LiveLinkSourceGuid, LiveLinkSubjectKey.SubjectName, SkeletonData);
#endif
}

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
#define MANUSLIVELINKBONEDEFINER_ENUM(bone, parent) StaticData.BoneNames.Add(FName(*ManusBoneNameEnumPtr->GetNameStringByIndex((int)bone))); StaticData.BoneParents.Add(parent); SkeletonBoneParents.Add(parent);
void FManusLiveLinkSource::SetupLiveLinkData(FLiveLinkSkeletonStaticData& StaticData)
{
#else
#define MANUSLIVELINKBONEDEFINER_ENUM(bone, parent) BoneNames.Add(FName(*ManusBoneNameEnumPtr->GetNameStringByIndex((int)bone))); BoneParents.Add(parent); SkeletonBoneParents.Add(parent);
void FManusLiveLinkSource::SetupLiveLinkData(FLiveLinkRefSkeleton& StaticData)
{
	TArray<FName> BoneNames;
	TArray<int32> BoneParents;
#endif

	const UEnum* ManusBoneNameEnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EManusBoneName"), true);

	// Setup skeleton bone hierarchy (only one can be the root = -1 parent)
	// Define the bones in their order in EManusBoneName

	// Polygon data
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::Root, -1); // 0
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::Hips, 0); // 1

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::Spine, 1); // 2
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::Chest, 2); // 3
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::UpperChest, 3); // 4

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::Neck, 4); // 5
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::Head, 5); // 6

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftUpperLeg, 1); // 7
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftLowerLeg, 7); // 8
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftFoot, 8); // 9
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftToes, 9); // 10
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftToesEnd, 10); // 11

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightUpperLeg, 1); // 12
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightLowerLeg, 12); // 13
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightFoot, 13); // 14
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightToes, 14); // 15
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightToesEnd, 15); // 16

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftShoulder, 4); // 17
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftUpperArm, 17); // 18
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftLowerArm, 18); // 19
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHand, 19); // 20

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightShoulder, 4); // 21
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightUpperArm, 21); // 22
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightLowerArm, 22); // 23
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHand, 23); // 24

	// Left Manus Glove data
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandTracker, 19); // 25 - Attach the fingers to the "LeftHand" bone, not the "LeftHandTracker"
	
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandThumb1, 20); // 26
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandThumb2, 26); // 27
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandThumb3, 27); // 28
	
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandIndex1, 20); // 29
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandIndex2, 29); // 30
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandIndex3, 30); // 31

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandMiddle1, 20); // 32
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandMiddle2, 32); // 33
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandMiddle3, 33); // 34

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandRing1, 20); // 35
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandRing2, 35); // 36
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandRing3, 36); // 37

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandPinky1, 20); // 38
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandPinky2, 38); // 39
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::LeftHandPinky3, 39); // 40

	// Right Manus Glove data
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandTracker, 23); // 41 - Attach the fingers to the "RightHand" bone, not the "RightHandTracker"

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandThumb1, 24); // 42
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandThumb2, 42); // 43
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandThumb3, 43); // 44

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandIndex1, 24); // 45
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandIndex2, 45); // 46
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandIndex3, 46); // 47
	
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandMiddle1, 24); // 48
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandMiddle2, 48); // 49
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandMiddle3, 49); // 50

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandRing1, 24); // 51
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandRing2, 51); // 52
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandRing3, 52); // 53

	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandPinky1, 24); // 54
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandPinky2, 54); // 55
	MANUSLIVELINKBONEDEFINER_ENUM(EManusBoneName::RightHandPinky3, 55); // 56

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 23
	StaticData.SetBoneNames(BoneNames);
	StaticData.SetBoneParents(BoneParents);
#endif
}
#undef MANUSLIVELINKBONEDEFINER_ENUM

void FManusLiveLinkSource::UpdatePolygonLiveLinkTransforms(int ManusLiveLinkUserIndex, TArray<FTransform>& OutTransforms, int64& LastUpdateTime)
{
	SCOPE_CYCLE_COUNTER(STAT_Manus_UpdatePolygonLiveLink);

	LastUpdateTime = 0;

	UManusSkeleton* ManusSkeleton;
	int64 PolygonSkeletonId = 0;
	EBoneAxis PolygonSkeletonStretchAxis = EBoneAxis::BA_X;
	const TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	if (ManusLiveLinkUsers.IsValidIndex(ManusLiveLinkUserIndex))
	{
		ManusSkeleton = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusSkeleton;
		if (ManusSkeleton && ManusSkeleton->bIsUsedForFullBodyTracking)
		{
			PolygonSkeletonId = ManusTools::GenerateManusIdFromManusLiveLinkUser(ManusLiveLinkUserIndex);
			PolygonSkeletonStretchAxis = ManusSkeleton->FullBodyStretchAxis;
		}
	}
	
	if (PolygonSkeletonId > 0)
	{
		FManusPolygonSkeleton PolygonSkeletonData;
		if (UManusBlueprintLibrary::GetPolygonSkeletonData(PolygonSkeletonId, PolygonSkeletonData) == EManusRet::Success)
		{
			LastUpdateTime = PolygonSkeletonData.LastUpdateTime;

			TMap<int, FQuat> PolygonSkeletonManusInternalDeltaOrientations;
			if (ManusLiveLinkUsers.IsValidIndex(ManusLiveLinkUserIndex))
			{
				PolygonSkeletonManusInternalDeltaOrientations = ManusSkeleton->ManusInternalDeltaOrientations;
			}

			FVector RootScale = FVector::OneVector;
			TArray<FVector> LocalScales;

			for (int BoneIndex = 0; BoneIndex < PolygonSkeletonData.Bones.Num(); BoneIndex++)
			{
				LocalScales.Add(FVector::OneVector);

				if (PolygonSkeletonData.Bones[BoneIndex].Validity)
				{
					OutTransforms[BoneIndex] = PolygonSkeletonData.Bones[BoneIndex].Transform;

					// Rotation
					if (FQuat* ManusInternalDeltaOrientation = PolygonSkeletonManusInternalDeltaOrientations.Find(BoneIndex))
					{
						FQuat Orientation = OutTransforms[BoneIndex].GetRotation() * (ManusConvert::ConvertUnityToUnrealQuat(*ManusInternalDeltaOrientation)).Inverse();
						Orientation.Normalize();
						OutTransforms[BoneIndex].SetRotation(Orientation);
					}

					// Scale
					if (BoneIndex == (int)EManusBoneName::Root)
					{
						RootScale = PolygonSkeletonData.Bones[BoneIndex].Transform.GetScale3D();
						OutTransforms[BoneIndex].SetScale3D(RootScale);
						LocalScales[BoneIndex] = RootScale;
					}
					else
					{
						// Scale: Stretch scale
						float StretchScale = PolygonSkeletonData.Bones[BoneIndex].Transform.GetScale3D().X;
						if (BoneIndex == (int)EManusBoneName::Head
							|| BoneIndex == (int)EManusBoneName::LeftHand
							|| BoneIndex == (int)EManusBoneName::RightHand
							|| BoneIndex == (int)EManusBoneName::LeftFoot
							|| BoneIndex == (int)EManusBoneName::RightFoot)
						{
							StretchScale = 1.0f;
						}

						// Scale: Bone parent anti-scale (so that only the bone itself scales, not its children bones)
						float ParentStretchScale = 1.0f;
						float StretchAntiScale = 1.0f;
						int ParentBoneIndex = SkeletonBoneParents[BoneIndex];
						if (PolygonSkeletonData.Bones.IsValidIndex(ParentBoneIndex) && PolygonSkeletonData.Bones[ParentBoneIndex].Validity && ParentBoneIndex != (int)EManusBoneName::Root)
						{
							ParentStretchScale = PolygonSkeletonData.Bones[ParentBoneIndex].Transform.GetScale3D().X;
							StretchAntiScale = 1.0f / ParentStretchScale;
						}

						// Scale: Ref bone
						FVector RefScale = FVector::OneVector;
						FVector* RefScalePtr = ManusSkeleton->InitialScales.Find(BoneIndex);
						if (RefScalePtr)
						{
							RefScale = *RefScalePtr;
						}

						// Scale: Final scale
						float FinalStretchScale = StretchScale * StretchAntiScale;
						switch (PolygonSkeletonStretchAxis)
						{
						case EBoneAxis::BA_X: RefScale.X *= FinalStretchScale; break; // Use Manus coordinate system X -> Z
						case EBoneAxis::BA_Y: RefScale.Y *= FinalStretchScale; break; // Use Manus coordinate system Y -> X
						case EBoneAxis::BA_Z: RefScale.Z *= FinalStretchScale; break; // Use Manus coordinate system Z -> Y
						}
						LocalScales[BoneIndex] = RefScale;

						// Convert scale to component space
						while (PolygonSkeletonData.Bones.IsValidIndex(ParentBoneIndex))
						{
							if (PolygonSkeletonData.Bones[ParentBoneIndex].Validity)
							{
								RefScale *= LocalScales[ParentBoneIndex];
							}
							ParentBoneIndex = SkeletonBoneParents[ParentBoneIndex];
						}
						OutTransforms[BoneIndex].SetScale3D(RefScale);

						// Convert back to Unreal unit / coordinate system
						OutTransforms[BoneIndex] = OutTransforms[BoneIndex];
					}
				}
				else
				{
					OutTransforms[BoneIndex].SetScale3D(FVector::ZeroVector);
				}
			}
		}
		else
		{
			// Zero the scale to let the anim node know that there was no valid data
			for (int BoneIndex = 0; BoneIndex < BODY_LIVE_LINK_BONE_NUM; BoneIndex++)
			{
				OutTransforms[BoneIndex].SetScale3D(FVector::ZeroVector);
			}
		}
	}
}

void FManusLiveLinkSource::UpdateGloveLiveLinkTransforms(float DeltaTime, int ManusLiveLinkUserIndex, EManusHandType HandType, TArray<FTransform>& OutTransforms, int64& LastUpdateTime, bool& ForceUpdate)
{
	SCOPE_CYCLE_COUNTER(STAT_Manus_UpdateGloveLiveLink);

	LastUpdateTime = 0;
	ForceUpdate = false;

	// Retrieve the Manus Skeleton
	UManusSkeleton* ManusSkeleton = NULL;
	TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	if (ManusLiveLinkUsers.IsValidIndex(ManusLiveLinkUserIndex))
	{
		ManusSkeleton = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusSkeleton;
	}

	// Animation variables
	bool bWasHandAnimated = false;
	const int FirstGloveBoneIndex = (HandType == EManusHandType::Left ? (int)EManusBoneName::LeftHandThumb1 : (int)EManusBoneName::RightHandThumb1);

#if WITH_EDITORONLY_DATA
	// Preview hand gesture pose if needed
	if (ManusSkeleton && ManusLiveLinkUsers[ManusLiveLinkUserIndex].bIsHandPreviewOnGoing)
	{
		// Fingers tracking: Send stretch and spread values
		for (int Finger = 0; Finger < (int)EManusFingerName::Max; Finger++) // Thumb -> Pinky
		{
			// Spread preview
			FQuat SpreadQuaternion = FQuat::Identity;
			if (ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewSettings.PreviewType == EManusHandPreviewType::Spread)
			{
				float SpreadValue = ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewSettings.GetManusHandPreviewValue(
					ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewBounds->FingerSpreadExtent.GetLowerBoundValue(),
					ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewBounds->FingerSpreadExtent.GetUpperBoundValue(),
					ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewLoopProgress
				);
				float LowerBoundValue = ManusSkeleton->HandsAnimationSetup[(int)HandType].FingersRotationsExtents[Finger].FingerSpreadExtent.GetLowerBoundValue();
				float UpperBoundValue = ManusSkeleton->HandsAnimationSetup[(int)HandType].FingersRotationsExtents[Finger].FingerSpreadExtent.GetUpperBoundValue();
				float SpreadAngle = LowerBoundValue + (SpreadValue * (UpperBoundValue - LowerBoundValue));
				SpreadQuaternion = ManusTools::AngleAxis(SpreadAngle, (Finger == (int)EManusFingerName::Thumb) ? ManusSkeleton->HandsAnimationSetup[(int)HandType].ThumbSpreadRotationAxis : ManusSkeleton->HandsAnimationSetup[(int)HandType].FingersSpreadRotationAxis);
			}

			// Go through all the joints of all the fingers
			for (int Joint = 0; Joint < (int)EManusPhalangeName::Max; Joint++) // Proximal Joints -> Distal Joints
			{
				// Stretch preview
				FQuat StretchQuaternion = FQuat::Identity;
				if (ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewSettings.PreviewType == EManusHandPreviewType::Stretch)
				{
					float StretchValue = ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewSettings.GetManusHandPreviewValue(
						ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewBounds->JointsStretchExtents[Joint].GetLowerBoundValue(),
						ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewBounds->JointsStretchExtents[Joint].GetUpperBoundValue(),
						ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewLoopProgress
					);
					float LowerBoundValue = ManusSkeleton->HandsAnimationSetup[(int)HandType].FingersRotationsExtents[Finger].JointsStretchExtents[Joint].GetLowerBoundValue();
					float UpperBoundValue = ManusSkeleton->HandsAnimationSetup[(int)HandType].FingersRotationsExtents[Finger].JointsStretchExtents[Joint].GetUpperBoundValue();
					float StretchAngle = LowerBoundValue + (StretchValue * (UpperBoundValue - LowerBoundValue));
					StretchQuaternion = ManusTools::AngleAxis(StretchAngle, (Finger == (int)EManusFingerName::Thumb) ? ManusSkeleton->HandsAnimationSetup[(int)HandType].ThumbStretchRotationAxis : ManusSkeleton->HandsAnimationSetup[(int)HandType].FingersStretchRotationAxis);
				}

				// Set the rotation
				int TransformIndex = FirstGloveBoneIndex + Finger * (int)EManusPhalangeName::Max + Joint;
				OutTransforms[TransformIndex].SetRotation(Joint == 0 ? SpreadQuaternion * StretchQuaternion : StretchQuaternion);
				OutTransforms[TransformIndex].SetScale3D(FVector::OneVector);
			}
		}

		// Reset hand bones
		const int HandBoneIndex = (HandType == EManusHandType::Left ? (int)EManusBoneName::LeftHand : (int)EManusBoneName::RightHand);
		const int HandTrackerBoneIndex = (HandType == EManusHandType::Left ? (int)EManusBoneName::LeftHandTracker : (int)EManusBoneName::RightHandTracker);
		OutTransforms[HandBoneIndex].SetScale3D(FVector::ZeroVector);
		OutTransforms[HandTrackerBoneIndex].SetScale3D(FVector::ZeroVector);

		ForceUpdate = true;
		bWasHandAnimated = true;
	}
#endif // WITH_EDITORONLY_DATA
	
	// Get Manus Dashboard User index
	int ManusDashboardUserIndex = INDEX_NONE;
	if (ManusLiveLinkUsers.IsValidIndex(ManusLiveLinkUserIndex))
	{
		ManusDashboardUserIndex = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusDashboardUserIndex;
	}

	// We need a valid Manus Dashboard User index for the rest
	if (ManusDashboardUserIndex != INDEX_NONE)
	{
		// If we haven't animated the hand yet by another way, use actual Manus glove data to animate the hand
		if (!bWasHandAnimated)
		{
			// Get Glove from the Manus Live Link User index
			int64 GloveId = (HandType == EManusHandType::Left ? ManusLiveLinkUsers[ManusLiveLinkUserIndex].GetLeftGloveId() : ManusLiveLinkUsers[ManusLiveLinkUserIndex].GetRightGloveId());

			// Glove data
			FManusGlove GloveData;
			if (ManusSkeleton && GloveId > 0 && UManusBlueprintLibrary::GetGloveData(GloveId, GloveData) == EManusRet::Success)
			{
				LastUpdateTime = GloveData.LastUpdateTime;

				// Fingers tracking: Send stretch and spread values
				for (int Finger = 0; Finger < GloveData.Fingers.Num(); Finger++) // Thumb -> Pinky
				{
					// Turn the spread raw value into a quaternion
					float LowerBoundValue = ManusSkeleton->HandsAnimationSetup[(int)HandType].FingersRotationsExtents[Finger].FingerSpreadExtent.GetLowerBoundValue();
					float UpperBoundValue = ManusSkeleton->HandsAnimationSetup[(int)HandType].FingersRotationsExtents[Finger].FingerSpreadExtent.GetUpperBoundValue();
					float SpreadAngle = LowerBoundValue + (GloveData.Fingers[Finger].Spread * (UpperBoundValue - LowerBoundValue));
					EManusAxisOption SpreadAxis = (Finger == (int)EManusFingerName::Thumb) ? ManusSkeleton->HandsAnimationSetup[(int)HandType].ThumbSpreadRotationAxis : ManusSkeleton->HandsAnimationSetup[(int)HandType].FingersSpreadRotationAxis;
					if (HandType == EManusHandType::Right && Finger != (int)EManusFingerName::Thumb)
					{
						// Spread values sent by Core are not mirrored, so let's mirror the rotation here
						SpreadAxis = (EManusAxisOption)(((int)SpreadAxis + 3) % (int)EManusFingerName::Max);
					}
					FQuat SpreadQuaternion = ManusTools::AngleAxis(SpreadAngle, SpreadAxis);

					// Go through all the joints of all the fingers
					for (int Joint = 0; Joint < GloveData.Fingers[Finger].Joints.Num(); Joint++) // Proximal Joints -> Distal Joints
					{
						// Turn the stretch value into a quaternion
						LowerBoundValue = ManusSkeleton->HandsAnimationSetup[(int)HandType].FingersRotationsExtents[Finger].JointsStretchExtents[Joint].GetLowerBoundValue();
						UpperBoundValue = ManusSkeleton->HandsAnimationSetup[(int)HandType].FingersRotationsExtents[Finger].JointsStretchExtents[Joint].GetUpperBoundValue();
						float StretchAngle = LowerBoundValue + (GloveData.Fingers[Finger].Joints[Joint].Stretch * (UpperBoundValue - LowerBoundValue));
						FQuat StretchQuaternion = ManusTools::AngleAxis(StretchAngle, (Finger == (int)EManusFingerName::Thumb) ? ManusSkeleton->HandsAnimationSetup[(int)HandType].ThumbStretchRotationAxis : ManusSkeleton->HandsAnimationSetup[(int)HandType].FingersStretchRotationAxis);

						// Set the rotation
						int TransformIndex = FirstGloveBoneIndex + Finger * (int)EManusPhalangeName::Max + Joint;
						OutTransforms[TransformIndex].SetRotation(Joint == 0 ? SpreadQuaternion * StretchQuaternion : StretchQuaternion);
						OutTransforms[TransformIndex].SetScale3D(FVector::OneVector);
					}
				}

				// Gesture detection
				DetectGesture(DeltaTime, ManusLiveLinkUserIndex, HandType, GloveData);

				// Hand orientation from Glove IMU
				const int HandBoneIndex = (HandType == EManusHandType::Left ? (int)EManusBoneName::LeftHand : (int)EManusBoneName::RightHand);
				OutTransforms[HandBoneIndex].SetRotation(GloveData.WristImuOrientation * ManusSkeleton->TrackingDeviceToManusGloveTransform);
				OutTransforms[HandBoneIndex].SetScale3D(FVector::OneVector);

				// Hand position and orientation from Tracker data
				bool HasValidTrackingData = false;
				FTransform TrackerTransform;
				switch (GetDefault<UManusSettings>()->HandTrackingMethod)
				{
				case EManusHandTrackingMethod::ManusCore:
				{
					FManusTracker TrackerData;
					if (UManusBlueprintLibrary::GetTrackerData(ManusDashboardUserIndex, HandType, TrackerData) == EManusRet::Success)
					{
						HasValidTrackingData = true;
						TrackerTransform = TrackerData.Transform;
					}
					break;
				}

				case EManusHandTrackingMethod::Unreal:
				{
					FManusTracker TrackerData;
					if (UManusBlueprintLibrary::GetTrackerData(ManusDashboardUserIndex, HandType, TrackerData) == EManusRet::Success)
					{
						FVector TrackingDevicePosition;
						FRotator TrackingDeviceRotation;
						if (FManusModule::Get().GetTrackingManager()->GetTrackingDevicePositionAndRotation(FName(*TrackerData.TrackerId), TrackingDevicePosition, TrackingDeviceRotation))
						{
							HasValidTrackingData = true;
							TrackerTransform.SetLocation(TrackingDevicePosition);
							TrackerTransform.SetRotation(TrackingDeviceRotation.Quaternion());
							TrackerTransform.SetScale3D(FVector::OneVector);
						}
					}
				}
				}
				const int HandTrackerBoneIndex = (HandType == EManusHandType::Left ? (int)EManusBoneName::LeftHandTracker : (int)EManusBoneName::RightHandTracker);
				if (HasValidTrackingData)
				{
					// Offset hand bone according to accomodate with the actual Tracking Device position / rotation according to the glove type
					if (GloveData.WristTrackerTransforms.IsValidIndex(ManusDashboardUserIndex) && !GloveData.WristTrackerTransforms[ManusDashboardUserIndex].GetScale3D().IsZero())
						TrackerTransform = GloveData.WristTrackerTransforms[ManusDashboardUserIndex] * TrackerTransform;

					OutTransforms[HandTrackerBoneIndex] = FTransform(ManusSkeleton->TrackingDeviceToManusGloveTransform, FVector::ZeroVector, FVector::OneVector) * TrackerTransform /*GetDefault<UManusSettings>()->TrackingDeviceToManusGloveTransform[(int)GloveData.GloveInfo.GloveType] **/;
				}
				else
				{
					OutTransforms[HandTrackerBoneIndex].SetScale3D(FVector::ZeroVector);
				}

				bWasHandAnimated = true;
			}
		}
	}

	// Zero the scales to let the anim node know that there was no valid data
	if (!bWasHandAnimated)
	{
		const int HandBoneIndex = (HandType == EManusHandType::Left ? (int)EManusBoneName::LeftHand : (int)EManusBoneName::RightHand);
		const int HandTrackerBoneIndex = (HandType == EManusHandType::Left ? (int)EManusBoneName::LeftHandTracker : (int)EManusBoneName::RightHandTracker);
		OutTransforms[HandBoneIndex].SetScale3D(FVector::ZeroVector);
		OutTransforms[HandTrackerBoneIndex].SetScale3D(FVector::ZeroVector);

		for (int Finger = 0; Finger < (int)EManusFingerName::Max; Finger++) // Thumb -> Pinky
		{
			for (int Joint = 0; Joint < (int)EManusPhalangeName::Max; Joint++) // Proximal Joints -> Distal Joints
			{
				int TransformIndex = FirstGloveBoneIndex + Finger * (int)EManusPhalangeName::Max + Joint;
				OutTransforms[TransformIndex].SetScale3D(FVector::ZeroVector);
			}
		}
	}
}

#if WITH_EDITOR
void FManusLiveLinkSource::PreviewGesture(int GestureIndex)
{
	const UManusSettings* ManusSettings = GetDefault<UManusSettings>();
	if (ManusSettings->HandGestureDescriptors.IsValidIndex(GestureIndex) && ManusSettings->HandGesturePreviewSettings.PreviewMode != EManusSkeletonPreviewMode::NoPreview && ManusSettings->HandGesturePreviewSettings.PreviewDuration > 0.0f)
	{
		// Preview bounds
		FFingerRotationsExtents PreviewBounds[(int)EManusFingerName::Max];
		for (int Finger = 0; Finger < (int)EManusFingerName::Max; Finger++) // Thumb -> Pinky
		{
			// Don't preview spread
			PreviewBounds[Finger].FingerSpreadExtent = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(0.0f));

			const FFingerGestureDescriptor& FingerGestureDescriptor = ManusSettings->HandGestureDescriptors[GestureIndex].FingerGestureDescriptors[Finger];
			for (int Joint = 0; Joint < (int)EManusPhalangeName::Max; Joint++) // Proximal Joints -> Distal Joints
			{
				// Preview gesture stretch bounds
				PreviewBounds[Finger].JointsStretchExtents[Joint] = FingerGestureDescriptor.FlexSensorRanges[FMath::Min(Joint, (int)EManusFingerFlexSensorType::Max - 1)];
			}
		}

		// Start preview on all the concerned Manus Live Link Users
		TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
		for (int ManusLiveLinkUserIndex = 0; ManusLiveLinkUserIndex < ManusLiveLinkUsers.Num(); ManusLiveLinkUserIndex++)
		{
			ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewSettings = ManusSettings->HandGesturePreviewSettings;
			ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewGestureIndex = GestureIndex;
			ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewTimer = ManusSettings->HandGesturePreviewSettings.PreviewDuration;
			FMemory::Memcpy(ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewBounds, PreviewBounds);
		}

		UE_LOG(LogManus, Log, TEXT("Started previewing Manus hand gesture \"%s\"."), *ManusSettings->HandGestureDescriptors[GestureIndex].GestureName.ToString());
	}
}

void FManusLiveLinkSource::PreviewSkeletonFingerRotationExtents(UManusSkeleton* ManusSkeleton)
{
	if (ManusSkeleton)
	{
		// Preview bounds
		FFingerRotationsExtents HandGestureFingersRotationsBounds[(int)EManusFingerName::Max];
		for (int Finger = 0; Finger < (int)EManusFingerName::Max; Finger++) // Thumb -> Pinky
		{
			// Don't preview spread
			HandGestureFingersRotationsBounds[Finger].FingerSpreadExtent = FFloatRange(FFloatRangeBound::Inclusive(-1.0f), FFloatRangeBound::Inclusive(1.0f));

			for (int Joint = 0; Joint < (int)EManusPhalangeName::Max; Joint++) // Proximal Joints -> Distal Joints
			{
				// Preview stretch bounds
				HandGestureFingersRotationsBounds[Finger].JointsStretchExtents[Joint] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
			}
		}

		// Start preview on all the concerned Manus Live Link Users
		TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
		for (int ManusLiveLinkUserIndex = 0; ManusLiveLinkUserIndex < ManusLiveLinkUsers.Num(); ManusLiveLinkUserIndex++)
		{
			if (ManusLiveLinkUsers.IsValidIndex(ManusLiveLinkUserIndex) && ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusSkeleton == ManusSkeleton)
			{
				FManusHandPreviewSettings PreviewSettings = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusSkeleton->FingersRotationsExtentsPreviewSettings;
				if (PreviewSettings.PreviewMode != EManusSkeletonPreviewMode::NoPreview && PreviewSettings.PreviewDuration > 0.0f)
				{
					ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewSettings = PreviewSettings;
					ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewGestureIndex = INDEX_NONE;
					ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewTimer = PreviewSettings.PreviewDuration;
					FMemory::Memcpy(ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewBounds, HandGestureFingersRotationsBounds);
				}
			}
		}

		UE_LOG(LogManus, Log, TEXT("Started previewing Manus fingers rotations extents."));
	}
}

void FManusLiveLinkSource::TickAllHandPreviews(float DeltaTime)
{
	const UManusSettings* ManusSettings = GetDefault<UManusSettings>();
	TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	for (int ManusLiveLinkUserIndex = 0; ManusLiveLinkUserIndex < ManusLiveLinkUsers.Num(); ManusLiveLinkUserIndex++)
	{
		if (ManusLiveLinkUsers.IsValidIndex(ManusLiveLinkUserIndex))
		{
			ManusLiveLinkUsers[ManusLiveLinkUserIndex].bIsHandPreviewOnGoing = (ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewTimer > 0.0f);
			if (ManusLiveLinkUsers[ManusLiveLinkUserIndex].bIsHandPreviewOnGoing)
			{
				ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewTimer -= FMath::Max(0.0f, DeltaTime);

				const float LoopDuration = ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewSettings.PreviewDuration / FMath::Max(1, ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewSettings.PreviewAnimationLoops);
				const float LoopTimer = FMath::Fmod(ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewSettings.PreviewDuration - ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewTimer, LoopDuration);
				const float LoopProgress = LoopTimer / LoopDuration;
				if (LoopProgress <= 0.5f)
				{
					ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewLoopProgress = LoopProgress * 2.0f;
				}
				else
				{
					ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewLoopProgress = 1.0f - ((LoopProgress - 0.5f) * 2.0f);
				}

				if (ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewTimer == 0.0f)
				{
					if (ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewGestureIndex != INDEX_NONE)
					{
						UE_LOG(LogManus, Log, TEXT("Finished previewing Manus hand gesture \"%s\"."), *ManusSettings->HandGestureDescriptors[ManusLiveLinkUsers[ManusLiveLinkUserIndex].OnGoingHandPreviewGestureIndex].GestureName.ToString());
					}
					else
					{
						UE_LOG(LogManus, Log, TEXT("Finished previewing Manus fingers rotations extents."));
					}
				}
			}
		}
	}
}
#endif // WITH_EDITOR

void FManusLiveLinkSource::DetectGesture(float DeltaTime, int ManusLiveLinkUserIndex, EManusHandType HandType, FManusGlove& GloveData)
{
	SCOPE_CYCLE_COUNTER(STAT_Manus_GestureDetection);

	TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	const TArray<FHandGestureDescriptor>& HandGestureDescriptors = GetDefault<UManusSettings>()->HandGestureDescriptors;
	ManusLiveLinkUsers[ManusLiveLinkUserIndex].HandGestureDetectionData[(int)HandType].ManusLiveLinkUserDetectedHandGestureTimers.SetNum(HandGestureDescriptors.Num());
	for (int GestureIndex = 0; GestureIndex < HandGestureDescriptors.Num(); GestureIndex++)
	{
		const FHandGestureDescriptor& HandGestureDescriptor = HandGestureDescriptors[GestureIndex];

		bool GestureDetected = true;

		for (int Finger = 0; Finger < (int)EManusFingerName::Max && GestureDetected; Finger++)
		{
			const FFingerGestureDescriptor& FingerGestureDescriptor = HandGestureDescriptor.FingerGestureDescriptors[Finger];
			if (!FingerGestureDescriptor.FlexSensorRanges[(int)EManusFingerFlexSensorType::First].Contains(GloveData.RawData.Fingers[Finger].McpFlexSensor)
			||	!FingerGestureDescriptor.FlexSensorRanges[(int)EManusFingerFlexSensorType::Second].Contains(GloveData.RawData.Fingers[Finger].PipFlexSensor)
			) {
				GestureDetected = false;
			}
		}

		if (GestureDetected)
		{
			ManusLiveLinkUsers[ManusLiveLinkUserIndex].HandGestureDetectionData[(int)HandType].ManusLiveLinkUserDetectedHandGestureTimers[GestureIndex] += DeltaTime;
		}
		else
		{
			ManusLiveLinkUsers[ManusLiveLinkUserIndex].HandGestureDetectionData[(int)HandType].ManusLiveLinkUserDetectedHandGestureTimers[GestureIndex] = 0.0f;
		}
	}
}

void FManusLiveLinkSource::InitPolygonForAllManusLiveLinkUsers(bool ResetRetry)
{
	TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	for (int i = 0; i < ManusLiveLinkUsers.Num(); i++)
	{
		InitPolygonForManusLiveLinkUser(i, ResetRetry);
	}
}

void FManusLiveLinkSource::InitPolygonForManusLiveLinkUser(int ManusLiveLinkUserIndex, bool ResetRetry)
{
	TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	if (ManusLiveLinkUsers.IsValidIndex(ManusLiveLinkUserIndex))
	{
		UManusSkeleton* ManusSkeleton = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusSkeleton;
		if (ManusSkeleton && ManusSkeleton->bIsUsedForFullBodyTracking && ManusSkeleton->GetSkeleton())
		{
			UE_LOG(LogManus, Log, TEXT("Initializing Manus Polygon for skeleton %s."), *ManusSkeleton->GetSkeleton()->GetName());

			const BoneName_t* ManusLiveLinkUserPolygonSkeletonBoneNameMap = ManusSkeleton->BoneMap;

			// Bone map and scales
			TArray<FTransform> ManusLiveLinkUserPolygonSkeletonRefBoneTransforms = ManusSkeleton->GetSkeleton()->GetReferenceSkeleton().GetRefBonePose();
			for (int i = 0; i < (int)EManusBoneName::Max; i++)
			{
				FName MappedBoneName = GET_BONE_NAME(ManusLiveLinkUserPolygonSkeletonBoneNameMap[i]);
				int32 BoneIndex = ManusSkeleton->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(MappedBoneName);
				if (BoneIndex != INDEX_NONE)
				{
					ManusSkeleton->BoneIndexMap.Add(i, BoneIndex);
					ManusSkeleton->InitialScales.Add(i, ManusLiveLinkUserPolygonSkeletonRefBoneTransforms[BoneIndex].GetScale3D());
				}
			}

			// Calculate Manus Core internal transforms
			if (ManusTools::CalculateManusInternalOrientations(
				ManusLiveLinkUserIndex, 
				ManusSkeleton->ManusInternalOrientations,
				ManusSkeleton->ManusInternalDeltaOrientations)
			) {
				// Start Polygon for this Manus Live Link User
				EManusRet ReturnCode = CoreSdk::AddOrUpdatePolygonSkeleton(
					ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusDashboardUserIndex,
					ManusTools::GenerateManusIdFromManusLiveLinkUser(ManusLiveLinkUserIndex),
					ManusSkeleton
				);
				if (ReturnCode != EManusRet::Success)
				{
					if (ResetRetry)
					{
						ManusLiveLinkUsers[ManusLiveLinkUserIndex].PolygonInitializationRetryNumber = 0;
					}
					ManusLiveLinkUsers[ManusLiveLinkUserIndex].PolygonInitializationRetryNumber++;
					ManusLiveLinkUsers[ManusLiveLinkUserIndex].PolygonInitializationRetryCountdown = FMath::Pow(10, FMath::Min(3, ManusLiveLinkUsers[ManusLiveLinkUserIndex].PolygonInitializationRetryNumber));

					UE_LOG(LogManus, Warning, TEXT("Manus Polygon initialization failed for Skeleton %s. Next attempt in %d frames."), *ManusSkeleton->GetName(), ManusLiveLinkUsers[ManusLiveLinkUserIndex].PolygonInitializationRetryCountdown);
				}
			}
			else
			{
				UE_LOG(LogManus, Warning, TEXT("Manus Polygon initialization failed for Skeleton %s."), *ManusSkeleton->GetName());
			}
		}
	}
}

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 23
bool FManusLiveLinkSource::IsSubjectEnabled(FName LiveLinkSubjectName)
{
	if (LiveLinkClient)
	{
		TArray<FName> SubjectNames;
		LiveLinkClient->GetSubjectNames(SubjectNames);
		return SubjectNames.Contains(LiveLinkSubjectName);
	}
	return false;
}
#endif

#undef LOCTEXT_NAMESPACE
