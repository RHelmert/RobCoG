// Copyright 2015-2022 Manus

#include "ManusBlueprintLibrary.h"
#include "Manus.h"
#include "ManusComponent.h"
#include "ManusTools.h"
#include "ManusSkeleton.h"
#include "ManusClientSkeleton.h" 
#include "ManusLiveLinkSource.h"
#include "CoreSdk.h"
#include <sstream>
#include "GameFramework/PlayerController.h"
#include "Engine/NetConnection.h"
#include "ManusConvert.h"


// Acticate / deactivate %Manus tracking.
void UManusBlueprintLibrary::SetManusActive(bool bNewIsActive)
{
	FManusModule::Get().SetActive(bNewIsActive);
}


/// @brief passes the current selected %Manus ip/host address for connections
/// @param p_ManusIP 
void UManusBlueprintLibrary::SetAsClient(bool p_IsClient)
{
    FManusModule::Get().SetAsClient(p_IsClient);
}

/// @brief passes the current selected %Manus ip/host address for connections
/// @param p_ManusIP 
void UManusBlueprintLibrary::SetManusCoreIP(FString p_ManusIP)
{
	FManusModule::Get().SetManusCoreIP(p_ManusIP);
}

/// @brief get all currently detected %Manus hosts ip/host addresses
/// @return 
TArray<FString> UManusBlueprintLibrary::GetManusCoreIPs()
{
	TArray<FString> t_Values;

	// add the found %Manus Core instances.
	FManusModule::Get().GetRemoteHosts(t_Values);	

	t_Values.Add("LocalHost"); // default local host value (this in case people don't get the ip address)
	t_Values.Add("127.0.0.1"); // default local host value
	return t_Values;
}

/// @brief Convert the given %Manus ID to a string. 
FString UManusBlueprintLibrary::ConvertManusIdToString(int64 ManusId)
{
	uint32_t ManusIdAsUint32 = ManusId;

	std::ostringstream Stream;
	Stream << std::hex << ManusIdAsUint32;

	return FString(Stream.str().c_str());
}

/// @brief get the current users from %Manus Core (if everything is just starting this may be 0 the first second while everything is getting filled in)
EManusRet UManusBlueprintLibrary::GetManusUsers(TArray<int64>& p_Users)
{
	int32 t_NumberOfUsers = 0;
	EManusRet t_ReturnCode = CoreSdk::GetNumberOfAvailableUsers(t_NumberOfUsers);
	if (t_ReturnCode != EManusRet::Success) return t_ReturnCode;

	p_Users.Reserve(t_NumberOfUsers);
	return CoreSdk::GetIdsOfUsers(p_Users);
}

/// @brief get the current TrackerId's from %Manus Core (if everything is just starting this may be 0 the first second while everything is getting filled in)
EManusRet UManusBlueprintLibrary::GetTrackerIds(TArray<FString>& p_TrackersIds)
{
	return CoreSdk::GetTrackerIds(p_TrackersIds);
}

/// @brief 
EManusRet UManusBlueprintLibrary::AssignTrackerToManusUser(FString& p_TrackerId, int64 p_ManusUserId)
{
	if (p_TrackerId.Len() == 0) return EManusRet::InvalidArgument;

	char* t_Id = TCHAR_TO_ANSI(*p_TrackerId); // TODO update these too for unicode ?

	return CoreSdk::AssignTrackerToUser(t_Id,p_ManusUserId);
}
// todo do we want the other tracker assignment functions too? not a major thing to implement, but probably not needed for 99.9999% of customers as they can use the sdk directly.

/// @brief Retrieves an array containing the glove IDs of all available gloves.
EManusRet UManusBlueprintLibrary::GetIdsOfAvailableGloves(TArray<int64> &GloveIds)
{
	return CoreSdk::GetIdsOfAvailableGloves(GloveIds); // for now ok...
}

