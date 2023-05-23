// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

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

	// Pointing hand gesture
	FHandGestureDescriptor PointingGesture;
	PointingGesture.GestureName = "Pointing";
	PointingGesture.FingerGestureDescriptors[(int)EManusFingerName::Thumb].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	PointingGesture.FingerGestureDescriptors[(int)EManusFingerName::Thumb].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	PointingGesture.FingerGestureDescriptors[(int)EManusFingerName::Index].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(0.2f));
	PointingGesture.FingerGestureDescriptors[(int)EManusFingerName::Index].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(0.2f));
	PointingGesture.FingerGestureDescriptors[(int)EManusFingerName::Middle].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.05f), FFloatRangeBound::Inclusive(1.0f));
	PointingGesture.FingerGestureDescriptors[(int)EManusFingerName::Middle].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	PointingGesture.FingerGestureDescriptors[(int)EManusFingerName::Ring].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.05f), FFloatRangeBound::Inclusive(1.0f));
	PointingGesture.FingerGestureDescriptors[(int)EManusFingerName::Ring].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	PointingGesture.FingerGestureDescriptors[(int)EManusFingerName::Pinky].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.05f), FFloatRangeBound::Inclusive(1.0f));
	PointingGesture.FingerGestureDescriptors[(int)EManusFingerName::Pinky].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	HandGestureDescriptors.Add(PointingGesture);

	// Fist hand gesture
	FHandGestureDescriptor FistGesture;
	FistGesture.GestureName = "Fist";
	FistGesture.FingerGestureDescriptors[(int)EManusFingerName::Thumb].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.2f), FFloatRangeBound::Inclusive(1.0f));
	FistGesture.FingerGestureDescriptors[(int)EManusFingerName::Thumb].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	FistGesture.FingerGestureDescriptors[(int)EManusFingerName::Index].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.5f), FFloatRangeBound::Inclusive(1.0f));
	FistGesture.FingerGestureDescriptors[(int)EManusFingerName::Index].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	FistGesture.FingerGestureDescriptors[(int)EManusFingerName::Middle].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.5f), FFloatRangeBound::Inclusive(1.0f));
	FistGesture.FingerGestureDescriptors[(int)EManusFingerName::Middle].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	FistGesture.FingerGestureDescriptors[(int)EManusFingerName::Ring].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.5f), FFloatRangeBound::Inclusive(1.0f));
	FistGesture.FingerGestureDescriptors[(int)EManusFingerName::Ring].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	FistGesture.FingerGestureDescriptors[(int)EManusFingerName::Pinky].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.5f), FFloatRangeBound::Inclusive(1.0f));
	FistGesture.FingerGestureDescriptors[(int)EManusFingerName::Pinky].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	HandGestureDescriptors.Add(FistGesture);

	// Thumbs up hand gesture
	FHandGestureDescriptor ThumbsUpGesture;
	ThumbsUpGesture.GestureName = "ThumbsUp";
	ThumbsUpGesture.FingerGestureDescriptors[(int)EManusFingerName::Thumb].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(-1.0f), FFloatRangeBound::Inclusive(0.05f));
	ThumbsUpGesture.FingerGestureDescriptors[(int)EManusFingerName::Thumb].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(-1.0f), FFloatRangeBound::Inclusive(0.05f));
	ThumbsUpGesture.FingerGestureDescriptors[(int)EManusFingerName::Index].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	ThumbsUpGesture.FingerGestureDescriptors[(int)EManusFingerName::Index].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	ThumbsUpGesture.FingerGestureDescriptors[(int)EManusFingerName::Middle].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	ThumbsUpGesture.FingerGestureDescriptors[(int)EManusFingerName::Middle].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	ThumbsUpGesture.FingerGestureDescriptors[(int)EManusFingerName::Ring].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	ThumbsUpGesture.FingerGestureDescriptors[(int)EManusFingerName::Ring].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	ThumbsUpGesture.FingerGestureDescriptors[(int)EManusFingerName::Pinky].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	ThumbsUpGesture.FingerGestureDescriptors[(int)EManusFingerName::Pinky].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.6f), FFloatRangeBound::Inclusive(1.0f));
	HandGestureDescriptors.Add(ThumbsUpGesture);
	
	// Metal hand gesture
	FHandGestureDescriptor MetalGesture;
	MetalGesture.GestureName = "Metal";
	MetalGesture.FingerGestureDescriptors[(int)EManusFingerName::Thumb].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.4f), FFloatRangeBound::Inclusive(1.0f));
	MetalGesture.FingerGestureDescriptors[(int)EManusFingerName::Thumb].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	MetalGesture.FingerGestureDescriptors[(int)EManusFingerName::Index].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(-0.1f), FFloatRangeBound::Inclusive(0.1f));
	MetalGesture.FingerGestureDescriptors[(int)EManusFingerName::Index].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(-0.1f), FFloatRangeBound::Inclusive(0.1f));
	MetalGesture.FingerGestureDescriptors[(int)EManusFingerName::Middle].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	MetalGesture.FingerGestureDescriptors[(int)EManusFingerName::Middle].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.5f), FFloatRangeBound::Inclusive(1.0f));
	MetalGesture.FingerGestureDescriptors[(int)EManusFingerName::Ring].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	MetalGesture.FingerGestureDescriptors[(int)EManusFingerName::Ring].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.5f), FFloatRangeBound::Inclusive(1.0f));
	MetalGesture.FingerGestureDescriptors[(int)EManusFingerName::Pinky].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(-0.1f), FFloatRangeBound::Inclusive(0.1f));
	MetalGesture.FingerGestureDescriptors[(int)EManusFingerName::Pinky].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(-0.1f), FFloatRangeBound::Inclusive(0.1f));
	HandGestureDescriptors.Add(MetalGesture);

	// Grab hand gesture
	FHandGestureDescriptor GrabGesture;
	GrabGesture.GestureName = "Grab";
	GrabGesture.FingerGestureDescriptors[(int)EManusFingerName::Thumb].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	GrabGesture.FingerGestureDescriptors[(int)EManusFingerName::Thumb].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	GrabGesture.FingerGestureDescriptors[(int)EManusFingerName::Index].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.25f), FFloatRangeBound::Inclusive(1.0f));
	GrabGesture.FingerGestureDescriptors[(int)EManusFingerName::Index].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.5f), FFloatRangeBound::Inclusive(1.0f));
	GrabGesture.FingerGestureDescriptors[(int)EManusFingerName::Middle].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.25f), FFloatRangeBound::Inclusive(1.0f));
	GrabGesture.FingerGestureDescriptors[(int)EManusFingerName::Middle].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.5f), FFloatRangeBound::Inclusive(1.0f));
	GrabGesture.FingerGestureDescriptors[(int)EManusFingerName::Ring].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.15f), FFloatRangeBound::Inclusive(1.0f));
	GrabGesture.FingerGestureDescriptors[(int)EManusFingerName::Ring].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.4f), FFloatRangeBound::Inclusive(1.0f));
	GrabGesture.FingerGestureDescriptors[(int)EManusFingerName::Pinky].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.15f), FFloatRangeBound::Inclusive(1.0f));
	GrabGesture.FingerGestureDescriptors[(int)EManusFingerName::Pinky].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.4f), FFloatRangeBound::Inclusive(1.0f));
	HandGestureDescriptors.Add(GrabGesture);

	// Pinch hand gesture
	FHandGestureDescriptor PinchGesture;
	PinchGesture.GestureName = "Pinch";
	PinchGesture.FingerGestureDescriptors[(int)EManusFingerName::Thumb].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(0.25f));
	PinchGesture.FingerGestureDescriptors[(int)EManusFingerName::Thumb].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.5f), FFloatRangeBound::Inclusive(1.0f));
	PinchGesture.FingerGestureDescriptors[(int)EManusFingerName::Index].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.15f), FFloatRangeBound::Inclusive(0.4f));
	PinchGesture.FingerGestureDescriptors[(int)EManusFingerName::Index].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.3f), FFloatRangeBound::Inclusive(0.6f));
	PinchGesture.FingerGestureDescriptors[(int)EManusFingerName::Middle].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	PinchGesture.FingerGestureDescriptors[(int)EManusFingerName::Middle].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	PinchGesture.FingerGestureDescriptors[(int)EManusFingerName::Ring].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	PinchGesture.FingerGestureDescriptors[(int)EManusFingerName::Ring].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	PinchGesture.FingerGestureDescriptors[(int)EManusFingerName::Pinky].FlexSensorRanges[0] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	PinchGesture.FingerGestureDescriptors[(int)EManusFingerName::Pinky].FlexSensorRanges[1] = FFloatRange(FFloatRangeBound::Inclusive(0.0f), FFloatRangeBound::Inclusive(1.0f));
	HandGestureDescriptors.Add(PinchGesture);

	// Replication
	DefaultReplicationOffsetTime = 0.1f;
	bUpdateReplicationOffsetTimeUsingPing = true;
	ReplicationOffsetTimePingMultiplier = 1.5f;
	ReplicationOffsetTimePingMultiplier = 0.1f;

	// Tracking
	TrackingSmoothing = 0.025f;
	HandTrackingMethod = EManusHandTrackingMethod::Unreal;
	TrackingManagerDeviceUpdateFrequency = 2.5f;
}

#if WITH_EDITOR
void UManusSettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
	FProperty* HeadProperty = PropertyChangedEvent.PropertyChain.GetHead()->GetValue();
#else
	UProperty* HeadProperty = PropertyChangedEvent.PropertyChain.GetHead()->GetValue();
#endif
	if (HeadProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UManusSettings, HandGestureDescriptors))
	{
		int HandGestureIndex = PropertyChangedEvent.GetArrayIndex(TEXT("HandGestureDescriptors"));
		if (HandGestureDescriptors.IsValidIndex(HandGestureIndex))
		{
			int FingerGestureIndex = PropertyChangedEvent.GetArrayIndex(TEXT("FingerGestureDescriptors"));
			if (FingerGestureIndex != INDEX_NONE)
			{
				TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
				if (ManusLocalLiveLinkSource.IsValid())
				{
					ManusLocalLiveLinkSource->PreviewGesture(HandGestureIndex);
				}
			}
		}
	}
}
#endif //WITH_EDITOR
