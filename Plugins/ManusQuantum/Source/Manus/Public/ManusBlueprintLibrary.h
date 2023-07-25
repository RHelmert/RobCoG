// Copyright 2015-2022 Manus

#pragma once

// Set up a Doxygen group.
/** @addtogroup ManusBlueprintLibrary
 *  @{
 */

#include "ManusBlueprintTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ManusBlueprintLibrary.generated.h"

UCLASS()
class MANUS_API UManusBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/// @brief Acticate / deactivate Manus tracking.
	/// @param  bNewIsActive		Whether Manus tracking should be active or not.
	UFUNCTION(BlueprintCallable, Category = Manus)
	static void SetManusActive(bool bNewIsActive);

	/// @brief Get all manus user ID's and store a copy of the ID's in p_Users
    /// @param  p_Users					
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetManusUsers(TArray<int64>& p_Users);

	/// @brief Get All tracker ID's and store a copy of the ID's in p_TrackersIds
    /// @param  p_TrackersIds					
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetTrackerIds(TArray<FString>& p_TrackersIds);

	/// @brief assign a tracker based on its ID to a user based on its ID.
    /// @param  p_TrackerId					
    /// @param  p_ManusUserId					
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet AssignTrackerToManusUser(FString& p_TrackerId, int64 p_ManusUserId);

    /// @brief Convert the given Manus ID to an FString.
	/// @param  GloveId The Manus ID that should be converted.
	/// @return The FString containing the converted Manus ID.
	UFUNCTION(BlueprintPure, Category = Manus)
	static FString ConvertManusIdToString(int64 ManusId);

    /// @brief Retrieves an array containing the glove IDs of all available gloves.
	/// These IDs can be used for functions ending in _UsingId, which will only have an effect on the specified glove.
	/// @param  GloveIds  The array of glove IDs.
    /// @return If the function succeeded in getting the glove IDs.
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetIdsOfAvailableGloves(TArray<int64> &GloveIds);

    /// @brief set the first haptic dongle id in p_DongleId. 
    /// This may get deprecated soon.
    /// @param  DongleId 
	UFUNCTION(BlueprintCallable, Category = Manus)
	static EManusRet GetFirstHapticDongle(int64& DongleId);

    /// @brief Tell a Manus glove to vibrate its fingers. The first available glove of the given hand type will be used.
	/// @param  p_DongleId	The haptics dongle id of the glove to look for.
	/// @param  HandType		The hand type of the glove to look for.
	/// @param  ThumbPower   The strength of the vibration for the thumb, between 0.0 and 1.0.
	/// @param  IndexPower   The strength of the vibration for the index, between 0.0 and 1.0.
    /// @param  MiddlePower  The strength of the vibration for the middle finger, between 0.0 and 1.0.
	/// @param  RingPower    The strength of the vibration for the ring finger, between 0.0 and 1.0.
	/// @param  PinkyPower   The strength of the vibration for the pinky finger, between 0.0 and 1.0.
    /// @return If the glove was succesfully told to vibrate.
	UFUNCTION(BlueprintCallable, Category = Manus)
	static EManusRet VibrateFingers(int64 p_DongleId,EManusHandType HandType, float ThumbPower = 1.0f, float IndexPower = 1.0f, float MiddlePower = 1.0f, float RingPower = 1.0f, float PinkyPower = 1.0f);

    /// @brief Tell a Manus glove associated to the given skeleton id to vibrate its fingers.
	/// @param  p_SkeletonId	The skeleton id associated to the glove to look for.
    /// @param  HandType		The hand type of the glove to look for.
	/// @param  ThumbPower   The strength of the vibration for the thumb, between 0.0 and 1.0.
	/// @param  IndexPower   The strength of the vibration for the index, between 0.0 and 1.0.
	/// @param  MiddlePower  The strength of the vibration for the middle finger, between 0.0 and 1.0.
	/// @param  RingPower    The strength of the vibration for the ring finger, between 0.0 and 1.0.
	/// @param  PinkyPower   The strength of the vibration for the pinky finger, between 0.0 and 1.0.
	/// @return If the glove was succesfully told to vibrate.
	UFUNCTION(BlueprintCallable, Category = Manus)
	static EManusRet VibrateFingersForSkeleton(int64 p_SkeletonId, EManusHandType HandType, float ThumbPower = 1.0f, float IndexPower = 1.0f, float MiddlePower = 1.0f, float RingPower = 1.0f, float PinkyPower = 1.0f);

    /// @brief Get the latest skeleton data that was received from Manus for the skeleton with the given ID.
	/// @param  SkeletonId				The ID of the skeleton to get the data from.
	/// @param  Skeleton		Output		The latest skeleton data for the connected skeleton.
    /// @return If the skeleton data were successfuly retrieved.
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetSkeletonData(int64 SkeletonId, FManusMetaSkeleton& Skeleton);

    /// @brief create a skeleton structure and receive the skeleton index for later reference.
    /// @param  p_Skeleton				The Skeleton Setup data
	/// @param  p_SkeletonSetupIndex		Output		The index of the skeleton 
	/// @return If the skeleton was successfuly created.
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet CreateSkeletonSetup(const FManusSkeletonSetup& p_Skeleton, int64& p_SkeletonSetupIndex);

    /// @brief add a node to the skeleton structure 
	/// @param  p_SkeletonSetupIndex		The Skeleton index
	/// @param  p_Node					The node data
	/// @return If the node was successfuly added.
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet AddNodeToSkeletonSetup(int64 p_SkeletonSetupIndex, const FManusNodeSetup& p_Node);

    /// @brief add a chain to the skeleton structure
	/// @param  p_SkeletonSetupIndex		The Skeleton index
	/// @param  p_Chain					The node data
	/// @return If the node was successfuly added.
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet AddChainToSkeletonSetup(int64 p_SkeletonSetupIndex, const FManusChainSetup& p_Chain);

    /// @brief load the created skeleton into %Manus Core (important or nothing gets animated)
	/// @param  p_SkeletonIndex						The temporary skeleton index as defined in the SDK
	/// @param  p_SkeletonId				output		the skeleton id as generated by %Manus Core
    /// @return If the skeleton was succesfully loaded.
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet LoadSkeleton(int64 p_SkeletonIndex, int64& p_SkeletonId);

    /// @brief unload the created skeleton from %Manus Core 
	/// @param  p_SkeletonId						The Skeleton id
    /// @return If the skeleton was succesfully loaded.
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet UnloadSkeleton(int64 p_SkeletonId);


    /// @brief Get the latest Tracker data that was received from %Manus Core for the Tracker assigned to the given User Index for the hand of the given type.
	/// @param  ManusLiveLinkUserIndex	The index of the Manus Live Link User.
	/// @param  HandTypeOfTracker		The hand type of the glove.
	/// @param  Tracker			Output	The latest Tracker data.
    /// @return If the Tracker data were successfuly retrieved.
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetHandTrackerData(int p_ManusLiveLinkUserIndex, EManusHandType p_HandTypeOfTracker, FManusTracker& p_Tracker);

    /// @brief Returns the Player join request URL.
	/// @param  Controller	The Player Controller from which we want the join request URL.
	/// @return The Player join request URL.
	UFUNCTION(BlueprintCallable, Category = Manus)
	static FString GetPlayerJoinRequestURL(APlayerController* Controller);

    /// @brief gets ergonomics data
	/// @param  p_Data	The ergonomics data.
	/// @param  p_GloveId	The glove id of the ergonomics data we want.
	/// @returns success state
	UFUNCTION(BlueprintCallable, Category = Manus)
	static EManusRet GetErgonomics(FManusErgonomicsData& p_Data, int64 p_GloveId);

    /// @brief get glove id of user using user index and hand type.
    /// @param UserIndex 
    /// @param p_HandTypeOfGlove 
    /// @param p_GloveId 
    /// @return returncode for success state
    UFUNCTION(BlueprintCallable, Category = Manus)
    static EManusRet GetGloveIdOfUser_UsingUserIndex(int32 UserIndex, EManusHandType p_HandTypeOfGlove, int64& p_GloveId);
    /// @brief get glove id of user using user id and handtype
    /// @param UserId 
    /// @param p_HandTypeOfGlove 
    /// @param p_GloveId 
    /// @return returncode for success state
    UFUNCTION(BlueprintCallable, Category = Manus)
    static EManusRet GetGloveIdOfUser_UsingUserId(int32 UserId, EManusHandType p_HandTypeOfGlove, int64& p_GloveId);


    /// @brief set if this session is as a Client or not
    UFUNCTION(BlueprintCallable, Category = Manus)
    static void SetAsClient(bool p_IsClient);

    /// @brief set the %Manus Core host address to connect to
	/// @param  p_ManusIP The new %Manus Core host address to connect to.
	UFUNCTION(BlueprintCallable, Category = Manus)
	static void SetManusCoreIP(FString p_ManusIP);

    /// @brief get all the %Manus Core hosts that can be connected to
    /// @returns TArray of all available %Manus Core host addresses.
	UFUNCTION(BlueprintCallable, Category = Manus)
	static TArray<FString> GetManusCoreIPs();

    /// @brief for demo purposes only. set that the gloves use trackers.
	/// @param  p_UseTrackers 
    UFUNCTION(BlueprintCallable, Category = Manus)
    static void SetGlovesUsingTrackers(bool p_UseTrackers);
};

// Close the Doxygen group.
/** @} */

