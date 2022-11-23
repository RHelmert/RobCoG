// Copyright 2015-2020 Manus

#include "ManusTools.h"
#include "Manus.h"
#include "ManusSettings.h"
#include "ManusConvert.h"

#include "AnimationRuntime.h"
#include "Animation/Skeleton.h"


int64 ManusTools::GenerateManusIdFromManusLiveLinkUser(int ManusLiveLinkUserIndex)
{
	const TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	if (ManusLiveLinkUsers.IsValidIndex(ManusLiveLinkUserIndex))
	{
		UManusSkeleton* ManusSkeleton = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusSkeleton;
		if (ManusSkeleton && ManusSkeleton->GetSkeleton())
		{
			return ManusLiveLinkUserIndex + ManusSkeleton->GetSkeleton()->GetGuid().A;
		}
	}
	return 0;
}


#if WITH_EDITORONLY_DATA

struct FAutomaticBoneNameMappingBoneInfo
{
	int Index = INDEX_NONE;
	int Parent = INDEX_NONE;
	int ChildNum = 0;
	int Depth = 0;
	float Distance = 0.0f;
	int ChildMaxDepth = 0;
	float ChildMaxDistance = 0.0f;
	TArray<int> Children;
	FName Name;
};

struct FAutomaticBoneNameMappingBoneGroup
{
	TArray<int> Bones;

	bool operator<(const FAutomaticBoneNameMappingBoneGroup& Other) const
	{
		return Bones.Num() > Other.Bones.Num();
	}
};

void InnerAutomaticBoneNameMappingRecursion(USkeleton* Skeleton, int ParentBoneIndex, TArray<FAutomaticBoneNameMappingBoneInfo>& SkeletonBoneInfo)
{
	const TArray<FTransform> LocalPoses = Skeleton->GetRefLocalPoses();

	// Index
	SkeletonBoneInfo[ParentBoneIndex].Index = ParentBoneIndex;

	// Children info
	SkeletonBoneInfo[ParentBoneIndex].ChildNum = Skeleton->GetChildBones(ParentBoneIndex, SkeletonBoneInfo[ParentBoneIndex].Children);

	// Name
	SkeletonBoneInfo[ParentBoneIndex].Name = Skeleton->GetReferenceSkeleton().GetBoneName(ParentBoneIndex);

	// Children
	for (int ChildIndex : SkeletonBoneInfo[ParentBoneIndex].Children)
	{
		// Parent info
		SkeletonBoneInfo[ChildIndex].Parent = ParentBoneIndex;

		// Depth info
		SkeletonBoneInfo[ChildIndex].Depth = SkeletonBoneInfo[ParentBoneIndex].Depth + 1;

		// Distance info
		SkeletonBoneInfo[ChildIndex].Distance = SkeletonBoneInfo[ParentBoneIndex].Distance + LocalPoses[ChildIndex].GetLocation().Size();

		// Update parents Child Max Depth
		int Parent = ParentBoneIndex;
		int Depth = 0;
		float Distance = 0.0f;
		while (Parent != INDEX_NONE)
		{
			Depth++;
			Distance += LocalPoses[Parent].GetLocation().Size();
			if (Depth > SkeletonBoneInfo[Parent].ChildMaxDepth)
			{
				SkeletonBoneInfo[Parent].ChildMaxDepth = Depth;
			}
			if (Distance > SkeletonBoneInfo[Parent].ChildMaxDistance)
			{
				SkeletonBoneInfo[Parent].ChildMaxDistance = Distance;
			}
			Parent = SkeletonBoneInfo[Parent].Parent;
		}

		// Get info of children
		InnerAutomaticBoneNameMappingRecursion(Skeleton, ChildIndex, SkeletonBoneInfo);
	}
}

