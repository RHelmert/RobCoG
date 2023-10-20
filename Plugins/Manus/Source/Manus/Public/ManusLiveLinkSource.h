#pragma once

// Set up a Doxygen group.
/** @addtogroup FManusLiveLinkSource
 *  @{
 */

#include "CoreMinimal.h"
#include "ILiveLinkSource.h"
#include "Tickable.h"
#include "ManusBlueprintTypes.h"
#include "ManusReplicator.h"

#include "ManusLiveLinkUser.h"

#include <map>
#include <string>

#define RECONNECT_RETRY_COUNT_MAX 60


/// @brief The types of Manus Live Link sources.
enum class EManusLiveLinkSourceType : uint8
{
	Local,
	Replicated,
	Max
};

/// @brief Glove assignment.
struct FManusGloveAssignment
{
    /// @brief  The IDs of the Gloves assigned.
	int64 GloveIds[(int)EManusHandType::Max];

	FManusGloveAssignment()
	{
		for (int i = 0; i < (int)EManusHandType::Max; i++)
		{
			GloveIds[i] = 0;
		}
	}
};

/// @brief The main module for the plugin that implements Manus glove support.
class MANUS_API FManusLiveLinkSource : public ILiveLinkSource, public FTickableGameObject
{
public:
	/// @brief sets up FManusLiveLinkSource with EManusLiveLinkSourceType 
	/// @param InSourceType 
	FManusLiveLinkSource(EManusLiveLinkSourceType InSourceType);

	/// @brief 
	virtual void Init();
	/// @brief 
	virtual void Destroy();

	/** FTickableObject Interface */

    /// @brief TODO
    void TickUpdateSkeletons();

    /// @brief TODO
    bool TickConnection();

	/// @brief every time unreal wants to update anything it calls this function.
    /// in this everything manus related is updated. for more details also check the unreal FTickableObject documentation.
	/// @param DeltaTime 
	virtual void Tick(float DeltaTime) override;
	/// @brief 
	/// @return TStatId of the current livelink
	virtual TStatId GetStatId() const override;
	/// @brief always returns ETickableTickType::Always
	/// @return 
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	/// @brief always returns true.
	/// @return 
	virtual bool IsTickableWhenPaused() const override { return true; }
	/// @brief  always returns true
	/// @return 
	virtual bool IsTickableInEditor() const override { return true; }
	/** FTickableObject Interface */

	/** ILiveLinkSource Interface */

