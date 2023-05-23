// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "SeperateActorManusComponent.h"
#include "ManusComponent.h"
#include "Manus.h"
#include "ManusBlueprintTypes.h"
#include "ManusBlueprintLibrary.h"
#include "ManusReplicator.h"
#include "ManusSettings.h"
#include "ManusSkeleton.h"
#include "CoreSdk.h"

#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/CollisionProfile.h"
#include "Net/UnrealNetwork.h"
#include "Engine/NetConnection.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"


USeperateActorManusComponent::USeperateActorManusComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//
}





void USeperateActorManusComponent::TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// Skip the Manus Tick Component
	Super::Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);

	// Init Manus Replicator ID
	InitManusReplicatorID();

	// Only when locally controlled
	if (IsLocalPCControlled())
	{
		// Finger Haptics
		if (bFingerHaptics)
		{
			TickFingerHaptics();
		}

		// Gesture detection
		TickGestureDetection();
	}

}


bool USeperateActorManusComponent::IsLocalPCControlled() const
{
	if (GEngine && GEngine->GetWorldContextFromWorld(GetWorld()))
	{
		APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
		//UE_LOG(LogManus, Warning, TEXT("SeperateActorManus component: \"Class works\" was set to TRUE to support the Manus Glove finger haptics."));
		//bool local = (LocalPlayerController && GetOwner() == LocalPlayerController->GetPawn());
		//bool bol = GetAttachmentRootActor() == LocalPlayerController->GetPawn();
		//UE_LOG(LogTemp, Warning, TEXT("ComponentWorks"));
		return (LocalPlayerController && (GetOwner() == LocalPlayerController->GetPawn()||GetAttachmentRootActor()==LocalPlayerController->GetPawn()));
	}
	return true;
}




void USeperateActorManusComponent::TickFingerHaptics()
{
	// Update vibrate powers according to current overlaps (used when the body is set to Overlap)
	const TArray<FOverlapInfo>& OverlapInfos = GetOverlapInfos();
	for (const FOverlapInfo& OverlapInfo : OverlapInfos)
	{
		// Don't vibrate when the overlapping component is attached to the Manus component
		if (!OverlapInfo.OverlapInfo.Component->IsAttachedTo(this))
		{
			// Retrieve the bone name from the body index
			if (Bodies.IsValidIndex(OverlapInfo.OverlapInfo.Item))
			{
				AddFingerVibration(Bodies[OverlapInfo.OverlapInfo.Item]->BodySetup->BoneName);
			}
		}
	}

	if (FManusModule::Get().IsActive())
	{
		// Vibrate
		if (MotionCaptureType == EManusMotionCaptureType::LeftHand
			|| MotionCaptureType == EManusMotionCaptureType::BothHands
			|| MotionCaptureType == EManusMotionCaptureType::FullBody
			) {
			UManusBlueprintLibrary::VibrateFingers(
				EManusHandType::Left,
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Thumb],
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Index],
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Middle],
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Ring],
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Pinky]);
		}
		if (MotionCaptureType == EManusMotionCaptureType::RightHand
			|| MotionCaptureType == EManusMotionCaptureType::BothHands
			|| MotionCaptureType == EManusMotionCaptureType::FullBody
			) {
			UManusBlueprintLibrary::VibrateFingers(
				EManusHandType::Right,
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Thumb],
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Index],
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Middle],
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Ring],
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Pinky]);
		}
	}

	// Reset vibrate powers
	for (int i = 0; i < (int)EManusFingerName::Max; i++)
	{
		LeftHandFingersCollisionVibratePowers[i] = 0.0f;
		RightHandFingersCollisionVibratePowers[i] = 0.0f;
	}
}

