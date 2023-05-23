// Copyright 2015-2020 Manus

#pragma once

#include "ManusBlueprintTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ManusBlueprintLibrary.generated.h"

UCLASS()
class MANUS_API UManusBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/* Acticate / deactivate Manus tracking.
	 * @param  bNewIsActive		Whether Manus tracking should be active or not.
	 */
	UFUNCTION(BlueprintCallable, Category = Manus)
	static void SetManusActive(bool bNewIsActive);

	/**
	 * Returns the glove ID of the given Manus Dashboard User.
	 * @param  ManusDashboardUserIndex	The index of the Manus Dashboard User to get the glove ID from.
	 * @param  HandType					The hand type of the glove to get the glove ID from.
	 * @param  GloveId					Output: The glove ID of the given Manus Dashboard User (0 if the ID is invalid).
	 * @return If glove ID was successfully retrieved.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetManusDashboardUserGloveId(int ManusDashboardUserIndex, EManusHandType HandType, int64& GloveId);

	/**
	 * Convert the given Manus ID to a string.
	 * @param  GloveId The Manus ID that should be converted.
	 * @return The string containing the converted Manus ID.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static FString ConvertManusIdToString(int64 ManusId);

	/**
	 * Check if the Manus glove with the given ID is available.
	 * @param  GloveId     The ID of the glove to do the check for.
	 * @param  IsAvailable Output: if glove data is available.
	 * @return If the check was successfully done.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet IsGloveDataAvailable(int64 GloveId, bool &IsAvailable);

	/**
	 * Get the latest glove data that was received from Manus for the Manus glove with the given ID.
	 * @param  GloveId   The ID of the glove that glove data should be retrieved for.
	 * @param  GloveData Output: the latest glove data for the glove.
	 * @return If glove data was successfully retrieved.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetGloveData(int64 GloveId, FManusGlove &GloveData);

	/**
	 * Get the latest glove finger data that was received for the Manus glove with the given ID.
	 * @param  GloveId  The ID of the glove that finger data should be retrieved for.
	 * @param  Finger	The finger we want the processed data for.
	 * @param  FingerData Output: the latest finger data for the glove.
	 * @return If finger data was successfully retrieved.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetFingerData(int64 GloveId, EManusFingerName Finger, FManusFingerData& FingerData);

	/**
	 * Get the latest hand orientation based on the glove IMUs (inertial measurement units).
	 * @param  GloveId  The ID of the glove.
	 * @param  Orientation Output: the orientation of the hand determined by the glove IMUs (inertial measurement units).
	 * @return If the orientation was successfully retrieved.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetHandOrientation(int64 GloveId, FRotator& Orientation);

	/**
	 * Retrieves an array containing the glove IDs of all available gloves.
	 * These IDs can be used for functions ending in _UsingId, which will only have an effect on the specified glove.
	 * @param  GloveIds  The array of glove IDs.
	 * @return If the function succeeded in getting the glove IDs.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetIdsOfAvailableGloves(TArray<int64> &GloveIds);

	/**
	 * Get the glove ID of the first available Manus glove with the given hand type.
	 * @param  HandType  The hand type of the glove to look for.
	 * @param  GloveId   Output: the ID of the glove that was found.
	 * @return If the function succeeded in finding a glove ID.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetIdOfFirstAvailableGlove(EManusHandType HandType, int64 &GloveId);

	/**
	 * Get the ID of the dongle that the Manus glove with the given ID is paired to.
	 * @param  GloveId  The glove ID of the Manus glove that the paired dongle ID should be retrieved for.
	 * @param  DongleId Output: the ID of the dongle the first connected device with the given hand type is paired to.
	 * @return If the device ID of the dongle was successfully retrieved.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetIdOfDongleGloveIsConnectedTo(int64 GloveId, int64 &DongleId);

	/**
	 * Get the hand type of the glove with the given glove ID.
	 * @param GloveId         The glove ID of the glove to get the hand type of.
	 * @param HandTypeOfGlove The hand type that the glove with the given glove ID has.
	 * return If the hand type of the glove was successfully retrieved.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetHandTypeOfGlove(int64 GloveId, EManusHandType &HandTypeOfGlove);

	/**
	 * Get the battery level of the Manus glove with the given ID, in percent.
	 * @param  GloveId           The glove ID of the glove that the battery level should be retrieved for.
	 * @param  BatteryPercentage Output: the batterly level of the glove, in precent.
	 * @return If the battery level was successfully retrieved.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetBatteryPercentage(int64 GloveId, int32 &BatteryPercentage);

	/**
	 * Get the transmission strength of the Manus glove with the given ID, in dB.
	 * @param  GloveId                  The glove ID of the glove that the transmission strength should be retrieved for.
	 * @param  TransmissionStrengthInDb Output: the transmission strength of the glove, in decibel.
	 * @return If the transmission strength was successfully retrieved.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetTransmissionStrength(int64 GloveId, int32 &TransmissionStrengthInDb);

	/**
	 * Tell a Manus glove to vibrate. The glove with the given ID will be used.
	 * @param  GloveId      The ID of the target glove.
	 * @param  Power        The strength of the vibration, between 0.0 and 1.0.
	 * @param  Milliseconds The number of milliseconds the glove should rumble for.
	 * @return If the glove was succesfully told to vibrate.
	 */
	UFUNCTION(BlueprintCallable, Category = Manus)
	static EManusRet VibrateGlove(int64 GloveId, float Power = 0.6f, int32 Milliseconds = 300);

	/**
	 * Tell a Manus glove to vibrate its fingers. The first available glove of the given hand type will be used.
	 * @param  HandType		The hand type of the glove to look for.
	 * @param  ThumbPower   The strength of the vibration for the thumb, between 0.0 and 1.0.
	 * @param  IndexPower   The strength of the vibration for the index, between 0.0 and 1.0.
	 * @param  MiddlePower  The strength of the vibration for the middle finger, between 0.0 and 1.0.
	 * @param  RingPower    The strength of the vibration for the ring finger, between 0.0 and 1.0.
	 * @param  PinkyPower   The strength of the vibration for the pinky finger, between 0.0 and 1.0.
	 * @return If the glove was succesfully told to vibrate.
	 */
	UFUNCTION(BlueprintCallable, Category = Manus)
	static EManusRet VibrateFingers(EManusHandType HandType, float ThumbPower = 1.0f, float IndexPower = 1.0f, float MiddlePower = 1.0f, float RingPower = 1.0f, float PinkyPower = 1.0f);
	
	/**
	 * Returns the Polygon skeleton ID of the given Manus Live Link User.
	 * @param  ManusLiveLinkUserIndex   The index of the Manus Live Link User.
	 * @return The ID of the Polygon skeleton of the given Manus Live Link User.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	int64 GetManusLiveLinkUserPolygonSkeletonId(int ManusLiveLinkUserIndex);

	/**
	 * Get the Polygon skeleton ID of the first available Polygon skeleton.
	 * @param  PolygonSkeletonId   Output: the ID of the Polygon skeleton that was found.
	 * @return If the function succeeded in finding a Polygon skeleton ID.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetFirstPolygonSkeletonId(int64& PolygonSkeletonId);

	/**
	 * Retrieves an array containing the Polygon skeleton IDs of all Polygon skeletons.
	 * These IDs can be used to retrieve data of a specific Polygon skeleton.
	 * @param  PolygonSkeletonIds  The array of Polygon skeleton IDs.
	 * @return If the function succeeded in getting the Polygon skeleton IDs.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetIdsOfPolygonSkeletons(TArray<int64>& PolygonSkeletonIds);

	/**
	 * Get the latest Polygon skeleton data that was received from Manus for the Polygon skeleton with the given ID.
	 * @param  PolygonSkeletonId				The ID of the Polygon skeleton to get the data from.
	 * @param  PolygonSkeleton		Output		The latest Polygon skeleton data for the first connected Polygon skeleton.
	 * @return If the Polygon skeleton data were successfuly retrieved.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetPolygonSkeletonData(int64 PolygonSkeletonId, FManusPolygonSkeleton& PolygonSkeleton);

	/**
	 * Get the latest Tracker data that was received from Manus Core for the Tracker assigned to the given User Index for the hand of the given type.
	 * @param  ManusLiveLinkUserIndex	The index of the Manus Live Link User.
	 * @param  HandTypeOfTracker		The hand type of the glove.
	 * @param  Tracker			Output	The latest Tracker data.
	 * @return If the Tracker data were successfuly retrieved.
	 */
	UFUNCTION(BlueprintPure, Category = Manus)
	static EManusRet GetTrackerData(int ManusLiveLinkUserIndex, EManusHandType HandTypeOfTracker, FManusTracker& Tracker);

	/* Returns the Player join request URL.
	 * @param  Controller	The Player Controller from which we want the join request URL.
	 * @return The Player join request URL.
	 */
	UFUNCTION(BlueprintCallable, Category = Manus)
	static FString GetPlayerJoinRequestURL(APlayerController* Controller);
};