void ManusTools::AutomaticSkeletonBoneNameMapping(USkeleton* Skeleton, BoneName_t* BoneMap)
{
	if (!Skeleton || !BoneMap)
	{
		return;
	}

	// This is the array we want to fill
	int BoneIndexMap[(int)EManusBoneName::Max];
	for (int i = 0; i < (int)EManusBoneName::Max; i++)
	{
		BoneIndexMap[i] = INDEX_NONE;
	}

	// Get skeleton bone info
	TArray<FAutomaticBoneNameMappingBoneInfo> SkeletonBoneInfo;
	const TArray<FTransform> LocalPoses = Skeleton->GetRefLocalPoses();
	SkeletonBoneInfo.SetNum(Skeleton->GetReferenceSkeleton().GetNum());
	InnerAutomaticBoneNameMappingRecursion(Skeleton, 0, SkeletonBoneInfo);

	// Skeleton component space transforms
	TArray<FTransform> ComponentSpaceTransforms;
	FAnimationRuntime::FillUpComponentSpaceTransforms(Skeleton->GetReferenceSkeleton(), Skeleton->GetReferenceSkeleton().GetRefBonePose(), ComponentSpaceTransforms);

	// Axis
	FVector FrontAxis = FVector::ZeroVector;
	FVector LeftAxis = FVector::ZeroVector;

	// Find the Root bone
	BoneIndexMap[0] = 0;

	// Find the Hips bone
	for (FAutomaticBoneNameMappingBoneInfo BoneInfo : SkeletonBoneInfo)
	{
		// The Hips bone should be the closest to the root with at least two children that have at least 3 more children
		if (BoneInfo.ChildNum >= 2)
		{
			int PotentialLegCount = 0;
			for (int i = 0; i < BoneInfo.Children.Num(); i++)
			{
				if (SkeletonBoneInfo[BoneInfo.Children[i]].ChildMaxDepth >= 3)
				{
					PotentialLegCount++;
				}
			}
			if (PotentialLegCount >= 2)
			{
				if (BoneIndexMap[(int)EManusBoneName::Hips] == INDEX_NONE || SkeletonBoneInfo[BoneInfo.Index].Depth < SkeletonBoneInfo[BoneIndexMap[(int)EManusBoneName::Hips]].Depth)
				{
					BoneIndexMap[(int)EManusBoneName::Hips] = BoneInfo.Index;
				}
			}
		}
	}

	// Find the legs bones
	if (BoneIndexMap[(int)EManusBoneName::Hips] != INDEX_NONE)
	{
		FAutomaticBoneNameMappingBoneInfo& HipBoneInfo = SkeletonBoneInfo[BoneIndexMap[(int)EManusBoneName::Hips]];
		TArray<FAutomaticBoneNameMappingBoneGroup> PotentialLegBones;
		for (int UpperLegBoneIndex = 0; UpperLegBoneIndex < HipBoneInfo.Children.Num(); UpperLegBoneIndex++)
		{
			if (SkeletonBoneInfo[HipBoneInfo.Children[UpperLegBoneIndex]].ChildMaxDepth >= 3)
			{
				int UpperLegBone = HipBoneInfo.Children[UpperLegBoneIndex];
				FAutomaticBoneNameMappingBoneInfo& UpperLegBoneInfo = SkeletonBoneInfo[UpperLegBone];
				int LegIndex = PotentialLegBones.AddDefaulted();
				PotentialLegBones[LegIndex].Bones.Add(UpperLegBone);

				for (int LowerLegBoneIndex = 0; LowerLegBoneIndex < UpperLegBoneInfo.Children.Num(); LowerLegBoneIndex++)
				{
					if (SkeletonBoneInfo[UpperLegBoneInfo.Children[LowerLegBoneIndex]].ChildMaxDepth >= 2
						&& ComponentSpaceTransforms[UpperLegBoneInfo.Children[LowerLegBoneIndex]].GetLocation().Z < ComponentSpaceTransforms[UpperLegBone].GetLocation().Z
						) {
						int LowerLegBone = UpperLegBoneInfo.Children[LowerLegBoneIndex];
						FAutomaticBoneNameMappingBoneInfo& LowerLegBoneInfo = SkeletonBoneInfo[LowerLegBone];
						PotentialLegBones[LegIndex].Bones.Add(LowerLegBone);

						for (int FootBoneIndex = 0; FootBoneIndex < LowerLegBoneInfo.Children.Num(); FootBoneIndex++)
						{
							if (SkeletonBoneInfo[LowerLegBoneInfo.Children[FootBoneIndex]].ChildMaxDepth >= 1
								&& ComponentSpaceTransforms[LowerLegBoneInfo.Children[FootBoneIndex]].GetLocation().Z < ComponentSpaceTransforms[LowerLegBone].GetLocation().Z
								) {
								int FootBone = LowerLegBoneInfo.Children[FootBoneIndex];
								FAutomaticBoneNameMappingBoneInfo& FootBoneInfo = SkeletonBoneInfo[FootBone];
								PotentialLegBones[LegIndex].Bones.Add(FootBone);
								if (FootBoneInfo.ChildNum > 0)
								{
									// Prefer the bone that might have toes (it can help to avoid taking heel bone for the toe bone)
									int ToeBoneIndex = 0;
									for (int PotentialToeBoneIndex = 1; PotentialToeBoneIndex < FootBoneInfo.ChildNum; PotentialToeBoneIndex++)
									{
										if (SkeletonBoneInfo[FootBoneInfo.Children[PotentialToeBoneIndex]].ChildMaxDepth > SkeletonBoneInfo[FootBoneInfo.Children[ToeBoneIndex]].ChildMaxDepth)
										{
											ToeBoneIndex = PotentialToeBoneIndex;
										}
									}

									PotentialLegBones[LegIndex].Bones.Add(FootBoneInfo.Children[ToeBoneIndex]);
									if (SkeletonBoneInfo[FootBoneInfo.Children[ToeBoneIndex]].ChildNum > 0)
									{
										PotentialLegBones[LegIndex].Bones.Add(SkeletonBoneInfo[FootBoneInfo.Children[ToeBoneIndex]].Children[0]);
									}
								}
								break;
							}
						}

						break;
					}
				}
			}
		}

		// Choose the two legs that matched the best
		if (PotentialLegBones.Num() > 2)
		{
			PotentialLegBones.Sort();
			PotentialLegBones.SetNum(2);
		}

		// Figure out the axes
		if (PotentialLegBones.Num() == 2)
		{
			// Lateral axis (without sign, 50/50 to be right)
			FVector UpperLeg0SpaceLocation = ComponentSpaceTransforms[PotentialLegBones[0].Bones[0]].GetLocation();
			FVector UpperLeg1SpaceLocation = ComponentSpaceTransforms[PotentialLegBones[1].Bones[0]].GetLocation();
			float XDiff = 0.0f;
			if (FMath::Sign(UpperLeg0SpaceLocation.X) != FMath::Sign(UpperLeg1SpaceLocation.X))
			{
				XDiff = FMath::Abs(UpperLeg0SpaceLocation.X - UpperLeg1SpaceLocation.X);
			}
			float YDiff = 0.0f;
			if (FMath::Sign(UpperLeg0SpaceLocation.Y) != FMath::Sign(UpperLeg1SpaceLocation.Y))
			{
				YDiff = FMath::Abs(UpperLeg0SpaceLocation.Y - UpperLeg1SpaceLocation.Y);
			}
			if (XDiff > YDiff)
			{
				LeftAxis = FVector(1, 0, 0);
				FrontAxis = FVector(0, 1, 0);
			}
			else
			{
				LeftAxis = FVector(0, 1, 0);
				FrontAxis = FVector(-1, 0, 0);
			}

			// Figure out the axis signs using the toes, which are pointing forward
			if (PotentialLegBones[0].Bones.IsValidIndex(3))
			{
				FVector FootSpaceLocation = ComponentSpaceTransforms[PotentialLegBones[0].Bones[2]].GetLocation();
				FVector ToesSpaceLocation = ComponentSpaceTransforms[PotentialLegBones[0].Bones[3]].GetLocation();
				if (LeftAxis.X > 0.0f)
				{
					if (ToesSpaceLocation.Y > FootSpaceLocation.Y)
					{
						FrontAxis = FVector(0, 1, 0);
						LeftAxis = FVector(1, 0, 0);
					}
					else
					{
						FrontAxis = FVector(0, -1, 0);
						LeftAxis = FVector(-1, 0, 0);
					}
				}
				else
				{
					if (ToesSpaceLocation.X > FootSpaceLocation.X)
					{
						FrontAxis = FVector(1, 0, 0);
						LeftAxis = FVector(0, -1, 0);
					}
					else
					{
						FrontAxis = FVector(-1, 0, 0);
						LeftAxis = FVector(0, 1, 0);
					}
				}
			}
		}

		// Fill in bones
		for (FAutomaticBoneNameMappingBoneGroup Group : PotentialLegBones)
		{
			if (Group.Bones.Num() > 0)
			{
				int ManusBoneNameIndex = INDEX_NONE;
				FVector UpperLegSpaceLocation = ComponentSpaceTransforms[Group.Bones[0]].GetLocation();
				if (FMath::Sign(UpperLegSpaceLocation.X) * FMath::Sign(LeftAxis.X) == 1 || FMath::Sign(UpperLegSpaceLocation.Y) * FMath::Sign(LeftAxis.Y) == 1)
				{
					ManusBoneNameIndex = (int)EManusBoneName::LeftUpperLeg;
				}
				else
				{
					ManusBoneNameIndex = (int)EManusBoneName::RightUpperLeg;
				}

				for (int Bone : Group.Bones)
				{
					BoneIndexMap[ManusBoneNameIndex++] = Bone;
				}
			}
		}
	}

	// Find the hands root bone
	int NumFingersToLookForArray[4] = { 5, 4, 3, 2 };
	int NumFingersToLookForIndex = 0;
	TArray<FAutomaticBoneNameMappingBoneGroup> PotentialHandBones;
	do
	{
		for (FAutomaticBoneNameMappingBoneInfo BoneInfo : SkeletonBoneInfo)
		{
			if (BoneInfo.ChildNum >= NumFingersToLookForArray[NumFingersToLookForIndex])
			{
				FAutomaticBoneNameMappingBoneGroup Hand;
				Hand.Bones.Add(BoneInfo.Index);

				// Look for fingers
				TArray<int> Fingers;
				int Thumb = INDEX_NONE;
				for (int ChildIndex : BoneInfo.Children)
				{
					// This should be the first phalange, and it should have at least 2 more bones (2 more phalanges)
					if (SkeletonBoneInfo[ChildIndex].ChildMaxDepth >= 2 && SkeletonBoneInfo[ChildIndex].ChildNum == 1)
					{
						Fingers.Add(ChildIndex);
					}
				}

				// If we found too many fingers, try to eliminate by name
				if (Fingers.Num() > NumFingersToLookForArray[NumFingersToLookForIndex])
				{
					TArray<int> FingersToRemove;
					for (int i = Fingers.Num() - 1; i >= 0; i--)
					{
						if (!SkeletonBoneInfo[Fingers[i]].Name.ToString().Contains(TEXT("Thumb"))
							&& !SkeletonBoneInfo[Fingers[i]].Name.ToString().Contains(TEXT("Index"))
							&& !SkeletonBoneInfo[Fingers[i]].Name.ToString().Contains(TEXT("Middle"))
							&& !SkeletonBoneInfo[Fingers[i]].Name.ToString().Contains(TEXT("Ring"))
							&& !SkeletonBoneInfo[Fingers[i]].Name.ToString().Contains(TEXT("Pinky"))
							) {
							FingersToRemove.Add(i);
						}
					}
					if (Fingers.Num() - FingersToRemove.Num() >= 2)
					{
						// Only remove them if we have at least 5 fingers left
						for (int i = 0; i < FingersToRemove.Num(); i++)
						{
							Fingers.RemoveAt(FingersToRemove[i]);
						}
					}
				}

				// Look for the thumb (shortest finger)
				for (int Finger : Fingers)
				{
					if (Thumb == INDEX_NONE || SkeletonBoneInfo[Finger].ChildMaxDistance < SkeletonBoneInfo[Thumb].ChildMaxDistance)
					{
						Thumb = Finger;
					}
				}

				// Look for the other fingers
				if (Thumb != INDEX_NONE && Fingers.Num() > 2)
				{
					// Figure out which finger is which by ordering the other fingers using their distance to the thumb
					Fingers.Sort([&](const int& A, const int& B)
						{
							return FVector::Dist(LocalPoses[Thumb].GetLocation(), LocalPoses[A].GetLocation()) < FVector::Dist(LocalPoses[Thumb].GetLocation(), LocalPoses[B].GetLocation());
						});
					int Index = Fingers.IsValidIndex(1) ? Fingers[1] : INDEX_NONE;
					int Middle = Fingers.IsValidIndex(2) ? Fingers[2] : INDEX_NONE;
					int Ring = Fingers.IsValidIndex(3) ? Fingers[3] : INDEX_NONE;
					int Pinky = Fingers.IsValidIndex(4) ? Fingers[4] : INDEX_NONE;

					// Add fingers to porential hand (same order as in EManusBoneName)
					Hand.Bones.Add(Thumb);
					Hand.Bones.Add(SkeletonBoneInfo[Thumb].Children[0]);
					Hand.Bones.Add(SkeletonBoneInfo[SkeletonBoneInfo[Thumb].Children[0]].Children[0]);
					if (Index != INDEX_NONE)
					{
						Hand.Bones.Add(Index);
						Hand.Bones.Add(SkeletonBoneInfo[Index].Children[0]);
						Hand.Bones.Add(SkeletonBoneInfo[SkeletonBoneInfo[Index].Children[0]].Children[0]);
					}
					if (Middle != INDEX_NONE)
					{
						Hand.Bones.Add(Middle);
						Hand.Bones.Add(SkeletonBoneInfo[Middle].Children[0]);
						Hand.Bones.Add(SkeletonBoneInfo[SkeletonBoneInfo[Middle].Children[0]].Children[0]);
					}
					if (Ring != INDEX_NONE)
					{
						Hand.Bones.Add(Ring);
						Hand.Bones.Add(SkeletonBoneInfo[Ring].Children[0]);
						Hand.Bones.Add(SkeletonBoneInfo[SkeletonBoneInfo[Ring].Children[0]].Children[0]);
					}
					if (Pinky != INDEX_NONE)
					{
						Hand.Bones.Add(Pinky);
						Hand.Bones.Add(SkeletonBoneInfo[Pinky].Children[0]);
						Hand.Bones.Add(SkeletonBoneInfo[SkeletonBoneInfo[Pinky].Children[0]].Children[0]);
					}

					// Add to potential hands
					PotentialHandBones.Add(Hand);
				}
			}
		}

		// Try to find a different number of fingers
		NumFingersToLookForIndex++;

	} while (PotentialHandBones.Num() == 0 && NumFingersToLookForIndex < 4);

	// Set hands bone names in the map
	if (PotentialHandBones.Num() >= 2)
	{
		// If there are more than 2 potential hands, let's try to reduce the number of hands by looking at the bones name
		TArray<int> PotentialHandsToRemove;
		for (int i = PotentialHandBones.Num() - 1; i >= 0; i--)
		{
			if (!SkeletonBoneInfo[PotentialHandBones[i].Bones[0]].Name.ToString().Contains(TEXT("Hand")))
			{
				PotentialHandsToRemove.Add(i);
			}
		}
		if (PotentialHandBones.Num() - PotentialHandsToRemove.Num() >= 2)
		{
			// Only remove them if we have at least 2 hands left
			for (int i = 0; i < PotentialHandsToRemove.Num(); i++)
			{
				PotentialHandBones.RemoveAt(PotentialHandsToRemove[i]);
			}
		}

		// If we didn't find axes using legs, try to figure out axes using hand bones
		if (LeftAxis.IsZero())
		{
			// We assume fingers are oriented outward, and thumbs are oriented forward (most often the case in Tpose)
			// Lateral axis (without sign, 50/50 to be right)
			FVector HandSpaceLocation = ComponentSpaceTransforms[PotentialHandBones[0].Bones[0]].GetLocation(); // Hand bone
			FVector MiddleSpaceLocation = ComponentSpaceTransforms[PotentialHandBones[0].Bones[4]].GetLocation(); // Firt bone of index finger
			float XDiff = FMath::Abs(HandSpaceLocation.X - MiddleSpaceLocation.X);
			float YDiff = FMath::Abs(HandSpaceLocation.Y - MiddleSpaceLocation.Y);
			if (XDiff > YDiff)
			{
				LeftAxis = FVector(1, 0, 0);
				FrontAxis = FVector(0, 1, 0);
			}
			else
			{
				LeftAxis = FVector(0, 1, 0);
				FrontAxis = FVector(-1, 0, 0);
			}

			// Figure out the axis signs using the thumb, which is normally pointing forward
			FVector ThumbFirstPhalangeSpaceLocation = ComponentSpaceTransforms[PotentialHandBones[0].Bones[1]].GetLocation();
			FVector ThumbLastPhalangeSpaceLocation = ComponentSpaceTransforms[PotentialHandBones[0].Bones[3]].GetLocation();
			if (LeftAxis.X > 0.0f)
			{
				if (ThumbLastPhalangeSpaceLocation.Y > ThumbFirstPhalangeSpaceLocation.Y)
				{
					FrontAxis = FVector(0, 1, 0);
					LeftAxis = FVector(1, 0, 0);
				}
				else
				{
					FrontAxis = FVector(0, -1, 0);
					LeftAxis = FVector(-1, 0, 0);
				}
			}
			else
			{
				if (ThumbLastPhalangeSpaceLocation.X > ThumbFirstPhalangeSpaceLocation.X)
				{
					FrontAxis = FVector(1, 0, 0);
					LeftAxis = FVector(0, -1, 0);
				}
				else
				{
					FrontAxis = FVector(-1, 0, 0);
					LeftAxis = FVector(0, 1, 0);
				}
			}
		}

		// Now we have axes info, figure out which hand is which and fill in bones accordingly
		for (FAutomaticBoneNameMappingBoneGroup Group : PotentialHandBones)
		{
			if (Group.Bones.Num() > 0)
			{
				int ManusHandBoneNameIndex = INDEX_NONE;
				int ManusFingerBoneNameIndex = INDEX_NONE;
				FVector HandSpaceLocation = ComponentSpaceTransforms[Group.Bones[0]].GetLocation();
				if (FMath::Sign(HandSpaceLocation.X) * FMath::Sign(LeftAxis.X) == 1 || FMath::Sign(HandSpaceLocation.Y) * FMath::Sign(LeftAxis.Y) == 1)
				{
					ManusHandBoneNameIndex = (int)EManusBoneName::LeftHand;
					ManusFingerBoneNameIndex = (int)EManusBoneName::LeftHandThumb1;
				}
				else
				{
					ManusHandBoneNameIndex = (int)EManusBoneName::RightHand;
					ManusFingerBoneNameIndex = (int)EManusBoneName::RightHandThumb1;
				}

				BoneIndexMap[ManusHandBoneNameIndex] = Group.Bones[0];
				for (int i = 1; i < Group.Bones.Num(); i++)
				{
					BoneIndexMap[ManusFingerBoneNameIndex++] = Group.Bones[i];
				}
			}
		}

	}
	else if (PotentialHandBones.Num() == 1)
	{
		// If we found only one hand, fill both side with the same bone names
		BoneIndexMap[(int)EManusBoneName::LeftHand] = PotentialHandBones[0].Bones[0];
		BoneIndexMap[(int)EManusBoneName::RightHand] = PotentialHandBones[0].Bones[0];
		int ManusBoneNameIndexLeft = (int)EManusBoneName::LeftHandThumb1;
		int ManusBoneNameIndexRight = (int)EManusBoneName::RightHandThumb1;
		for (int i = 1; i < PotentialHandBones[0].Bones.Num(); i++)
		{
			BoneIndexMap[ManusBoneNameIndexLeft++] = PotentialHandBones[0].Bones[i];
			BoneIndexMap[ManusBoneNameIndexRight++] = PotentialHandBones[0].Bones[i];
		}
	}

	// Figure out arms bones starting from the hands and going up in the bone hierarchy
	if (BoneIndexMap[(int)EManusBoneName::LeftHand] != INDEX_NONE && BoneIndexMap[(int)EManusBoneName::RightHand] != INDEX_NONE)
	{
		// We are looking for LowerArm, UpperArm and Shoulder bones, take the parents of the hands
		BoneIndexMap[(int)EManusBoneName::LeftLowerArm] = SkeletonBoneInfo[BoneIndexMap[(int)EManusBoneName::LeftHand]].Parent;
		if (BoneIndexMap[(int)EManusBoneName::LeftLowerArm] != INDEX_NONE)
		{
			BoneIndexMap[(int)EManusBoneName::LeftUpperArm] = SkeletonBoneInfo[BoneIndexMap[(int)EManusBoneName::LeftLowerArm]].Parent;
		}
		if (BoneIndexMap[(int)EManusBoneName::LeftUpperArm] != INDEX_NONE)
		{
			BoneIndexMap[(int)EManusBoneName::LeftShoulder] = SkeletonBoneInfo[BoneIndexMap[(int)EManusBoneName::LeftUpperArm]].Parent;
		}
		BoneIndexMap[(int)EManusBoneName::RightLowerArm] = SkeletonBoneInfo[BoneIndexMap[(int)EManusBoneName::RightHand]].Parent;
		if (BoneIndexMap[(int)EManusBoneName::RightLowerArm] != INDEX_NONE)
		{
			BoneIndexMap[(int)EManusBoneName::RightUpperArm] = SkeletonBoneInfo[BoneIndexMap[(int)EManusBoneName::RightLowerArm]].Parent;
		}
		if (BoneIndexMap[(int)EManusBoneName::RightUpperArm] != INDEX_NONE)
		{
			BoneIndexMap[(int)EManusBoneName::RightShoulder] = SkeletonBoneInfo[BoneIndexMap[(int)EManusBoneName::RightUpperArm]].Parent;
		}
	}

	// Look for the junction of both arms, which will be the UpperChest bone
	if (BoneIndexMap[(int)EManusBoneName::LeftShoulder] != INDEX_NONE && BoneIndexMap[(int)EManusBoneName::LeftShoulder] != BoneIndexMap[(int)EManusBoneName::RightShoulder])
	{
		int LeftParent = BoneIndexMap[(int)EManusBoneName::LeftShoulder];
		int RightParent = BoneIndexMap[(int)EManusBoneName::RightShoulder];
		do {
			LeftParent = SkeletonBoneInfo[LeftParent].Parent;
			RightParent = SkeletonBoneInfo[RightParent].Parent;
		} while (LeftParent != RightParent && LeftParent != INDEX_NONE && RightParent != INDEX_NONE);

		if (LeftParent == RightParent && LeftParent != INDEX_NONE)
		{
			BoneIndexMap[(int)EManusBoneName::UpperChest] = LeftParent;
		}
	}

	// Between the Hips and UpperChest, there are the Chest and Spine bones
	if (BoneIndexMap[(int)EManusBoneName::Hips] != INDEX_NONE && BoneIndexMap[(int)EManusBoneName::UpperChest] != INDEX_NONE)
	{
		// Get all the bones in-between
		FAutomaticBoneNameMappingBoneGroup Group;
		int Parent = SkeletonBoneInfo[BoneIndexMap[(int)EManusBoneName::UpperChest]].Parent;
		while (Parent != BoneIndexMap[(int)EManusBoneName::Hips] && Parent != INDEX_NONE)
		{
			Group.Bones.Add(SkeletonBoneInfo[Parent].Index);
			Parent = SkeletonBoneInfo[Parent].Parent;
		}

		// Pick the two bones in the middle
		if (Parent != INDEX_NONE)
		{
			float Div = Group.Bones.Num() / 3.0f;
			int FirstIndex = FMath::FloorToInt(Div);
			if (Group.Bones.IsValidIndex(FirstIndex))
			{
				BoneIndexMap[(int)EManusBoneName::Chest] = Group.Bones[FirstIndex];
			}
			int SecondIndex = FMath::FloorToInt(2 * Div);
			if (Group.Bones.IsValidIndex(SecondIndex))
			{
				BoneIndexMap[(int)EManusBoneName::Spine] = Group.Bones[SecondIndex];
			}
		}
	}

	// Finally we need to find the Neck and Head bones
	if (BoneIndexMap[(int)EManusBoneName::UpperChest] != INDEX_NONE)
	{
		// Take the first child of UpperChest that is not a shoulder, and that is at least two bones deep
		FAutomaticBoneNameMappingBoneInfo& BoneInfo = SkeletonBoneInfo[BoneIndexMap[(int)EManusBoneName::UpperChest]];
		for (int i = 0; i < BoneInfo.Children.Num(); i++)
		{
			if (SkeletonBoneInfo[BoneInfo.Children[i]].ChildMaxDepth >= 1 && BoneInfo.Children[i] != BoneIndexMap[(int)EManusBoneName::LeftShoulder] && BoneInfo.Children[i] != BoneIndexMap[(int)EManusBoneName::RightShoulder])
			{
				BoneIndexMap[(int)EManusBoneName::Neck] = BoneInfo.Children[i];
				BoneIndexMap[(int)EManusBoneName::Head] = SkeletonBoneInfo[BoneInfo.Children[i]].Children[0];
				break;
			}
		}
	}

	// Set the bone name map
	UE_LOG(LogManus, Log, TEXT("Automatic Bone Name Mapping started"));
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EManusBoneName"), true);
	for (int i = 0; i < (int)EManusBoneName::Max; i++)
	{
		FName ManusBoneName = FName(*EnumPtr->GetNameStringByIndex(i));
		UE_LOG(LogManus, Log, TEXT("%s -> %s"), *ManusBoneName.ToString(), BoneIndexMap[i] != INDEX_NONE ? *SkeletonBoneInfo[BoneIndexMap[i]].Name.ToString() : TEXT("NOT FOUND"));

		GET_BONE_NAME(BoneMap[i]) = (BoneIndexMap[i] == INDEX_NONE ? ManusBoneName : SkeletonBoneInfo[BoneIndexMap[i]].Name);
	}
	UE_LOG(LogManus, Log, TEXT("Automatic Bone Name Mapping finished"));
}

