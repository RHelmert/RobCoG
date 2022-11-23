// Copyright 2015-2020 Manus

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#if ENGINE_MAJOR_VERSION == 5 ||  ENGINE_MINOR_VERSION >= 23
#include "Roles/LiveLinkAnimationTypes.h"
#else
#include "LiveLinkTypes.h"
#endif

#include "ManusReplicator.generated.h"

//-----------------------------------------------------------------------------
//
USTRUCT()
struct FManusReplicatedFrameData
{
	GENERATED_BODY()

	UPROPERTY()
	int ManusDashboardUserIndex;
	
	UPROPERTY()
	class UManusSkeleton* ManusSkeleton;

	UPROPERTY()
	FTransform LeftHandTrackerTransform;

	UPROPERTY()
	FTransform LeftHandTransform;

	UPROPERTY()
	TArray<FQuat> LeftHandFingerAngles;

	UPROPERTY()
	FTransform RightHandTransform;

	UPROPERTY()
	FTransform RightHandTrackerTransform;

	UPROPERTY()
	TArray<FQuat> RightHandFingerAngles;

	UPROPERTY()
	TArray<FTransform> PolygonTransforms;
};

//-----------------------------------------------------------------------------
//
USTRUCT()
struct FManusReplicatedData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FManusReplicatedFrameData> ReplicatedFrameDataArray;
};

/**
 * AManusReplicator class is used to replicate the Manus data sent through Live Link
 * for each Manus Live Link User (one Manus Live Link User being associated with one Live Link subject).
 */
UCLASS(transient, notplaceable)
class AManusReplicator : public AActor
{
	GENERATED_BODY()

public:
	AManusReplicator();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool IsLiveLinkSourceLocal();

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
	void OnLiveLinkTicked();
#else
	virtual void Tick(float DeltaTime) override;
#endif
	
	UFUNCTION(Server, Unreliable, WithValidation)
	void SendReplicatedDataToServer(FManusReplicatedData DataToReplicate);

	UFUNCTION()
	void OnReplicatedDataReceivedFromServer();

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
	static void CompressReplicatedFrameData(const FLiveLinkAnimationFrameData& UncompressedFrameData, FManusReplicatedFrameData& CompressedFrameData);
	static void DecompressReplicatedFrameData(FLiveLinkAnimationFrameData& UncompressedFrameData, const FManusReplicatedFrameData& CompressedFrameData);
#else
	static void CompressReplicatedFrameData(const FLiveLinkFrameData& UncompressedFrameData, FManusReplicatedFrameData& CompressedFrameData);
	static void DecompressReplicatedFrameData(FLiveLinkFrameData& UncompressedFrameData, const FManusReplicatedFrameData& CompressedFrameData);
#endif

public:
	UPROPERTY(Transient, Replicated)
	int32 ReplicatorId;

	UPROPERTY(Transient, ReplicatedUsing = OnReplicatedDataReceivedFromServer)
	FManusReplicatedData ReplicatedData;
};