/// @brief small support function for ease of use. 
/// returns the first available haptics dongle
/// p1 has a separate haptics dongle. for p2 and up its combined with regular dongle. (p1 haptics have been discontinued. so we will never see those again)
EManusRet UManusBlueprintLibrary::GetFirstHapticDongle(int64& DongleId)
{
	TArray<int64> t_HapticDongleIds;
	EManusRet t_Result = CoreSdk::GetHapticDongleIds(t_HapticDongleIds); // fine for now
	if (t_Result != EManusRet::Success)
	{
		return t_Result;
	}
	if (t_HapticDongleIds.Num() == 0)
	{
		return EManusRet::DataNotAvailable;
	}
	DongleId = t_HapticDongleIds[0];
	return EManusRet::Success;
}


/// @brief Tell a %Manus glove to vibrate its fingers. The first available glove of the given hand type attached to the donlge id will be used.
/// for prime 1 the haptics dongle is a separate dongle id then then regular dongle id. 
/// for prime 2 and up the haptics dongle is the same as the regular dongle.
EManusRet UManusBlueprintLibrary::VibrateFingers(int64 p_DongleId, EManusHandType p_HandType, float p_ThumbPower, float p_IndexPower, float p_MiddlePower, float p_RingPower, float p_PinkyPower)
{
	if (p_DongleId == 0)
	{
		return EManusRet::InvalidArgument;
	}

	TArray<float> t_Powers;
	t_Powers.Add(FMath::Clamp(p_ThumbPower, 0.0f, 1.0f));
	t_Powers.Add(FMath::Clamp(p_IndexPower, 0.0f, 1.0f));
	t_Powers.Add(FMath::Clamp(p_MiddlePower, 0.0f, 1.0f));
	t_Powers.Add(FMath::Clamp(p_RingPower, 0.0f, 1.0f));
	t_Powers.Add(FMath::Clamp(p_PinkyPower, 0.0f, 1.0f));

	return CoreSdk::VibrateFingers(p_DongleId, p_HandType, t_Powers); 
}

/// @brief Tell a %Manus glove to vibrate its fingers. The first available glove of the given hand type associated to the skeleton id will be used.
EManusRet UManusBlueprintLibrary::VibrateFingersForSkeleton(int64 p_SkeletonId, EManusHandType p_HandType, float p_ThumbPower, float p_IndexPower, float p_MiddlePower, float p_RingPower, float p_PinkyPower)
{
	if (p_SkeletonId == 0)
	{
		return EManusRet::InvalidArgument;
	}

	TArray<float> t_Powers;
	t_Powers.Add(FMath::Clamp(p_ThumbPower, 0.0f, 1.0f));
	t_Powers.Add(FMath::Clamp(p_IndexPower, 0.0f, 1.0f));
	t_Powers.Add(FMath::Clamp(p_MiddlePower, 0.0f, 1.0f));
	t_Powers.Add(FMath::Clamp(p_RingPower, 0.0f, 1.0f));
	t_Powers.Add(FMath::Clamp(p_PinkyPower, 0.0f, 1.0f));

	return CoreSdk::VibrateFingersForSkeleton(p_SkeletonId, p_HandType, t_Powers);
}

/// @brief see manusLiveLinkSource.cpp for example on how to setup a skeleton ,its nodes and chains.
/// first setup the structure for a skeleton and get its index (not id)
EManusRet UManusBlueprintLibrary::CreateSkeletonSetup(const FManusSkeletonSetup& p_Skeleton, int64& p_SkeletonSetupIndex)
{
	// convert
	SkeletonSetupInfo t_Setup;

	t_Setup.id = p_Skeleton.Id;

    ManusTools::ConvertFStringToUTF8Array(t_Setup.name, p_Skeleton.Name, MAX_NUM_CHARS_IN_SKELETON_NAME);

	if (p_Skeleton.Type == EManusSkeletonType::Body) t_Setup.type = SkeletonType::SkeletonType_Body;
	else if (p_Skeleton.Type == EManusSkeletonType::Both) t_Setup.type = SkeletonType::SkeletonType_Both;
	else if (p_Skeleton.Type == EManusSkeletonType::Hand) t_Setup.type = SkeletonType::SkeletonType_Hand;
	else t_Setup.type = SkeletonType::SkeletonType_Invalid;

	// convert settings
	t_Setup.settings.scaleToTarget = p_Skeleton.ScaleToTarget;
    t_Setup.settings.useEndPointApproximations = p_Skeleton.UseEndPointApproximations;
	t_Setup.settings.skeletonGloveData.gloveID = (uint32_t)p_Skeleton.SkeletonGloveId;
	t_Setup.settings.skeletonTargetUserData.userID = (uint32_t)p_Skeleton.SkeletonTargetUserId;

	char* t_AnsiName = TCHAR_TO_ANSI(*p_Skeleton.SkeletonTargetAnimationId);
	strcpy_s(t_Setup.settings.skeletonTargetAnimationData.id, t_AnsiName);

	if (p_Skeleton.TargetType == EManusSkeletonTargetType::AnimationData) t_Setup.settings.targetType = SkeletonTargetType::SkeletonTarget_AnimationData;
	else if (p_Skeleton.TargetType == EManusSkeletonTargetType::GloveData) t_Setup.settings.targetType = SkeletonTargetType::SkeletonTarget_GloveData;
	else if (p_Skeleton.TargetType == EManusSkeletonTargetType::UserData) t_Setup.settings.targetType = SkeletonTargetType::SkeletonTarget_UserData;
	else if (p_Skeleton.TargetType == EManusSkeletonTargetType::UserIndexData) t_Setup.settings.targetType = SkeletonTargetType::SkeletonTarget_UserIndexData;
	else t_Setup.settings.targetType = SkeletonTargetType::SkeletonTarget_Invalid;	

	// and call
	uint32_t t_SkeletonSetupIndex = 0;
	EManusRet t_Ret = CoreSdk::CreateSkeletonSetup(t_Setup, t_SkeletonSetupIndex);
	p_SkeletonSetupIndex = t_SkeletonSetupIndex;
	return t_Ret;
}

/// @brief see manusLiveLinkSource.cpp for example on how to setup a skeleton ,its nodes and chains.
/// after setting up the skeleton you need to add the bone nodes with this function
EManusRet UManusBlueprintLibrary::AddNodeToSkeletonSetup(int64 p_SkeletonSetupIndex, const FManusNodeSetup& p_Node)
{
	// convert
	NodeSetup t_Node;
	ManusConvert::BpManusNodeSetupToSDK(t_Node, p_Node);
		
	return CoreSdk::AddNodeToSkeletonSetup((uint32_t)p_SkeletonSetupIndex, t_Node);
}

/// @brief see manusLiveLinkSource.cpp for example on how to setup a skeleton ,its nodes and chains.
/// Manually add a chain to the newly generated skeleton.
EManusRet UManusBlueprintLibrary::AddChainToSkeletonSetup(int64 p_SkeletonSetupIndex, const FManusChainSetup& p_Chain)
{
	// convert
	ChainSetup t_Setting;

	t_Setting.dataIndex = p_Chain.DataIndex;
	t_Setting.dataType = ManusConvert::ChainTypeToSDK(p_Chain.DataType);
	t_Setting.id = (uint32_t)p_Chain.Id;
	t_Setting.nodeIdCount = p_Chain.NodeIds.Num();
	if (MAX_CHAIN_LENGTH < t_Setting.nodeIdCount) return EManusRet::ArgumentSizeMismatch;

	for (size_t i = 0; i < t_Setting.nodeIdCount; i++)
	{
		t_Setting.nodeIds[i] = (uint32_t)p_Chain.NodeIds[i];
	}
	if (p_Chain.Side == EManusSide::Center) t_Setting.side = Side::Side_Center;
	else if (p_Chain.Side == EManusSide::Left) t_Setting.side = Side::Side_Left;
	else if (p_Chain.Side == EManusSide::Right) t_Setting.side = Side::Side_Right;
	else t_Setting.side = Side::Side_Invalid;

	t_Setting.type = ManusConvert::ChainTypeToSDK(p_Chain.Type);

	// settings
	t_Setting.settings.usedSettings = (ChainType)p_Chain.Settings.UsedSettings; // this is a combined flag, so we convert it hard like this. as it is handled like a number.(bitflag)
	switch (t_Setting.settings.usedSettings)
	{
		case ChainType::ChainType_Arm:
		{
			t_Setting.settings.arm.armLengthMultiplier = p_Chain.Settings.Arm.ArmLengthMultiplier;
			t_Setting.settings.arm.elbowRotationOffset = p_Chain.Settings.Arm.ElbowRotationOffset;
			
			t_Setting.settings.arm.armRotationOffset.x = p_Chain.Settings.Arm.ArmRotationOffset.X;
			t_Setting.settings.arm.armRotationOffset.y = p_Chain.Settings.Arm.ArmRotationOffset.Y;
			t_Setting.settings.arm.armRotationOffset.z = p_Chain.Settings.Arm.ArmRotationOffset.Z;
			
			t_Setting.settings.arm.positionMultiplier.x = p_Chain.Settings.Arm.PositionMultiplier.X;
			t_Setting.settings.arm.positionMultiplier.y = p_Chain.Settings.Arm.PositionMultiplier.Y;
			t_Setting.settings.arm.positionMultiplier.z = p_Chain.Settings.Arm.PositionMultiplier.Z;

			t_Setting.settings.arm.positionOffset.x = p_Chain.Settings.Arm.PositionOffset.X;
			t_Setting.settings.arm.positionOffset.y = p_Chain.Settings.Arm.PositionOffset.Y;
			t_Setting.settings.arm.positionOffset.z = p_Chain.Settings.Arm.PositionOffset.Z;

			break;
		}
		case ChainType::ChainType_FingerIndex:
		case ChainType::ChainType_FingerMiddle:
		case ChainType::ChainType_FingerPinky:
		case ChainType::ChainType_FingerRing:
		case ChainType::ChainType_FingerThumb:
		{			
			t_Setting.settings.finger.handChainId = p_Chain.Settings.Finger.HandChainId;
			t_Setting.settings.finger.metacarpalBoneId = p_Chain.Settings.Finger.MetacarpalBoneId;
			t_Setting.settings.finger.useLeafAtEnd = p_Chain.Settings.Finger.UseLeafAtEnd;
			break;
		}
		case ChainType::ChainType_Foot:
		{
			t_Setting.settings.foot.toeChainIdsUsed = p_Chain.Settings.Foot.ToeChainIds.Num();
			for (size_t i = 0; i < t_Setting.settings.foot.toeChainIdsUsed; i++)
			{
				t_Setting.settings.foot.toeChainIds[i] = p_Chain.Settings.Foot.ToeChainIds[i];
			}
			break;
		}
		case ChainType::ChainType_Hand:
		{
			t_Setting.settings.hand.fingerChainIdsUsed = p_Chain.Settings.Hand.FingerChainIds.Num();
			for (size_t i = 0; i < t_Setting.settings.hand.fingerChainIdsUsed; i++)
			{
				t_Setting.settings.hand.fingerChainIds[i] = p_Chain.Settings.Hand.FingerChainIds[i];
			}

			if (p_Chain.Settings.Hand.HandMotion == EManusHandMotion::IMU) t_Setting.settings.hand.handMotion = HandMotion::HandMotion_IMU;
			else if (p_Chain.Settings.Hand.HandMotion == EManusHandMotion::Tracker) t_Setting.settings.hand.handMotion = HandMotion::HandMotion_Tracker;
            else if (p_Chain.Settings.Hand.HandMotion == EManusHandMotion::Tracker_RotationOnly) t_Setting.settings.hand.handMotion = HandMotion::HandMotion_Tracker_RotationOnly;
            else if (p_Chain.Settings.Hand.HandMotion == EManusHandMotion::Auto) t_Setting.settings.hand.handMotion = HandMotion::HandMotion_Auto;
            else t_Setting.settings.hand.handMotion = HandMotion::HandMotion_None;
			break;
		}
		case ChainType::ChainType_Head:
		{
			t_Setting.settings.head.headPitchOffset = p_Chain.Settings.Head.HeadPitchOffset;
			t_Setting.settings.head.headTiltOffset = p_Chain.Settings.Head.HeadTiltOffset;
			t_Setting.settings.head.headYawOffset = p_Chain.Settings.Head.HeadYawOffset;
			t_Setting.settings.head.useLeafAtEnd = p_Chain.Settings.Head.UseLeafAtEnd;
			break;
		}
		case ChainType::ChainType_Leg:
		{
			t_Setting.settings.leg.footForwardOffset = p_Chain.Settings.Leg.FootForwardOffset;
			t_Setting.settings.leg.footSideOffset = p_Chain.Settings.Leg.FootSideOffset;
			t_Setting.settings.leg.kneeRotationOffset = p_Chain.Settings.Leg.KneeRotationOffset;
			t_Setting.settings.leg.reverseKneeDirection = p_Chain.Settings.Leg.ReverseKneeDirection;
			break;
		}
		case ChainType::ChainType_Neck:
		{
			t_Setting.settings.neck.neckBendOffset = p_Chain.Settings.Neck.NeckBendOffset;
			break;
		}
		case ChainType::ChainType_Shoulder:
		{
			t_Setting.settings.shoulder.forwardMultiplier = p_Chain.Settings.Shoulder.ForwardMultiplier;
			t_Setting.settings.shoulder.forwardOffset = p_Chain.Settings.Shoulder.ForwardOffset;
			t_Setting.settings.shoulder.shrugMultiplier = p_Chain.Settings.Shoulder.ShrugMultiplier;
			t_Setting.settings.shoulder.shrugOffset = p_Chain.Settings.Shoulder.ShrugOffset;
			break;
		}
		case ChainType::ChainType_Spine:
		{
			t_Setting.settings.spine.spineBendOffset = p_Chain.Settings.Spine.SpineBendOffset;
			break;
		}
		case ChainType::ChainType_Toe:
		{
			t_Setting.settings.toe.footChainId = p_Chain.Settings.Toe.FootChainId;
			t_Setting.settings.toe.useLeafAtEnd = p_Chain.Settings.Toe.UseLeafAtEnd;
			break;
		}
	}	
	return CoreSdk::AddChainToSkeletonSetup((uint32_t)p_SkeletonSetupIndex, t_Setting);
}

/// @brief see manusLiveLinkSource.cpp for example on how to setup a skeleton ,its nodes and chains.
/// after all nodes and chains have been made for the skeleton, you can now load it into %Manus Core
/// and receive the skeleton ID.
EManusRet UManusBlueprintLibrary::LoadSkeleton(int64 p_SkeletonIndex, int64& p_SkeletonId)
{
	return CoreSdk::LoadSkeleton((uint32_t)p_SkeletonIndex, (uint32_t&)p_SkeletonId);
}

/// @brief for cleaning up, you can unload the skeleton
EManusRet UManusBlueprintLibrary::UnloadSkeleton(int64 p_SkeletonId)
{
	return CoreSdk::UnloadSkeleton(p_SkeletonId);
}

/// @brief Get the latest skeleton data that was received from %Manus Core for the skeleton with the given skeleton ID.
EManusRet UManusBlueprintLibrary::GetSkeletonData(int64 p_SkeletonId, FManusMetaSkeleton& p_Skeleton)
{
	return CoreSdk::GetDataForSkeleton(p_SkeletonId, p_Skeleton);
}

/*	// todo ? not really required on a blueprint lvl. the livelinksource.cpp has a more direct example. and already works with the blueprint.
* to be determined.
	static EManusRet OverwriteChainToSkeletonSetup(uint32_t p_SkeletonIndex, const ChainSetup& p_Chain);
	static EManusRet GetSkeletonSetupArraySizes(uint32_t p_SkeletonSetupIndex, SkeletonSetupArraySizes& p_SkeletonInfo);
	allocatechains?
*/