EBoneAxis ManusTools::AutomaticSkeletonStretchAxisDetection(USkeleton* Skeleton)
{
	const FReferenceSkeleton RefSkeleton = Skeleton->GetReferenceSkeleton();
	const TArray<FTransform> BonePoses = RefSkeleton.GetRefBonePose();

	FVector AllBoneLength = FVector::ZeroVector;
	for (int i = 1; i < BonePoses.Num(); i++)
	{
		AllBoneLength += BonePoses[i].GetLocation().GetAbs();
	}

	EBoneAxis Axis = BA_X;
	if (AllBoneLength.Y > AllBoneLength.X && AllBoneLength.Y > AllBoneLength.Z)
	{
		Axis = BA_Y;
	}
	if (AllBoneLength.Z > AllBoneLength.Y && AllBoneLength.Z > AllBoneLength.X)
	{
		Axis = BA_Z;
	}

	return Axis;
}

float ManusTools::AutomaticSkeletonHeightDetection(USkeleton* Skeleton)
{
	const FReferenceSkeleton RefSkeleton = Skeleton->GetReferenceSkeleton();
	const TArray<FTransform> BonePoses = RefSkeleton.GetRefBonePose();

	// Skeleton component space transforms
	TArray<FTransform> ComponentSpaceTransforms;
	FAnimationRuntime::FillUpComponentSpaceTransforms(RefSkeleton, BonePoses, ComponentSpaceTransforms);

	// Use the heighest bone
	float Height = 0.0f;
	for (int i = 0; i < ComponentSpaceTransforms.Num(); i++)
	{
		float BoneHeight = ComponentSpaceTransforms[i].GetLocation().Z;
		if (BoneHeight > Height)
		{
			Height = BoneHeight;
		}
	}

	// Convert to Unity unit (meters)
	Height = FMath::CeilToInt(Height);
	Height /= 100.0f;

	return Height;
}

