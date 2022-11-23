// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#pragma once

#include "CoreTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "ManusBlueprintTypes.h"
#include "ManusSettings.generated.h"


/**
 * Settings for Manus plugin.
 */
UCLASS(config=Game, defaultconfig)
class MANUS_API UManusSettings : public UObject
{
	GENERATED_BODY()

public:
	UManusSettings();

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR

public:
	/** Whether Manus should be used in-game. */
	UPROPERTY(config, EditAnywhere, Category = "General")
	bool bUseManusInGame;

	/** How often to update which gloves are assigned to the each User in the Manus Dashboard. */
	UPROPERTY(config, EditAnywhere, Category = "Users", meta = (ForceUnits = s, UIMin = 0.0f))
	float ManusDashboardUserGloveAssignmentUpdateFrequency;

	/** Array of hand gestures descriptors. They define the poses all the fingers of a hand must match to be recognized as a gesture. */
	UPROPERTY(config, EditAnywhere, Category = "Gestures")
	TArray<FHandGestureDescriptor> HandGestureDescriptors;

	/** Duration of the preview of a gesture after its values were edited. */
	UPROPERTY(config, EditAnywhere, Category = "Gestures")
	FManusHandPreviewSettings HandGesturePreviewSettings;

	/** How many seconds we should delay the tracking animation to cope with the tracking device having a lower update frequency. */
	UPROPERTY(config, EditAnywhere, Category = "Tracking", meta = (ForceUnits = s, UIMin = 0.0f))
	float TrackingSmoothing;

	/** Which hand tracking method to use. */
	UPROPERTY(config, EditAnywhere, Category = "Tracking")
	EManusHandTrackingMethod HandTrackingMethod;

	/** How often the Tracking Manager (used in the "Unreal" Hand Tracking method) should check for new tracking devices, in seconds. */
	UPROPERTY(config, EditAnywhere, Category = "Tracking", meta = (ForceUnits = s, UIMin = 0.0f))
	float TrackingManagerDeviceUpdateFrequency;

	/** How far back should we read the replicated Manus data (in seconds) to cope with latency. */
	UPROPERTY(config, EditAnywhere, Category = "Replication", meta = (ForceUnits = s, UIMin = 0.0f))
	float DefaultReplicationOffsetTime;

	/** Whether to update the Replication Offset Time at runtime according to the current ping. */
	UPROPERTY(config, EditAnywhere, Category = "Replication")
	bool bUpdateReplicationOffsetTimeUsingPing;

	/** Updated Replication Offset Time will be computed using the ping multiplied by this value. */
	UPROPERTY(config, EditAnywhere, Category = "Replication", meta = (EditCondition = "bUpdateReplicationOffsetTimeUsingPing"))
	float ReplicationOffsetTimePingMultiplier;

	/** Updated Replication Offset Time will be computed using the ping with this extra time added. */
	UPROPERTY(config, EditAnywhere, Category = "Replication", meta = (EditCondition = "bUpdateReplicationOffsetTimeUsingPing"))
	float ReplicationOffsetTimePingExtraTime;
};