void USeperateActorManusComponent::TickGestureDetection()
{
	int ManusLiveLinkUserIndex = FManusModule::Get().GetManusLiveLinkUserIndex(ManusDashboardUserIndex, ManusSkeleton);
	if (ManusLiveLinkUserIndex != INDEX_NONE)
	{
		FManusLiveLinkUser& ManusLiveLinkUser = FManusModule::Get().GetManusLiveLinkUser(ManusLiveLinkUserIndex);

		const TArray<FHandGestureDescriptor>& HandGestureDescriptors = GetDefault<UManusSettings>()->HandGestureDescriptors;
		for (int HandType = 0; HandType < (int)EManusHandType::Max; HandType++)
		{
			if (MotionCaptureType == EManusMotionCaptureType::BothHands
				|| MotionCaptureType == EManusMotionCaptureType::FullBody
				|| (MotionCaptureType == EManusMotionCaptureType::LeftHand && (EManusHandType)HandType == EManusHandType::Left)
				|| (MotionCaptureType == EManusMotionCaptureType::RightHand && (EManusHandType)HandType == EManusHandType::Right)
				) {
				ManusComponentHandLiveData[HandType].IsGestureOnGoing.SetNum(HandGestureDescriptors.Num());

				const TArray<float>& DetectedHandGestureTimers = ManusLiveLinkUser.HandGestureDetectionData[HandType].ManusLiveLinkUserDetectedHandGestureTimers;
				for (int GestureIndex = 0; GestureIndex < HandGestureDescriptors.Num() && GestureIndex < DetectedHandGestureTimers.Num(); GestureIndex++)
				{
					if (DetectedHandGestureTimers[GestureIndex] > 0.0f)
					{
						if (!ManusComponentHandLiveData[HandType].IsGestureOnGoing[GestureIndex])
						{
							// Gesture started delegate
							OnGestureStarted.Broadcast((EManusHandType)HandType, HandGestureDescriptors[GestureIndex].GestureName);
						}

						// Gesture on-going delegate
						OnGestureOnGoing.Broadcast((EManusHandType)HandType, HandGestureDescriptors[GestureIndex].GestureName, DetectedHandGestureTimers[GestureIndex]);

						// Update on-going gesture status
						ManusComponentHandLiveData[HandType].IsGestureOnGoing[GestureIndex] = true;
					}
					else if (ManusComponentHandLiveData[HandType].IsGestureOnGoing[GestureIndex])
					{
						// Update on-going gesture status
						ManusComponentHandLiveData[HandType].IsGestureOnGoing[GestureIndex] = false;

						// Gesture finished delegate
						OnGestureFinished.Broadcast((EManusHandType)HandType, HandGestureDescriptors[GestureIndex].GestureName);
					}
				}
			}
		}
	}

	//
}

void USeperateActorManusComponent::AddFingerVibration(FName BoneName)
{
	if (ManusSkeleton)
	{
		if (MotionCaptureType == EManusMotionCaptureType::LeftHand
			|| MotionCaptureType == EManusMotionCaptureType::BothHands
			|| MotionCaptureType == EManusMotionCaptureType::FullBody
			) {
			if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandThumb1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandThumb2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandThumb3]))
			{
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Thumb] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandIndex1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandIndex2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandIndex3]))
			{
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Index] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandMiddle1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandMiddle2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandMiddle3]))
			{
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Middle] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandRing1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandRing2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandRing3]))
			{
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Ring] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandPinky1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandPinky2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::LeftHandPinky3]))
			{
				LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Pinky] += 1.0f / (int)EManusPhalangeName::Max;
			}
		}
		if (MotionCaptureType == EManusMotionCaptureType::RightHand
			|| MotionCaptureType == EManusMotionCaptureType::BothHands
			|| MotionCaptureType == EManusMotionCaptureType::FullBody
			) {
			if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandThumb1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandThumb2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandThumb3]))
			{
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Thumb] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandIndex1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandIndex2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandIndex3]))
			{
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Index] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandMiddle1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandMiddle2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandMiddle3]))
			{
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Middle] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandRing1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandRing2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandRing3]))
			{
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Ring] += 1.0f / (int)EManusPhalangeName::Max;
			}
			else if (BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandPinky1]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandPinky2]) || BoneName == GET_BONE_NAME(ManusSkeleton->BoneMap[(int)EManusBoneName::RightHandPinky3]))
			{
				RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Pinky] += 1.0f / (int)EManusPhalangeName::Max;
			}
		}
	}
}