	/// @brief adds client
    /// see ILiveLinkSource for more details
	/// @param InClient 
	/// @param InSourceGuid 
	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
	/// @brief check if source still set
    /// see ILiveLinkSource for more details
	/// @return true if m_LiveLinkClient is not null.
	virtual bool IsSourceStillValid() const override;
	/// @brief shut down and clean up source
    /// see ILiveLinkSource for more details
	/// @return true
	virtual bool RequestSourceShutdown() override;
	/// @brief 
    /// see ILiveLinkSource for more details
	/// @return name of this pc
	virtual FText GetSourceMachineName() const override;
	/// @brief get source status
    /// see ILiveLinkSource for more details
	/// @return LOCTEXT("ManusLiveLinkStatus", "Active")
	virtual FText GetSourceStatus() const override;
	/// @brief get sourcetype in FText form
    /// see ILiveLinkSource for more details
    /// @return sourcetype converted to FText
	virtual FText GetSourceType() const override;
	/** ILiveLinkSource Interface */

public:
	/// @brief 
	/// @return m_LiveLinkClient
	ILiveLinkClient* GetLiveLinkClient() { return m_LiveLinkClient; }
	/// @brief sets buffer offset in EngineTimeOffset settings
	/// @param p_Offset 
	void SetBufferOffset(float p_Offset);
	/// @brief 
	/// @param p_ManusComponent 
	/// @return generated manus livelink name 
	static FName GetManusLiveLinkUserLiveLinkSubjectName(const class UManusComponent* p_ManusComponent);
	/// @brief 
	/// @param p_ManusLiveLinkUserIndex 
	/// @param p_ReplicatorId 
	/// @return generated manus livelink name
	static FName GetManusLiveLinkUserLiveLinkSubjectName(int p_ManusLiveLinkUserIndex, int32 p_ReplicatorId = 0);
	/// @brief replicates live link with manus data in mind.
	/// @param p_Replicator 
	void ReplicateLiveLink(class AManusReplicator* p_Replicator);
	/// @brief stop the replicator link
	/// @param p_Replicator 
	void StopReplicatingLiveLink(class AManusReplicator* p_Replicator);
	/// @brief initialize all the manus skeletons so they can be animated
	/// @param p_ResetRetry     retry in case of a reconnect.
	/// @param p_ManusLiveLinkUsers list of manuslivelink users for which to init the skeletons.
	void InitSkeletons(bool p_ResetRetry, TArray<FManusLiveLinkUser>& p_ManusLiveLinkUsers);

private:
	/// @brief initialize the individual skeleton for 1 manuslivelinkuser and user id
	/// @param p_ManusLiveLinkUserIndex 
	/// @param p_ManusUserId     this is value is only used in case it is not set in the umanusskeleton and we use targetuserid for the umanusskeleton (otherwise it uses the already present values in the umanusskeleton)
	/// @param ClearTempValues 
	void InitSkeletonsForManusLiveLinkUser(int p_ManusLiveLinkUserIndex, uint32_t p_ManusUserId, bool ClearTempValues = false);
	/// @brief called every tick to update all the data used.
	/// @param p_DeltaTime 
	/// @param p_ManusLiveLinkUserIndex 
	void UpdateLiveLink(float p_DeltaTime, int p_ManusLiveLinkUserIndex);
	/// @brief recreate livelink subject for user.
	/// @param p_LiveLinkSubjectKey 
	/// @param p_ManusLiveLinkUserIndex 
	void RecreateLiveLinkSubject(FLiveLinkSubjectKey& p_LiveLinkSubjectKey,int p_ManusLiveLinkUserIndex);
	/// @brief update animation data for a skeleton for a specific manuslivelinkuser and fills in the lastupdate time based on when the data was received.
	/// @param p_ManusLiveLinkUserIndex 
	/// @param p_OutTransforms 
	/// @param p_LastUpdateTime 
	/// @return 
	bool UpdateSkeletonsLiveLinkTransforms(int p_ManusLiveLinkUserIndex, TArray<FTransform>& p_OutTransforms, uint64& p_LastUpdateTime);

    /// @brief show a hot reload notification in unreal editor
    void HotReloadNotification();

public:
	/// @brief Identifier in LiveLink 
	FGuid liveLinkSourceGuid;

    /// @brief Data to replicate 
	TArray<FManusReplicatedFrameData> m_ReplicatedFrameDataArray;


    /// @brief The local client to push data updates to 
	ILiveLinkClient* m_LiveLinkClient = nullptr;

private:
    /// @brief Manus Live Link source type 
	EManusLiveLinkSourceType m_SourceType;


    /// @brief Whether the Live Link client is new 
	bool m_bNewLiveLinkClient = false;

    /// @brief Whether the connection with Core is currently timing out. 
	bool m_bIsConnectionWithCoreTimingOut = false;
    /// @brief flag if the skeletons are initialized
	bool m_bIsSkeletonsInitialized = false;

    /// @brief How much time left before the next glove assignment update. 
	float m_ManusDashboardUserGloveAssignmentUpdateTimer = 0.0f;

    /// @brief The Manus Dashboard User glove assignment cache. 
	TMap<int, FManusGloveAssignment> m_ManusDashboardUserGloveAssignmentCache;

    /// @brief The indexes of the Manus Dashboard Users we still need to update the glove assignment from.
	TArray<float> m_ManusDashboardUserGloveAssignmentUpdateQueue;
    
    /// @brief in case of reconnecting to %Manus Core
	int m_ReconnectCounter = 0;

};

// Close the Doxygen group.
/** @} */