void ManusTools::AutomaticFingersRotationAxesDetection(UManusSkeleton* ManusSkeleton, EManusHandType HandType, EManusAxisOption& OutStretchAxis, EManusAxisOption& OutSpreadAxis)
{
	EManusAxisOption Axis = EManusAxisOption::Z_Neg;

	if (ManusSkeleton->GetSkeleton())
	{
		const FReferenceSkeleton RefSkeleton = ManusSkeleton->GetSkeleton()->GetReferenceSkeleton();
		const TArray<FTransform> BonePoses = RefSkeleton.GetRefBonePose();
		TArray<FTransform> ModifiableBonePoses;
		ModifiableBonePoses.Append(BonePoses);

		// Compute original Skeleton component space transforms
		TArray<FTransform> RefComponentSpaceTransforms;
		FAnimationRuntime::FillUpComponentSpaceTransforms(RefSkeleton, BonePoses, RefComponentSpaceTransforms);

		// Try all the angles on the middle finger first joint and see the effect on the middle joint in hand space
		FVector MiddleFingerJoint2NewLocations[(int)EManusAxisOption::Max];
		int MiddleFingerJoint1BoneIndex = (HandType == EManusHandType::Left) ? (int)EManusBoneName::LeftHandMiddle1 : (int)EManusBoneName::RightHandMiddle1;
		int MiddleFingerJoint2BoneIndex = (HandType == EManusHandType::Left) ? (int)EManusBoneName::LeftHandMiddle2 : (int)EManusBoneName::RightHandMiddle2;
		int32 SkeletonMiddleFingerJoint1BoneIndex = RefSkeleton.FindBoneIndex(GET_BONE_NAME(ManusSkeleton->BoneMap[MiddleFingerJoint1BoneIndex]));
		int32 SkeletonMiddleFingerJoint2BoneIndex = RefSkeleton.FindBoneIndex(GET_BONE_NAME(ManusSkeleton->BoneMap[MiddleFingerJoint2BoneIndex]));
		if (SkeletonMiddleFingerJoint1BoneIndex != INDEX_NONE && SkeletonMiddleFingerJoint2BoneIndex != INDEX_NONE)
		{
			for (int i = 0; i < (int)EManusAxisOption::Max; i++)
			{
				// Rotate the joint 90 degrees around the axis
				FQuat RotationQuaternion = ManusTools::AngleAxis(90.0f, (EManusAxisOption)i);
				ModifiableBonePoses[SkeletonMiddleFingerJoint1BoneIndex].SetRotation(BonePoses[SkeletonMiddleFingerJoint1BoneIndex].GetRotation() * RotationQuaternion);

				// Compute Skeleton component space transforms
				TArray<FTransform> NewComponentSpaceTransforms;
				FAnimationRuntime::FillUpComponentSpaceTransforms(RefSkeleton, ModifiableBonePoses, NewComponentSpaceTransforms);
				MiddleFingerJoint2NewLocations[i] = NewComponentSpaceTransforms[SkeletonMiddleFingerJoint2BoneIndex].GetLocation();
			}
		}

		// Compare thumb location and new middle finger location
		int ThumbBoneIndex = (HandType == EManusHandType::Left) ? (int)EManusBoneName::LeftHandThumb3 : (int)EManusBoneName::RightHandThumb3;
		int32 SkeletonThumbBoneIndex = RefSkeleton.FindBoneIndex(GET_BONE_NAME(ManusSkeleton->BoneMap[ThumbBoneIndex]));
		if (SkeletonThumbBoneIndex != INDEX_NONE)
		{
			FVector ThumbLocation = RefComponentSpaceTransforms[SkeletonThumbBoneIndex].GetLocation();

			OutSpreadAxis = EManusAxisOption::X;
			for (int i = 1; i < (int)EManusAxisOption::Max; i++)
			{
				float Dist = FVector::DistSquared(MiddleFingerJoint2NewLocations[i], ThumbLocation);
				float StretchDist = FVector::DistSquared(MiddleFingerJoint2NewLocations[(int)OutStretchAxis], ThumbLocation);
				float SpreadDist = FVector::DistSquared(MiddleFingerJoint2NewLocations[(int)OutSpreadAxis], ThumbLocation);
				if (Dist < SpreadDist)
				{
					// Spread axis
					OutSpreadAxis = (EManusAxisOption)i;
				}
				else if (Dist < StretchDist)
				{
					// Stretch axis
					OutStretchAxis = (EManusAxisOption)i;
				}
			}
		}
	}
}
#endif // WITH_EDITORONLY_DATA

