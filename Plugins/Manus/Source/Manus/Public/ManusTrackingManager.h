// Copyright 2015-2020 Manus

#pragma once

#include <openvr.h>
#include "ManusBlueprintTypes.h"
#include "ManusTrackingManager.generated.h"

constexpr int32 MAX_NUM_STEAM_VR_DEVICES = static_cast<int32>(vr::k_unMaxTrackedDeviceCount);

/**
 * The Manus Tracking Manager handles everything related to Steam VR tracking for the Manus plugin.
 */
UCLASS()
class MANUS_API UManusTrackingManager : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	////////////////////////////////////////////////////////////////////////////
	// Public functions

	UManusTrackingManager();
	~UManusTrackingManager();

	/** UObjectBase interface */
	virtual UWorld* GetWorld() const override;

	/** FTickableGameObject interface */
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickableWhenPaused() const override;
	virtual bool IsTickableInEditor() const override;

	/**
	  Initializes the Tracing Manager.
	 */
	void Initialize();

	/**
	  Get the position and rotation of the tracking device tracking the given body part.
	  @param BodyPart The body part to get the position and rotation for.
	  @param Position Output parameter populated with the position of the tracking device tracking the given body part.
	  @param Rotation Output parameter populated with the rotation of the tracking device tracking the given body part.
	  @return True if a valid position and rotation were set.
	 */
	bool GetTrackingDevicePositionAndRotation(EManusTrackingBodyPart BodyPart, FVector& Position, FRotator& Rotation);

	/**
	  Get the position and rotation of the tracking device tracking the given body part.
	  @param DeviceId The device ID to get the position and rotation from.
	  @param Position Output parameter populated with the position of the tracking device.
	  @param Rotation Output parameter populated with the rotation of the tracking device.
	  @return True if a valid position and rotation were set.
	*/
	bool GetTrackingDevicePositionAndRotation(FName DeviceId, FVector& Position, FRotator& Rotation);

	/**
	  Get the device ID of the first tracking device tracking the given body part.
	  @param BodyPart The body part to get the device ID for.
	  @param DeviceId Output The device ID of the first tracking device tracking the given body part (if any).
	  @return True if a device ID was found.
	*/
	bool GetFirstTrackingDeviceId(EManusTrackingBodyPart BodyPart, FName& DeviceId);

	/**
	  Get the device IDs of the all the tracking devices tracking the given body part.
	  @param BodyPart The body part to get the device IDs for.
	  @param DeviceIds Output The array of device IDs of the tracking device tracking the given body part.
	*/
	void GetTrackingDeviceIds(EManusTrackingBodyPart BodyPart, TArray<FName>& DeviceIds);

private:
	////////////////////////////////////////////////////////////////////////////
	// Types

	/**
	 * A structure that stores information on a single trackable device.
	 */
	struct TrackingDeviceInfo
	{
		bool bIsAvailable = false;
		bool bWasAtSomePointAvailable = false;

		FName DeviceId;
		EManusTrackingDeviceType Type;
		EManusTrackingBodyPart BodyPart;
	};


	////////////////////////////////////////////////////////////////////////////
	// Private functions

	void DoTrackingDeviceUpdate();
	void UpdateViveTrackers();


	////////////////////////////////////////////////////////////////////////////
	// Private data

	TrackingDeviceInfo TrackingDevices[MAX_NUM_STEAM_VR_DEVICES];
	float TrackingDeviceUpdateTimer;
};
