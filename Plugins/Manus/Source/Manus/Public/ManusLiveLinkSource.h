#pragma once

#include "CoreMinimal.h"
#include "ILiveLinkSource.h"
#include "Tickable.h"
#include "ManusBlueprintTypes.h"
#include "ManusReplicator.h"
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 23
#include "LiveLinkClient.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#endif

/**
 * The types of Manus Live Link sources.
 */
enum class EManusLiveLinkSourceType : uint8
{
	Local,
	Replicated,
	Max
};

/**
 * Glove assignment.
 */
struct FManusGloveAssignment
{
	/** The IDs of the Gloves assigned.	*/
	int64 GloveIds[(int)EManusHandType::Max];

	FManusGloveAssignment()
	{
		for (int i = 0; i < (int)EManusHandType::Max; i++)
		{
			GloveIds[i] = 0;
		}
	}
};


#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 23
class SLiveLinkManusSourceEditor : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SLiveLinkManusSourceEditor)
	{
	}

	SLATE_END_ARGS()

	~SLiveLinkManusSourceEditor() {}

	void Construct(const FArguments& Args) {}
};
#endif


/**
 * The main module for the plugin that implements Manus glove support.
 */
class MANUS_API FManusLiveLinkSource : public ILiveLinkSource, public FTickableGameObject
{
public:
	FManusLiveLinkSource(EManusLiveLinkSourceType InSourceType);

	virtual void Init();
	virtual void Destroy();

	/** FTickableObject Interface */
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	/** FTickableObject Interface */

	/** ILiveLinkSource Interface */
	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
	virtual bool IsSourceStillValid() const override;
#else
	virtual bool IsSourceStillValid() override;
#endif
	virtual bool RequestSourceShutdown() override;
	virtual FText GetSourceMachineName() const override;
	virtual FText GetSourceStatus() const override;
	virtual FText GetSourceType() const override;
	/** ILiveLinkSource Interface */

public:
	ILiveLinkClient* GetLiveLinkClient() { return LiveLinkClient; }

	void SetBufferOffset(float Offset);

	static FName GetManusLiveLinkUserLiveLinkSubjectName(const class UManusComponent* ManusComponent);
	static FName GetManusLiveLinkUserLiveLinkSubjectName(int ManusLiveLinkUserIndex, int32 ReplicatorId = 0);

	void ReplicateLiveLink(class AManusReplicator* Replicator);
	void StopReplicatingLiveLink(class AManusReplicator* Replicator);

	int64 GetCachedGloveAssignment(int ManusDashboardUserIndex, EManusHandType HandType);

	void InitPolygonForAllManusLiveLinkUsers(bool ResetRetry);
	void InitPolygonForManusLiveLinkUser(int ManusLiveLinkUserIndex, bool ResetRetry);

#if WITH_EDITOR
	void PreviewGesture(int GestureIndex);
	void PreviewSkeletonFingerRotationExtents(class UManusSkeleton* ManusSkeleton);
#endif // WITH_EDITOR

private:
	void UpdateManusDashboardUsersGloveAssignments(float DeltaTime);
	void UpdateLiveLink(float DeltaTime, int ManusLiveLinkUserIndex);
	void RecreateLiveLinkSubject(FLiveLinkSubjectKey& LiveLinkSubjectKey);
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
	void SetupLiveLinkData(struct FLiveLinkSkeletonStaticData& StaticData);
#else
	void SetupLiveLinkData(struct FLiveLinkRefSkeleton& StaticData);
#endif
	void UpdatePolygonLiveLinkTransforms(int ManusLiveLinkUserIndex, TArray<FTransform>& OutTransforms, int64& LastUpdateTime);
	void UpdateGloveLiveLinkTransforms(float DeltaTime, int ManusLiveLinkUserIndex, EManusHandType HandType, TArray<FTransform>& OutTransforms, int64& LastUpdateTime, bool& ForceUpdate);
	void DetectGesture(float DeltaTime, int ManusLiveLinkUserIndex, EManusHandType HandType, FManusGlove& GloveData);
#if WITH_EDITOR
	void TickAllHandPreviews(float DeltaTime);
#endif // WITH_EDITOR

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 23
	bool IsSubjectEnabled(FName LiveLinkSubjectName);
#endif


public:
	/** Identifier in LiveLink */
	FGuid LiveLinkSourceGuid;

	/** Data to replicate */
	TArray<FManusReplicatedFrameData> ReplicatedFrameDataArray;

private:
	/** Manus Live Link source type */
	EManusLiveLinkSourceType SourceType;

	/** The local client to push data updates to */
	ILiveLinkClient* LiveLinkClient = nullptr;

	/** Whether the Live Link client is new */
	bool bNewLiveLinkClient = false;

	/** Live Link frame */
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
	FLiveLinkFrameDataStruct LiveLinkFrame;
#else
	FLiveLinkFrameData LiveLinkFrame;
#endif

	/** Index of the parent of each bone in the Live Link skeleton */
	TArray<int> SkeletonBoneParents;

	/** Whether the connection with Core is currently timing out. */
	bool bIsConnectionWithCoreTimingOut = false;

	/** How much time left before the next glove assignment update. */
	float ManusDashboardUserGloveAssignmentUpdateTimer = 0.0f;

	/** The Manus Dashboard User glove assignment cache. */
	TMap<int, FManusGloveAssignment> ManusDashboardUserGloveAssignmentCache;

	/** The indexes of the Manus Dashboard Users we still need to update the glove assignment from. */
	TArray<float> ManusDashboardUserGloveAssignmentUpdateQueue;
};