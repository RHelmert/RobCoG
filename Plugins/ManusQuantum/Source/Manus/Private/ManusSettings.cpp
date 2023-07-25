// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2022 Manus

#include "ManusSettings.h"
#include "Manus.h"
#include "ManusBlueprintTypes.h"
#include "ManusTools.h"
#include "ManusSkeleton.h"
#include "ManusComponent.h"


UManusSettings::UManusSettings()
{
	// General
	bUseManusInGame = true;

	// Users
	ManusDashboardUserGloveAssignmentUpdateFrequency = 3.0f;
	// Replication
	DefaultReplicationOffsetTime = 0.1f;
	bUpdateReplicationOffsetTimeUsingPing = true;
	ReplicationOffsetTimePingMultiplier = 1.5f;
	ReplicationOffsetTimePingMultiplier = 0.1f;

	// Tracking
	TrackingSmoothing = 0.025f;
	TrackingManagerDeviceUpdateFrequency = 2.5f;
}

#if WITH_EDITOR
void UManusSettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif //WITH_EDITOR
