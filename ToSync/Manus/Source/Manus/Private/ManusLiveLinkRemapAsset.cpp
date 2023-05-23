// Copyright Epic Games, Inc. All Rights Reserved.

#include "ManusLiveLinkRemapAsset.h"
#include "Manus.h"
#include "ManusSkeleton.h"
#include "ManusBlueprintLibrary.h"
#include "AnimNode_ManusLiveLinkPose.h"
#include "BonePose.h"
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
#include "Roles/LiveLinkAnimationTypes.h"
#else
#include "LiveLinkTypes.h"
#endif

DECLARE_CYCLE_STAT(TEXT("Manus Build Pose From Animation Data"), STAT_Manus_BuildPoseFromAnimationData, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("Manus Build Hand Pose From Animation Data"), STAT_Manus_BuildHandPoseFromAnimationData, STATGROUP_Manus);

#define LOCTEXT_NAMESPACE "FManusModule"

UManusLiveLinkRemapAsset::UManusLiveLinkRemapAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
void UManusLiveLinkRemapAsset::BuildPoseFromAnimationData(float DeltaTime, const FLiveLinkSkeletonStaticData* InSkeletonData, const FLiveLinkAnimationFrameData* InFrameData, FCompactPose& OutPose)
#else
void UManusLiveLinkRemapAsset::BuildPoseForSubject(float DeltaTime, const FLiveLinkSubjectFrame& InFrame, FCompactPose& OutPose, FBlendedCurve& OutCurve)
#endif
{
	SCOPE_CYCLE_COUNTER(STAT_Manus_BuildPoseFromAnimationData);

	if (bShouldAnimate)
	{
		// Hand bones
		if (MotionCaptureType == EManusMotionCaptureType::LeftHand || MotionCaptureType == EManusMotionCaptureType::BothHands || MotionCaptureType == EManusMotionCaptureType::FullBody)
		{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
			BuildHandPoseFromAnimationData(EManusHandType::Left, InSkeletonData, InFrameData, OutPose);
#else
			BuildHandPoseForSubject(EManusHandType::Left, InFrame, OutPose, OutCurve);
#endif
		}
		if (MotionCaptureType == EManusMotionCaptureType::RightHand || MotionCaptureType == EManusMotionCaptureType::BothHands || MotionCaptureType == EManusMotionCaptureType::FullBody)
		{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
			BuildHandPoseFromAnimationData(EManusHandType::Right, InSkeletonData, InFrameData, OutPose);
#else
			BuildHandPoseForSubject(EManusHandType::Right, InFrame, OutPose, OutCurve);
#endif
		}

		// Polygon body bones
		if (MotionCaptureType == EManusMotionCaptureType::FullBody)
		{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
			const TArray<FName>& SourceBoneNames = InSkeletonData->GetBoneNames();
#else
			const TArray<FName>& SourceBoneNames = InFrame.RefSkeleton.GetBoneNames();
#endif

			FCSPose<FCompactPose> MeshPoses;
			MeshPoses.InitPose(OutPose);

			int FirstBoneIndex = (int)EManusBoneName::Root;
			int LastBoneIndex = (int)EManusBoneName::RightHand;

			for (int BoneIndex = FirstBoneIndex; BoneIndex <= LastBoneIndex; BoneIndex++)
			{
				// Manus bone info
				FName BoneName = SourceBoneNames[BoneIndex];
				FName RemappedBoneName = GET_BONE_NAME(ManusLiveLinkNode.BoneMap[(int)ManusLiveLinkNode.BoneNameToEnum[BoneName]]);
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
				FTransform BoneTransform = InFrameData->Transforms[BoneIndex];
#else
				FTransform BoneTransform = InFrame.Transforms[BoneIndex];
#endif

				// We can ignore the hand tracker bones
				if (BoneIndex != (int)EManusBoneName::LeftHandTracker && BoneIndex != (int)EManusBoneName::RightHandTracker)
				{
					int32 MeshIndex = OutPose.GetBoneContainer().GetPoseBoneIndexForBoneName(RemappedBoneName);
					if (MeshIndex != INDEX_NONE)
					{
						FCompactPoseBoneIndex CPIndex = OutPose.GetBoneContainer().MakeCompactPoseIndex(FMeshPoseBoneIndex(MeshIndex));
						if (CPIndex != INDEX_NONE)
						{
							// Do not touch the bone if there was no valid data from Manus
							if (!BoneTransform.GetScale3D().IsZero())
							{
								MeshPoses.SetComponentSpaceTransform(CPIndex, BoneTransform);
							}
						}
					}
				}
			}
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 26
			FCSPose<FCompactPose>::ConvertComponentPosesToLocalPosesSafe(MeshPoses, OutPose);
#else
			ConvertComponentPosesToLocalPoses(MeshPoses, OutPose);
#endif
			// Convert root to local pose
			FCompactPoseBoneIndex CPIndex = OutPose.GetBoneContainer().MakeCompactPoseIndex(FMeshPoseBoneIndex(0));
			if (CPIndex != INDEX_NONE)
			{
				OutPose[CPIndex].SetToRelativeTransform(SkeletalMeshActorSpaceTransform);
			}
		}
	}
}

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
void UManusLiveLinkRemapAsset::BuildHandPoseFromAnimationData(EManusHandType HandType, const FLiveLinkSkeletonStaticData* InSkeletonData, const FLiveLinkAnimationFrameData* InFrameData, FCompactPose& OutPose)
#else
void UManusLiveLinkRemapAsset::BuildHandPoseForSubject(EManusHandType HandType, const FLiveLinkSubjectFrame& InFrame, FCompactPose& OutPose, FBlendedCurve& OutCurve)
#endif
{
	SCOPE_CYCLE_COUNTER(STAT_Manus_BuildHandPoseFromAnimationData);

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
	const TArray<FName>& SourceBoneNames = InSkeletonData->GetBoneNames();
#else
	const TArray<FName>& SourceBoneNames = InFrame.RefSkeleton.GetBoneNames();
#endif

	int FirstFingerBoneIndex = 0, LastFingerBoneIndex = 0;
	switch (HandType)
	{
	case EManusHandType::Left:
		FirstFingerBoneIndex = (int)EManusBoneName::LeftHandThumb1;
		LastFingerBoneIndex = (int)EManusBoneName::LeftHandPinky3;
		break;
	case EManusHandType::Right:
		FirstFingerBoneIndex = (int)EManusBoneName::RightHandThumb1;
		LastFingerBoneIndex = (int)EManusBoneName::RightHandPinky3;
		break;
	}

	// Finger bones
	for (int32 BoneIndex = FirstFingerBoneIndex; BoneIndex <= LastFingerBoneIndex; ++BoneIndex)
	{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
		FTransform BoneTransform = InFrameData->Transforms[BoneIndex];
#else
		FTransform BoneTransform = InFrame.Transforms[BoneIndex];
#endif

		// Do not touch the bone if there was no valid data from Manus
		if (!BoneTransform.GetScale3D().IsZero())
		{
			FName RemappedBoneName = GET_BONE_NAME(ManusLiveLinkNode.BoneMap[(int)ManusLiveLinkNode.BoneNameToEnum[SourceBoneNames[BoneIndex]]]);
			int32 MeshIndex = OutPose.GetBoneContainer().GetPoseBoneIndexForBoneName(RemappedBoneName);
			if (MeshIndex != INDEX_NONE)
			{
				FCompactPoseBoneIndex CPIndex = OutPose.GetBoneContainer().MakeCompactPoseIndex(FMeshPoseBoneIndex(MeshIndex));
				if (CPIndex != INDEX_NONE)
				{
					OutPose[CPIndex].SetRotation(OutPose.GetRefPose(CPIndex).GetRotation() * BoneTransform.GetRotation());
				}
			}
		}
	}

	// Hand bones
	// In Full Body tracking, the hand position is taken care by Polygon
	if (MotionCaptureType != EManusMotionCaptureType::FullBody)
	{
		// Get the hand bone
		int HandBoneIndex = (HandType == EManusHandType::Left ? (int)EManusBoneName::LeftHand : (int)EManusBoneName::RightHand);
		FName RemappedBoneName = GET_BONE_NAME(ManusLiveLinkNode.BoneMap[(int)ManusLiveLinkNode.BoneNameToEnum[SourceBoneNames[HandBoneIndex]]]);
		int32 MeshIndex = OutPose.GetBoneContainer().GetPoseBoneIndexForBoneName(RemappedBoneName);
		if (MeshIndex != INDEX_NONE)
		{
			FCompactPoseBoneIndex CPIndex = OutPose.GetBoneContainer().MakeCompactPoseIndex(FMeshPoseBoneIndex(MeshIndex));
			if (CPIndex != INDEX_NONE)
			{
				// Use the hand tracker data or the glove IMU data
				int HandTrackerBoneIndex = (HandType == EManusHandType::Left ? (int)EManusBoneName::LeftHandTracker : (int)EManusBoneName::RightHandTracker);
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
				if (!InFrameData->Transforms[HandTrackerBoneIndex].GetScale3D().IsZero())
#else
				if (!InFrame.Transforms[HandTrackerBoneIndex].GetScale3D().IsZero())
#endif
				{
					// Use the hand tracker data
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
					FTransform BoneTransform = InFrameData->Transforms[HandTrackerBoneIndex];
#else
					FTransform BoneTransform = InFrame.Transforms[HandTrackerBoneIndex];
#endif

					// Offset hand bone according to extra tracking device delta transform
					BoneTransform = ManusLiveLinkNode.TrackingDeviceDeltaTransform[(int)HandType] * BoneTransform;

					// Hand bone
					FVector HandLocation = BoneTransform.GetLocation();
					FQuat HandRotation = BoneTransform.GetRotation();

					// Hand mirroring
					if (bMirrorHandBone)
					{
						// Compensate Skeletal Mesh component negative scale for the Hand location given by the Tracker
						HandLocation *= FVector(1.0f, -1.0f, 1.0f);
						FRotator HandRotator = HandRotation.Rotator();
						HandRotator.Yaw = -HandRotator.Yaw;
						HandRotator.Roll = -HandRotator.Roll + 180.0f;
						HandRotation = HandRotator.Quaternion();
					}

					// Compensate Skeletal Mesh component scale for the Hand location given by the Tracker
					HandLocation /= SkeletalMeshScale;

					OutPose[CPIndex].SetLocation(HandLocation);
					OutPose[CPIndex].SetRotation(HandRotation);

					// Convert to local space
					TArray<FTransform> ParentTransforms;
					FCompactPoseBoneIndex ParentIndex = OutPose.GetParentBoneIndex(CPIndex);
					while (ParentIndex != INDEX_NONE)
					{
						ParentTransforms.Add(OutPose[ParentIndex]);
						ParentIndex = OutPose.GetParentBoneIndex(ParentIndex);
					}
					ParentTransforms.Add(SkeletalMeshActorSpaceTransform);
					for (int ParentTransformIndex = ParentTransforms.Num() - 1; ParentTransformIndex >= 0; ParentTransformIndex--)
					{
						OutPose[CPIndex].SetToRelativeTransform(ParentTransforms[ParentTransformIndex]);
					}
				}
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
				else if (!InFrameData->Transforms[HandBoneIndex].GetScale3D().IsZero())
#else
				else if (!InFrame.Transforms[HandBoneIndex].GetScale3D().IsZero())
#endif
				{
					// Use the hand data, which come from the glove IMU
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
					FTransform BoneTransform = InFrameData->Transforms[HandBoneIndex];
#else
					FTransform BoneTransform = InFrame.Transforms[HandBoneIndex];
#endif
					BoneTransform = ManusLiveLinkNode.TrackingDeviceDeltaTransform[(int)HandType] * BoneTransform;

					if (bMirrorHandBone)
					{
						// Compensate Skeletal Mesh component negative scale for the Hand location given by the Tracker
						FRotator HandRotator = BoneTransform.Rotator();
						HandRotator.Yaw = -HandRotator.Yaw;
						HandRotator.Roll = -HandRotator.Roll + 180.0f;
						BoneTransform.SetRotation( HandRotator.Quaternion());
					}

					OutPose[CPIndex].SetRotation( BoneTransform.GetRotation());

					// Convert to local space
					TArray<FTransform> ParentTransforms;
					FCompactPoseBoneIndex ParentIndex = OutPose.GetParentBoneIndex(CPIndex);
					while (ParentIndex != INDEX_NONE)
					{
						ParentTransforms.Add(OutPose[ParentIndex]);
						ParentIndex = OutPose.GetParentBoneIndex(ParentIndex);
					}
					ParentTransforms.Add(SkeletalMeshActorSpaceTransform);
					for (int ParentTransformIndex = ParentTransforms.Num() - 1; ParentTransformIndex >= 0; ParentTransformIndex--)
					{
						OutPose[CPIndex].SetToRelativeTransform(ParentTransforms[ParentTransformIndex]);
					}

					OutPose[CPIndex].SetLocation(OutPose.GetRefPose(CPIndex).GetTranslation());
				}
			}
		}
	}
}

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION <= 25
void UManusLiveLinkRemapAsset::ConvertComponentPosesToLocalPoses(FCSPose<FCompactPose>& InPose, FCompactPose& OutPose)
{
	checkSlow(InPose.Pose.IsValid());

	const int32 NumBones = InPose.GetPose().GetNumBones();

	// now we need to convert back to local bases
	// only convert back that has been converted to mesh base
	// if it was local base, and if it hasn't been modified
	// that is still okay even if parent is changed, 
	// that doesn't mean this local has to change
	// go from child to parent since I need parent inverse to go back to local
	// root is same, so no need to do Index == 0
	const FCompactPose::BoneIndexType RootBoneIndex(0);
	if (InPose.GetComponentSpaceFlags()[RootBoneIndex])
	{
		OutPose[RootBoneIndex] = InPose.GetPose()[RootBoneIndex];
	}


	for (int32 Index = NumBones - 1; Index > 0; Index--)
	{
		const FCompactPose::BoneIndexType BoneIndex(Index);
		if (InPose.GetComponentSpaceFlags()[BoneIndex])
		{
			const FCompactPose::BoneIndexType ParentIndex = OutPose.GetParentBoneIndex(BoneIndex);

			// if parent is local space, we have to calculate parent
			if (!InPose.GetComponentSpaceFlags()[ParentIndex])
			{
				// if I'm calculated, but not parent, update parent
				InPose.CalculateComponentSpaceTransform(ParentIndex);
			}

			OutPose[BoneIndex] = InPose.GetPose()[BoneIndex];
			OutPose[BoneIndex].SetToRelativeTransform(InPose.GetPose()[ParentIndex]);
			OutPose[BoneIndex].NormalizeRotation();
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE
