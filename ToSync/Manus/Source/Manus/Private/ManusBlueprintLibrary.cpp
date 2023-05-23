// Copyright 2015-2020 Manus

#include "ManusBlueprintLibrary.h"
#include "Manus.h"
#include "ManusComponent.h"
#include "ManusTools.h"
#include "ManusSkeleton.h"
#include "ManusLiveLinkSource.h"
#include "CoreSdk.h"
#include <sstream>
#include "GameFramework/PlayerController.h"
#include "Engine/NetConnection.h"


// Acticate / deactivate Manus tracking.
void UManusBlueprintLibrary::SetManusActive(bool bNewIsActive)
{
	FManusModule::Get().SetActive(bNewIsActive);
}

// Returns the Glove ID of the given Manus Dashboard User.
EManusRet UManusBlueprintLibrary::GetManusDashboardUserGloveId(int ManusDashboardUserIndex, EManusHandType HandType, int64& GloveId)
{
	// Get the ID from the Manus Dashboard User glove assignment cache
	TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
	if (ManusLocalLiveLinkSource.IsValid())
	{
		GloveId = ManusLocalLiveLinkSource->GetCachedGloveAssignment(ManusDashboardUserIndex, HandType);
	}

	return EManusRet::Success;
}

// Convert the given Manus ID to a string.
FString UManusBlueprintLibrary::ConvertManusIdToString(int64 ManusId)
{
	uint32_t ManusIdAsUint32 = ManusId;

	std::ostringstream Stream;
	Stream << std::hex << ManusIdAsUint32;

	return FString(Stream.str().c_str());
}

// Check if the Manus glove with the given ID is available.
EManusRet UManusBlueprintLibrary::IsGloveDataAvailable(int64 GloveId, bool &IsAvailable)
{
	IsAvailable = false;

	TArray<int64> GloveIds;
	EManusRet Result = CoreSdk::GetIdsOfAvailableGloves(GloveIds);
	if (Result != EManusRet::Success)
	{
		// An error message will be logged in the function, so don't print anything here.

		return Result;
	}

	for (int64 Id : GloveIds)
	{
		if (Id == GloveId)
		{
			IsAvailable = true;

			break;
		}
	}

	return EManusRet::Success;
}

// Get the latest glove data that was received from Manus Core for the Manus glove with the given ID.
EManusRet UManusBlueprintLibrary::GetGloveData(int64 GloveId, FManusGlove &GloveData)
{
	return CoreSdk::GetDataForGlove_UsingGloveId(GloveId, GloveData);
}
// Get the latest glove finger data that was received for the Manus glove with the given ID.
EManusRet UManusBlueprintLibrary::GetFingerData(int64 GloveId, EManusFingerName Finger, FManusFingerData& FingerData)
{
	EManusRet ReturnCode;
	FManusGlove GloveData;
	ReturnCode = CoreSdk::GetDataForGlove_UsingGloveId(GloveId, GloveData);
	if (ReturnCode == EManusRet::Success)
	{
		FingerData.FirstJointStretch = GloveData.Fingers[(int)Finger].Joints[0].Stretch;
		FingerData.SecondJointStretch = GloveData.Fingers[(int)Finger].Joints[1].Stretch;
		FingerData.ThirdJointStretch = GloveData.Fingers[(int)Finger].Joints[2].Stretch;
		FingerData.Spread = GloveData.Fingers[(int)Finger].Spread;
	}
	return ReturnCode;
}

// Get the latest hand orientation based on the glove IMUs(inertial measurement units).
EManusRet UManusBlueprintLibrary::GetHandOrientation(int64 GloveId, FRotator& Orientation)
{
	EManusRet ReturnCode;
	FManusGlove GloveData;
	ReturnCode = CoreSdk::GetDataForGlove_UsingGloveId(GloveId, GloveData);
	if (ReturnCode == EManusRet::Success)
	{
		Orientation = GloveData.WristImuOrientation.Rotator();
	}
	return ReturnCode;
}

// Retrieves an array containing the glove IDs of all available gloves.
EManusRet UManusBlueprintLibrary::GetIdsOfAvailableGloves(TArray<int64> &GloveIds)
{
	return CoreSdk::GetIdsOfAvailableGloves(GloveIds);
}

// Get the glove ID of the first available Manus glove with the given hand type.
EManusRet UManusBlueprintLibrary::GetIdOfFirstAvailableGlove(EManusHandType HandType, int64 &GloveId)
{
	return CoreSdk::GetIdOfFirstAvailableGloveOfType(HandType, GloveId);
}

// Get the ID of the dongle that the Manus glove with the given ID is paired to.
EManusRet UManusBlueprintLibrary::GetIdOfDongleGloveIsConnectedTo(int64 GloveId, int64 &DongleId)
{
	FManusGlove GloveData;
	EManusRet Result = CoreSdk::GetDataForGlove_UsingGloveId(GloveId, GloveData);
	if (Result != EManusRet::Success)
	{
		// An error message will be logged in the function, so don't print anything here.

		return Result;
	}

	DongleId = GloveData.GloveInfo.DongleId;

	return EManusRet::Success;
}

// Get the hand type of the glove with the given glove ID.
EManusRet UManusBlueprintLibrary::GetHandTypeOfGlove(int64 GloveId, EManusHandType &HandTypeOfGlove)
{
	return CoreSdk::GetHandTypeOfGlove(GloveId, HandTypeOfGlove);
}