bool ManusTools::CalculateManusInternalOrientations(int ManusLiveLinkUserIndex, TMap<int, FQuat>& OutOrientations, TMap<int, FQuat>& OutDeltaOrientations)
{
	OutOrientations.Reset();

	const TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	if (ManusLiveLinkUsers.IsValidIndex(ManusLiveLinkUserIndex))
	{
		UManusSkeleton* ManusSkeleton = ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusSkeleton;
		if (ManusSkeleton)
		{
			const BoneName_t* BoneMap = ManusSkeleton->BoneMap;
			const FReferenceSkeleton RefSkeleton = ManusSkeleton->GetSkeleton()->GetReferenceSkeleton();

			for (int i = 0; i < (int)EManusBoneName::Max; i++)
			{
				int32 BoneIndex = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[i]));
				if (BoneIndex != INDEX_NONE)
				{
					FQuat Orientation, DeltaOrientation;
					if (ManusTools::CalculateManusInternalOrientation((EManusBoneName)i, ManusSkeleton->GetSkeleton(), BoneMap, Orientation, DeltaOrientation))
					{
						OutOrientations.Add(i, Orientation);
						OutDeltaOrientations.Add(i, DeltaOrientation);
					}
				}
			}
			return true;
		}
	}
	return false;
}

#define GET_BONE_INDEX_OR_FAIL(NameOfBoneToGetIndexFrom) RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::NameOfBoneToGetIndexFrom])); \
	if (##NameOfBoneToGetIndexFrom##BoneIndex == INDEX_NONE) \
	{ \
		UE_LOG(LogManus, Warning, TEXT("ManusTools::CalculateManusInternalOrientation failed because the NameOfBoneToGetIndexFrom bone was not found.")); \
		return false; \
	}


bool ManusTools::CalculateManusInternalOrientation(EManusBoneName Bone, USkeleton* Skeleton, const BoneName_t* BoneMap, FQuat& OutOrientation, FQuat& OutDeltaOrientation)
{
	const FReferenceSkeleton RefSkeleton = Skeleton->GetReferenceSkeleton();
	const TArray<FTransform> BonePoses = RefSkeleton.GetRefBonePose();

	// Skeleton component space transforms
	TArray<FTransform> ComponentSpaceTransforms;
	FAnimationRuntime::FillUpComponentSpaceTransforms(RefSkeleton, BonePoses, ComponentSpaceTransforms);

	// Convert component space transforms to Manus coordinates system (= Unity coordinates system)
	for (int i = 0; i < ComponentSpaceTransforms.Num(); i++)
	{
		ComponentSpaceTransforms[i] = ManusConvert::ConvertUnrealToUnityTransform(ComponentSpaceTransforms[i]);
	}
	
	// Bone data
	int32 BoneIndex = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)Bone]));
	if (BoneIndex == INDEX_NONE)
	{
		UE_LOG(LogManus, Warning, TEXT("ManusTools::CalculateManusInternalOrientation failed because the bone %s was not found."), *GET_BONE_NAME(BoneMap[(int)Bone]).ToString());
		return false;
	}
	FTransform BoneTransform = ComponentSpaceTransforms[BoneIndex];

	// According to which bone we want to calculate the orientation for
	switch (Bone)
	{
	case EManusBoneName::Root:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);
		int32 LeftFootBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftFoot);
		int32 RightFootBoneIndex = GET_BONE_INDEX_OR_FAIL(RightFoot);

		TArray<TTuple<FTransform, FTransform>> Bones;
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftHandBoneIndex], ComponentSpaceTransforms[RightHandBoneIndex]));
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftFootBoneIndex], ComponentSpaceTransforms[RightFootBoneIndex]));

		FVector AimDirection = ManusTools::CalculateManusInternalForward(Bones);

		OutOrientation = ManusTools::LookRotation(AimDirection, ManusConvert::MANUS_UP_VECTOR);
	}
	break;

	case EManusBoneName::Head:
	case EManusBoneName::LeftFoot:
	case EManusBoneName::RightFoot:
	case EManusBoneName::LeftToes:
	case EManusBoneName::RightToes:
	case EManusBoneName::LeftToesEnd:
	case EManusBoneName::RightToesEnd:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);
		int32 LeftFootBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftFoot);
		int32 RightFootBoneIndex = GET_BONE_INDEX_OR_FAIL(RightFoot);

		TArray<TTuple<FTransform, FTransform>> Bones;
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftHandBoneIndex], ComponentSpaceTransforms[RightHandBoneIndex]));
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftFootBoneIndex], ComponentSpaceTransforms[RightFootBoneIndex]));

		FVector AimDirection = ManusTools::CalculateManusInternalForward(Bones);

		OutOrientation = ManusTools::LookRotation(AimDirection, ManusConvert::MANUS_UP_VECTOR);
	}
	break;

	case EManusBoneName::Hips:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);
		int32 LeftFootBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftFoot);
		int32 RightFootBoneIndex = GET_BONE_INDEX_OR_FAIL(RightFoot);

		TArray<TTuple<FTransform, FTransform>> Bones;
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftHandBoneIndex], ComponentSpaceTransforms[RightHandBoneIndex]));
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftFootBoneIndex], ComponentSpaceTransforms[RightFootBoneIndex]));

		int32 SpineBoneIndex = GET_BONE_INDEX_OR_FAIL(Spine);

		FVector AimDirection = ComponentSpaceTransforms[SpineBoneIndex].GetLocation() - BoneTransform.GetLocation();
		FVector UpDirection = -ManusTools::CalculateManusInternalForward(Bones);

		OutOrientation = ManusTools::LookRotation(AimDirection, UpDirection);
	}
	break;

	case EManusBoneName::Neck:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);
		int32 LeftFootBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftFoot);
		int32 RightFootBoneIndex = GET_BONE_INDEX_OR_FAIL(RightFoot);

		TArray<TTuple<FTransform, FTransform>> Bones;
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftHandBoneIndex], ComponentSpaceTransforms[RightHandBoneIndex]));
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftFootBoneIndex], ComponentSpaceTransforms[RightFootBoneIndex]));

		int32 HeadBoneIndex = GET_BONE_INDEX_OR_FAIL(Head);

		FVector AimDirection = ComponentSpaceTransforms[HeadBoneIndex].GetLocation() - BoneTransform.GetLocation();
		FVector UpDirection = -ManusTools::CalculateManusInternalForward(Bones);

		OutOrientation = ManusTools::LookRotation(AimDirection, UpDirection);
	}
	break;

	case EManusBoneName::Spine:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);
		int32 LeftFootBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftFoot);
		int32 RightFootBoneIndex = GET_BONE_INDEX_OR_FAIL(RightFoot);

		TArray<TTuple<FTransform, FTransform>> Bones;
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftHandBoneIndex], ComponentSpaceTransforms[RightHandBoneIndex]));
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftFootBoneIndex], ComponentSpaceTransforms[RightFootBoneIndex]));

		FTransform ChestOrNeckBoneTransform;
		int32 ChestBoneIndex = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::Chest]));
		if (ChestBoneIndex == INDEX_NONE)
		{
			int32 NeckBoneIndex = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::Neck]));
			if (NeckBoneIndex == INDEX_NONE)
			{
				UE_LOG(LogManus, Warning, TEXT("ManusTools::CalculateManusInternalOrientation failed because the Chest and Neck bones were both not found."));
				return false;
			}
			else
			{
				ChestOrNeckBoneTransform = ComponentSpaceTransforms[NeckBoneIndex];
			}
		}
		else
		{
			ChestOrNeckBoneTransform = ComponentSpaceTransforms[ChestBoneIndex];
		}

		FVector AimDirection = ChestOrNeckBoneTransform.GetLocation() - BoneTransform.GetLocation();
		FVector UpDirection = -ManusTools::CalculateManusInternalForward(Bones);

		OutOrientation = ManusTools::LookRotation(AimDirection, UpDirection);
	}
	break;

	case EManusBoneName::Chest:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);
		int32 LeftFootBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftFoot);
		int32 RightFootBoneIndex = GET_BONE_INDEX_OR_FAIL(RightFoot);

		TArray<TTuple<FTransform, FTransform>> Bones;
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftHandBoneIndex], ComponentSpaceTransforms[RightHandBoneIndex]));
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftFootBoneIndex], ComponentSpaceTransforms[RightFootBoneIndex]));

		FTransform UpperChestOrNeckBoneTransform;
		int32 UpperChestBoneIndex = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::UpperChest]));
		if (UpperChestBoneIndex == INDEX_NONE)
		{
			int32 NeckBoneIndex = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::Neck]));
			if (NeckBoneIndex == INDEX_NONE)
			{
				UE_LOG(LogManus, Warning, TEXT("ManusTools::CalculateManusInternalOrientation failed because the UpperChest and Neck bones were both not found."));
				return false;
			}
			else
			{
				UpperChestOrNeckBoneTransform = ComponentSpaceTransforms[NeckBoneIndex];
			}
		}
		else
		{
			UpperChestOrNeckBoneTransform = ComponentSpaceTransforms[UpperChestBoneIndex];
		}

		FVector AimDirection = UpperChestOrNeckBoneTransform.GetLocation() - BoneTransform.GetLocation();
		FVector UpDirection = -ManusTools::CalculateManusInternalForward(Bones);

		OutOrientation = ManusTools::LookRotation(AimDirection, UpDirection);
	}
	break;

	case EManusBoneName::UpperChest:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);
		int32 LeftFootBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftFoot);
		int32 RightFootBoneIndex = GET_BONE_INDEX_OR_FAIL(RightFoot);

		TArray<TTuple<FTransform, FTransform>> Bones;
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftHandBoneIndex], ComponentSpaceTransforms[RightHandBoneIndex]));
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftFootBoneIndex], ComponentSpaceTransforms[RightFootBoneIndex]));

		int32 NeckBoneIndex = GET_BONE_INDEX_OR_FAIL(Neck);

		FVector AimDirection = ComponentSpaceTransforms[NeckBoneIndex].GetLocation() - BoneTransform.GetLocation();
		FVector UpDirection = -ManusTools::CalculateManusInternalForward(Bones);

		OutOrientation = ManusTools::LookRotation(AimDirection, UpDirection);
	}
	break;

	case EManusBoneName::LeftUpperLeg:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);
		int32 LeftFootBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftFoot);
		int32 RightFootBoneIndex = GET_BONE_INDEX_OR_FAIL(RightFoot);

		TArray<TTuple<FTransform, FTransform>> Bones;
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftHandBoneIndex], ComponentSpaceTransforms[RightHandBoneIndex]));
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftFootBoneIndex], ComponentSpaceTransforms[RightFootBoneIndex]));

		int32 LeftLowerLegBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftLowerLeg);

		FVector AimDirection = ComponentSpaceTransforms[LeftLowerLegBoneIndex].GetLocation() - BoneTransform.GetLocation();
		FVector UpDirection = ManusTools::CalculateManusInternalForward(Bones);

		OutOrientation = ManusTools::LookRotation(AimDirection, UpDirection);
	}
	break;

	case EManusBoneName::RightUpperLeg:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);
		int32 LeftFootBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftFoot);
		int32 RightFootBoneIndex = GET_BONE_INDEX_OR_FAIL(RightFoot);

		TArray<TTuple<FTransform, FTransform>> Bones;
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftHandBoneIndex], ComponentSpaceTransforms[RightHandBoneIndex]));
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftFootBoneIndex], ComponentSpaceTransforms[RightFootBoneIndex]));

		int32 RightLowerLegBoneIndex = GET_BONE_INDEX_OR_FAIL(RightLowerLeg);

		FVector AimDirection = ComponentSpaceTransforms[RightLowerLegBoneIndex].GetLocation() - BoneTransform.GetLocation();
		FVector UpDirection = ManusTools::CalculateManusInternalForward(Bones);

		OutOrientation = ManusTools::LookRotation(AimDirection, UpDirection);
	}
	break;

	case EManusBoneName::LeftLowerLeg:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);
		int32 LeftFootBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftFoot);
		int32 RightFootBoneIndex = GET_BONE_INDEX_OR_FAIL(RightFoot);

		TArray<TTuple<FTransform, FTransform>> Bones;
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftHandBoneIndex], ComponentSpaceTransforms[RightHandBoneIndex]));
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftFootBoneIndex], ComponentSpaceTransforms[RightFootBoneIndex]));

		FVector AimDirection = ComponentSpaceTransforms[LeftFootBoneIndex].GetLocation() - BoneTransform.GetLocation();
		FVector UpDirection = ManusTools::CalculateManusInternalForward(Bones);

		OutOrientation = ManusTools::LookRotation(AimDirection, UpDirection);
	}
	break;

	case EManusBoneName::RightLowerLeg:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);
		int32 LeftFootBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftFoot);
		int32 RightFootBoneIndex = GET_BONE_INDEX_OR_FAIL(RightFoot);

		TArray<TTuple<FTransform, FTransform>> Bones;
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftHandBoneIndex], ComponentSpaceTransforms[RightHandBoneIndex]));
		Bones.Add(TTuple<FTransform, FTransform>(ComponentSpaceTransforms[LeftFootBoneIndex], ComponentSpaceTransforms[RightFootBoneIndex]));

		FVector AimDirection = ComponentSpaceTransforms[RightFootBoneIndex].GetLocation() - BoneTransform.GetLocation();
		FVector UpDirection = ManusTools::CalculateManusInternalForward(Bones);

		OutOrientation = ManusTools::LookRotation(AimDirection, UpDirection);
	}
	break;

	case EManusBoneName::LeftShoulder:
	{
		int32 LeftUpperArmBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftUpperArm);

		FVector AimDirection = ComponentSpaceTransforms[LeftUpperArmBoneIndex].GetLocation() - BoneTransform.GetLocation();

		OutOrientation = ManusTools::LookRotation(AimDirection, ManusConvert::MANUS_UP_VECTOR);
	}
	break;

	case EManusBoneName::RightShoulder:
	{
		int32 RightUpperArmBoneIndex = GET_BONE_INDEX_OR_FAIL(RightUpperArm);

		FVector AimDirection = ComponentSpaceTransforms[RightUpperArmBoneIndex].GetLocation() - BoneTransform.GetLocation();

		OutOrientation = ManusTools::LookRotation(AimDirection, ManusConvert::MANUS_UP_VECTOR);
	}
	break;

	case EManusBoneName::LeftUpperArm:
	{
		int32 LeftLowerArmBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftLowerArm);

		FVector AimDirection = ComponentSpaceTransforms[LeftLowerArmBoneIndex].GetLocation() - BoneTransform.GetLocation();

		OutOrientation = ManusTools::LookRotation(AimDirection, ManusConvert::MANUS_UP_VECTOR);
	}

	break;

	case EManusBoneName::RightUpperArm:
	{
		int32 RightLowerArmBoneIndex = GET_BONE_INDEX_OR_FAIL(RightLowerArm);

		FVector AimDirection = ComponentSpaceTransforms[RightLowerArmBoneIndex].GetLocation() - BoneTransform.GetLocation();

		OutOrientation = ManusTools::LookRotation(AimDirection, ManusConvert::MANUS_UP_VECTOR);
	}
	break;

	case EManusBoneName::LeftLowerArm:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);
		int32 LeftUpperArmBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftUpperArm);

		FVector AimDirection = ComponentSpaceTransforms[LeftHandBoneIndex].GetLocation() - BoneTransform.GetLocation();

		FVector ArmSide = FVector::CrossProduct(BoneTransform.GetLocation() - ComponentSpaceTransforms[LeftUpperArmBoneIndex].GetLocation(), ManusConvert::MANUS_UP_VECTOR).GetSafeNormal();
		FVector UpDirection = FVector::CrossProduct(ArmSide, BoneTransform.GetLocation() - ComponentSpaceTransforms[LeftUpperArmBoneIndex].GetLocation()).GetSafeNormal();

		OutOrientation = ManusTools::LookRotation(AimDirection, UpDirection);
	}
	break;

	case EManusBoneName::RightLowerArm:
	{
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);
		int32 RightUpperArmBoneIndex = GET_BONE_INDEX_OR_FAIL(RightUpperArm);

		FVector AimDirection = ComponentSpaceTransforms[RightHandBoneIndex].GetLocation() - BoneTransform.GetLocation();

		FVector ArmSide = FVector::CrossProduct(BoneTransform.GetLocation() - ComponentSpaceTransforms[RightUpperArmBoneIndex].GetLocation(), ManusConvert::MANUS_UP_VECTOR).GetSafeNormal();
		FVector UpDirection = FVector::CrossProduct(ArmSide, BoneTransform.GetLocation() - ComponentSpaceTransforms[RightUpperArmBoneIndex].GetLocation()).GetSafeNormal();
		
		OutOrientation = ManusTools::LookRotation(AimDirection, UpDirection);
	}
	break;

	case EManusBoneName::LeftHand:
	{
		int32 LeftHandBoneIndex = GET_BONE_INDEX_OR_FAIL(LeftHand);

		// Forward Vector
		FVector AimDirection = FVector::ZeroVector;
		int FingerCount = 0;

		int32 LeftHandIndex1Index = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::LeftHandIndex1]));
		if (LeftHandIndex1Index != INDEX_NONE)
		{
			AimDirection += ComponentSpaceTransforms[LeftHandIndex1Index].GetLocation() - ComponentSpaceTransforms[LeftHandBoneIndex].GetLocation();
			FingerCount++;
		}

		int32 LeftHandMiddle1Index = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::LeftHandMiddle1]));
		if (LeftHandMiddle1Index != INDEX_NONE)
		{
			AimDirection += ComponentSpaceTransforms[LeftHandMiddle1Index].GetLocation() - ComponentSpaceTransforms[LeftHandBoneIndex].GetLocation();
			FingerCount++;
		}

		int32 LeftHandRing1Index = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::LeftHandRing1]));
		if (LeftHandRing1Index != INDEX_NONE)
		{
			AimDirection += ComponentSpaceTransforms[LeftHandRing1Index].GetLocation() - ComponentSpaceTransforms[LeftHandBoneIndex].GetLocation();
			FingerCount++;
		}

		int32 LeftHandPinky1Index = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::LeftHandPinky1]));
		if (LeftHandPinky1Index != INDEX_NONE)
		{
			AimDirection += ComponentSpaceTransforms[LeftHandPinky1Index].GetLocation() - ComponentSpaceTransforms[LeftHandBoneIndex].GetLocation();
			FingerCount++;
		}

		AimDirection = (AimDirection / FingerCount).GetSafeNormal();

		// Up Vector
		FVector UpDirection = FVector::ZeroVector;
		FingerCount = 0;

		if (LeftHandIndex1Index != INDEX_NONE && LeftHandMiddle1Index != INDEX_NONE)
		{
			UpDirection += FVector::CrossProduct(ComponentSpaceTransforms[LeftHandMiddle1Index].GetLocation() - ComponentSpaceTransforms[LeftHandIndex1Index].GetLocation(), AimDirection);
			FingerCount++;
		}

		if (LeftHandMiddle1Index != INDEX_NONE && LeftHandRing1Index != INDEX_NONE)
		{
			UpDirection += FVector::CrossProduct(ComponentSpaceTransforms[LeftHandRing1Index].GetLocation() - ComponentSpaceTransforms[LeftHandMiddle1Index].GetLocation(), AimDirection);
			FingerCount++;
		}

		if (LeftHandRing1Index != INDEX_NONE && LeftHandPinky1Index != INDEX_NONE)
		{
			UpDirection += FVector::CrossProduct(ComponentSpaceTransforms[LeftHandPinky1Index].GetLocation() - ComponentSpaceTransforms[LeftHandRing1Index].GetLocation(), AimDirection);
			FingerCount++;
		}

		UpDirection = (UpDirection / FingerCount).GetSafeNormal();

		OutOrientation = ManusTools::LookRotation(AimDirection, UpDirection);
	}
	break;

	case EManusBoneName::RightHand:
	{
		int32 RightHandBoneIndex = GET_BONE_INDEX_OR_FAIL(RightHand);

		// Forward Vector
		FVector AimDirection = FVector::ZeroVector;
		int FingerCount = 0;

		int32 RightHandIndex1Index = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::RightHandIndex1]));
		if (RightHandIndex1Index != INDEX_NONE)
		{
			AimDirection += ComponentSpaceTransforms[RightHandIndex1Index].GetLocation() - ComponentSpaceTransforms[RightHandBoneIndex].GetLocation();
			FingerCount++;
		}

		int32 RightHandMiddle1Index = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::RightHandMiddle1]));
		if (RightHandMiddle1Index != INDEX_NONE)
		{
			AimDirection += ComponentSpaceTransforms[RightHandMiddle1Index].GetLocation() - ComponentSpaceTransforms[RightHandBoneIndex].GetLocation();
			FingerCount++;
		}

		int32 RightHandRing1Index = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::RightHandRing1]));
		if (RightHandRing1Index != INDEX_NONE)
		{
			AimDirection += ComponentSpaceTransforms[RightHandRing1Index].GetLocation() - ComponentSpaceTransforms[RightHandBoneIndex].GetLocation();
			FingerCount++;
		}

		int32 RightHandPinky1Index = RefSkeleton.FindBoneIndex(GET_BONE_NAME(BoneMap[(int)EManusBoneName::RightHandPinky1]));
		if (RightHandPinky1Index != INDEX_NONE)
		{
			AimDirection += ComponentSpaceTransforms[RightHandPinky1Index].GetLocation() - ComponentSpaceTransforms[RightHandBoneIndex].GetLocation();
			FingerCount++;
		}

		AimDirection = (AimDirection / FingerCount).GetSafeNormal();

		// Up Vector
		FVector UpDirection = FVector::ZeroVector;
		FingerCount = 0;

		if (RightHandIndex1Index != INDEX_NONE && RightHandMiddle1Index != INDEX_NONE)
		{
			UpDirection += FVector::CrossProduct(ComponentSpaceTransforms[RightHandMiddle1Index].GetLocation() - ComponentSpaceTransforms[RightHandIndex1Index].GetLocation(), AimDirection);
			FingerCount++;
		}

		if (RightHandMiddle1Index != INDEX_NONE && RightHandRing1Index != INDEX_NONE)
		{
			UpDirection += FVector::CrossProduct(ComponentSpaceTransforms[RightHandRing1Index].GetLocation() - ComponentSpaceTransforms[RightHandMiddle1Index].GetLocation(), AimDirection);
			FingerCount++;
		}

		if (RightHandRing1Index != INDEX_NONE && RightHandPinky1Index != INDEX_NONE)
		{
			UpDirection += FVector::CrossProduct(ComponentSpaceTransforms[RightHandPinky1Index].GetLocation() - ComponentSpaceTransforms[RightHandRing1Index].GetLocation(), AimDirection);
			FingerCount++;
		}

		UpDirection = (UpDirection / FingerCount).GetSafeNormal();

		OutOrientation = ManusTools::LookRotation(AimDirection, -UpDirection);
	}
	break;

	case EManusBoneName::LeftHandThumb1:
	case EManusBoneName::RightHandThumb1:
	case EManusBoneName::LeftHandIndex1:
	case EManusBoneName::RightHandIndex1:
	case EManusBoneName::LeftHandMiddle1:
	case EManusBoneName::RightHandMiddle1:
	case EManusBoneName::LeftHandRing1:
	case EManusBoneName::RightHandRing1:
	case EManusBoneName::LeftHandPinky1:
	case EManusBoneName::RightHandPinky1:
	case EManusBoneName::LeftHandThumb2:
	case EManusBoneName::RightHandThumb2:
	case EManusBoneName::LeftHandIndex2:
	case EManusBoneName::RightHandIndex2:
	case EManusBoneName::LeftHandMiddle2:
	case EManusBoneName::RightHandMiddle2:
	case EManusBoneName::LeftHandRing2:
	case EManusBoneName::RightHandRing2:
	case EManusBoneName::LeftHandPinky2:
	case EManusBoneName::RightHandPinky2:
	case EManusBoneName::LeftHandThumb3:
	case EManusBoneName::RightHandThumb3:
	case EManusBoneName::LeftHandIndex3:
	case EManusBoneName::RightHandIndex3:
	case EManusBoneName::LeftHandMiddle3:
	case EManusBoneName::RightHandMiddle3:
	case EManusBoneName::LeftHandRing3:
	case EManusBoneName::RightHandRing3:
	case EManusBoneName::LeftHandPinky3:
	case EManusBoneName::RightHandPinky3:
		// Maybe implement this later, but for now, skip fingers
		break;

	default:
		UE_LOG(LogManus, Error, TEXT("No rotation available for this bone"));
		break;
	}

	// Normalize the orientation
	OutOrientation.GetNormalized();

	// Calculate delta with the reference skeleton orientation
	OutDeltaOrientation = BoneTransform.GetRotation().Inverse() * OutOrientation;

	return true;
}