/// @brief Get the latest Tracker data that was received from %Manus Core for the Tracker assigned to the given livelinkUser Index for the hand of the given type.
EManusRet UManusBlueprintLibrary::GetHandTrackerData(int p_ManusLiveLinkUserIndex, EManusHandType p_HandTypeOfTracker, FManusTracker& p_Tracker)
{
	const TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	if (!ManusLiveLinkUsers.IsValidIndex(p_ManusLiveLinkUserIndex))
	{
		return EManusRet::InvalidArgument;
	}
	
	int32 t_NumberOfUsers = 0;
	EManusRet t_Ret = CoreSdk::GetNumberOfAvailableUsers(t_NumberOfUsers); 
	if (t_Ret != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to get user count."));
		return t_Ret;
	}
	if ((t_NumberOfUsers == 0) ||
		(t_NumberOfUsers < ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].ManusDashboardUserIndex))
	{
		// this is normal during startup as no landscape data has yet been processed
		return EManusRet::DataNotAvailable;
	} 
	
	TArray<int64> t_IdsOfAvailableUsers;
	t_IdsOfAvailableUsers.Reserve(t_NumberOfUsers);
	t_Ret = CoreSdk::GetIdsOfUsers(t_IdsOfAvailableUsers);
	if (t_Ret != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to get users."));
		return t_Ret;
	}
	uint32_t t_UserId = t_IdsOfAvailableUsers[ManusLiveLinkUsers[p_ManusLiveLinkUserIndex].ManusDashboardUserIndex];

	t_Ret = CoreSdk::GetDataForTracker(t_UserId, p_HandTypeOfTracker, p_Tracker);
	if (t_Ret != EManusRet::Success)
	{
		// An error message will be logged in the function, so don't print anything here.
		return t_Ret;
	}

	return EManusRet::Success;
}

/// @brief 
FString UManusBlueprintLibrary::GetPlayerJoinRequestURL(APlayerController* Controller)
{
	if (Controller && Controller->GetNetConnection())
	{
		return Controller->GetNetConnection()->RequestURL;
	}
	return "";
}

/// @brief Get the latest skeleton data that was received from %Manus Core for the skeleton with the given skeleton ID.
EManusRet UManusBlueprintLibrary::GetErgonomics(FManusErgonomicsData& p_Data, int64 p_GloveId)
{
	return CoreSdk::GetErgonomicsDataForGlove(p_Data, (uint32_t)p_GloveId);
}

EManusRet UManusBlueprintLibrary::GetGloveIdOfUser_UsingUserIndex(int32 UserIndex, EManusHandType p_HandTypeOfGlove, int64& p_GloveId)
{
    return CoreSdk::GetGloveIdOfUser_UsingUserIndex( UserIndex,  p_HandTypeOfGlove,  p_GloveId);
}

EManusRet UManusBlueprintLibrary::GetGloveIdOfUser_UsingUserId(int32 UserId, EManusHandType p_HandTypeOfGlove, int64& p_GloveId)
{
    return CoreSdk::GetGloveIdOfUser_UsingUserId( UserId,  p_HandTypeOfGlove,  p_GloveId);
}

/// @brief override for demo to let gloves use trackers
void UManusBlueprintLibrary::SetGlovesUsingTrackers(bool p_UseTrackers)
{
    FManusModule::Get().SetGlovesUsingTrackers(p_UseTrackers);
}