// Get the battery level of the Manus glove with the given ID, in percent.
EManusRet UManusBlueprintLibrary::GetBatteryPercentage(int64 GloveId, int32 &BatteryPercentage)
{
	FManusGlove GloveData;
	EManusRet Result = CoreSdk::GetDataForGlove_UsingGloveId(GloveId, GloveData);
	if (Result != EManusRet::Success)
	{
		// An error message will be logged in the function, so don't print anything here.

		return Result;
	}

	if (!GloveData.GloveInfo.ReceivedLowFrequencyData)
	{
		return EManusRet::DataNotAvailable;
	}

	BatteryPercentage = GloveData.GloveInfo.BatteryPercentage;

	return EManusRet::Success;
}

// Get the transmission strength of the Manus glove with the given ID, in dB.
EManusRet UManusBlueprintLibrary::GetTransmissionStrength(int64 GloveId, int32 &TransmissionStrengthInDb)
{
	FManusGlove GloveData;
	EManusRet Result = CoreSdk::GetDataForGlove_UsingGloveId(GloveId, GloveData);
	if (Result != EManusRet::Success)
	{
		// An error message will be logged in the function, so don't print anything here.

		return Result;
	}

	if (!GloveData.GloveInfo.ReceivedLowFrequencyData)
	{
		return EManusRet::DataNotAvailable;
	}

	TransmissionStrengthInDb = GloveData.GloveInfo.TransmissionStrengthInDb;

	return EManusRet::Success;
}

// Tell a Manus glove to vibrate. The glove with the given ID will be used.
EManusRet UManusBlueprintLibrary::VibrateGlove(int64 GloveId, float Power /*= 0.6f*/, int32 Milliseconds /*= 300*/)
{
	return CoreSdk::VibrateWristOfGlove(GloveId, Power, Milliseconds);
}

// Tell a Manus glove to vibrate its fingers. The first available glove of the given hand type will be used.
EManusRet UManusBlueprintLibrary::VibrateFingers(EManusHandType HandType, float ThumbPower, float IndexPower, float MiddlePower, float RingPower, float PinkyPower)
{
	TArray<int64> HapticDongleIds;
	EManusRet Result = CoreSdk::GetHapticDongleIds(HapticDongleIds);
	if (Result != EManusRet::Success)
	{
		return Result;
	}
	if (HapticDongleIds.Num() == 0)
	{
		return EManusRet::DataNotAvailable;
	}

	TArray<float> Powers;
	Powers.Add(FMath::Clamp(ThumbPower, 0.0f, 1.0f));
	Powers.Add(FMath::Clamp(IndexPower, 0.0f, 1.0f));
	Powers.Add(FMath::Clamp(MiddlePower, 0.0f, 1.0f));
	Powers.Add(FMath::Clamp(RingPower, 0.0f, 1.0f));
	Powers.Add(FMath::Clamp(PinkyPower, 0.0f, 1.0f));

	return CoreSdk::VibrateFingers(HapticDongleIds[0], HandType, Powers);
}

// Returns the Polygon skeleton ID of the given Manus Live Link User.
int64 UManusBlueprintLibrary::GetManusLiveLinkUserPolygonSkeletonId(int ManusLiveLinkUserIndex)
{
	return ManusTools::GenerateManusIdFromManusLiveLinkUser(ManusLiveLinkUserIndex);
}

// Get the Polygon skeleton ID of the first available Polygon skeleton.
EManusRet UManusBlueprintLibrary::GetFirstPolygonSkeletonId(int64& PolygonSkeletonId)
{
	return CoreSdk::GetIdOfFirstAvailablePolygonSkeleton(PolygonSkeletonId);
}

// Retrieves an array containing the Polygon skeleton IDs of all Polygon skeletons.
EManusRet UManusBlueprintLibrary::GetIdsOfPolygonSkeletons(TArray<int64>& PolygonSkeletonIds)
{
	return CoreSdk::GetIdsOfAvailablePolygonSkeletons(PolygonSkeletonIds);
}

// Get the latest Polygon skeleton data that was received from Manus Core for the Polygon skeleton with the given ID.
EManusRet UManusBlueprintLibrary::GetPolygonSkeletonData(int64 PolygonSkeletonId, FManusPolygonSkeleton& PolygonSkeleton)
{
	return CoreSdk::GetDataForPolygonSkeleton(PolygonSkeletonId, PolygonSkeleton);
}

// Get the latest Tracker data that was received from Manus Core for the Tracker assigned to the given User Index for the hand of the given type.
EManusRet UManusBlueprintLibrary::GetTrackerData(int ManusLiveLinkUserIndex, EManusHandType HandTypeOfTracker, FManusTracker& Tracker)
{
	const TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	if (!ManusLiveLinkUsers.IsValidIndex(ManusLiveLinkUserIndex))
	{
		return EManusRet::InvalidArgument;
	}

	EManusRet Result = CoreSdk::GetDataForTracker(ManusLiveLinkUsers[ManusLiveLinkUserIndex].ManusDashboardUserIndex, HandTypeOfTracker, Tracker);
	if (Result != EManusRet::Success)
	{
		// An error message will be logged in the function, so don't print anything here.
		return Result;
	}

	return EManusRet::Success;
}

FString UManusBlueprintLibrary::GetPlayerJoinRequestURL(APlayerController* Controller)
{
	if (Controller && Controller->GetNetConnection())
	{
		return Controller->GetNetConnection()->RequestURL;
	}
	return "";
}