FVector ManusTools::CalculateManusInternalForward(TArray<TTuple<FTransform, FTransform>> Directions)
{
	FVector Forward = FVector::ZeroVector;
	for (TTuple<FTransform, FTransform> Pair : Directions)
	{
		FVector Cross = FVector::CrossProduct(Pair.Value.GetLocation() - Pair.Key.GetLocation(), ManusConvert::MANUS_UP_VECTOR);
		Cross.Y = 0;
		Forward += Cross.GetSafeNormal();
	}

	return Forward / Directions.Num();
}

FQuat ManusTools::LookRotation(FVector LookAtDirection, FVector UpDirection)
{
	FVector Forward = LookAtDirection;
	FVector Up = UpDirection;

	Forward = Forward.GetSafeNormal();
	Up = Up - (Forward * FVector::DotProduct(Up, Forward));
	Up = Up.GetSafeNormal();

	FVector Vector = Forward.GetSafeNormal();
	FVector Vector2 = FVector::CrossProduct(Up, Vector);
	FVector Vector3 = FVector::CrossProduct(Vector, Vector2);
	float M00 = Vector2.X;
	float M01 = Vector2.Y;
	float M02 = Vector2.Z;
	float M10 = Vector3.X;
	float M11 = Vector3.Y;
	float M12 = Vector3.Z;
	float M20 = Vector.X;
	float M21 = Vector.Y;
	float M22 = Vector.Z;

	float Num8 = (M00 + M11) + M22;
	FQuat Quaternion = FQuat();
	if (Num8 > 0.0f)
	{
		float Num = (float)FMath::Sqrt(Num8 + 1.0f);
		Quaternion.W = Num * 0.5f;
		Num = 0.5f / Num;
		Quaternion.X = (M12 - M21) * Num;
		Quaternion.Y = (M20 - M02) * Num;
		Quaternion.Z = (M01 - M10) * Num;
		return Quaternion;
	}
	if (M00 >= M11 && M00 >= M22)
	{
		float Num7 = (float)FMath::Sqrt(((1.0f + M00) - M11) - M22);
		float Num4 = 0.5f / Num7;
		Quaternion.X = 0.5f * Num7;
		Quaternion.Y = (M01 + M10) * Num4;
		Quaternion.Z = (M02 + M20) * Num4;
		Quaternion.W = (M12 - M21) * Num4;
		return Quaternion;
	}
	if (M11 > M22)
	{
		float Num6 = (float)FMath::Sqrt(((1.0f + M11) - M00) - M22);
		float Num3 = 0.5f / Num6;
		Quaternion.X = (M10 + M01) * Num3;
		Quaternion.Y = 0.5f * Num6;
		Quaternion.Z = (M21 + M12) * Num3;
		Quaternion.W = (M20 - M02) * Num3;
		return Quaternion;
	}
	float Num5 = (float)FMath::Sqrt(((1.0f + M22) - M00) - M11);
	float Num2 = 0.5f / Num5;
	Quaternion.X = (M20 + M02) * Num2;
	Quaternion.Y = (M21 + M12) * Num2;
	Quaternion.Z = 0.5f * Num5;
	Quaternion.W = (M01 - M10) * Num2;

	return Quaternion;
}

FQuat ManusTools::AngleAxis(float Angle, EManusAxisOption Axis)
{
	FVector AxisVector = FVector::ZeroVector;
	switch (Axis)
	{
	case EManusAxisOption::X: AxisVector = FVector(1, 0, 0); break;
	case EManusAxisOption::Y: AxisVector = FVector(0, 1, 0); break;
	case EManusAxisOption::Z: AxisVector = FVector(0, 0, 1); break;
	case EManusAxisOption::X_Neg: AxisVector = FVector(-1, 0, 0); break;
	case EManusAxisOption::Y_Neg: AxisVector = FVector(0, -1, 0); break;
	case EManusAxisOption::Z_Neg: AxisVector = FVector(0, 0, -1); break;
	}
	AxisVector.Normalize();
	float Rad = FMath::DegreesToRadians(Angle) * 0.5f;
	AxisVector *= FMath::Sin(Rad);
	return FQuat(AxisVector.X, AxisVector.Y, AxisVector.Z, FMath::Cos(Rad));
}
