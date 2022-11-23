// Copyright 2015-2020 Manus

#include "ManusTrackingManager.h"
#include "Manus.h"
#include "ManusSettings.h"
#include "Engine/World.h"
#ifdef MANUS_PLUGIN_USE_STEAMVR
#include "ISteamVRPlugin.h"
#endif //MANUS_PLUGIN_USE_STEAMVR


UManusTrackingManager::UManusTrackingManager()
{
	TrackingDeviceUpdateTimer = 0.0f;
}

UManusTrackingManager::~UManusTrackingManager()
{

}

UWorld* UManusTrackingManager::GetWorld() const
{
	return Cast<UWorld>(GetOuter());
}

void UManusTrackingManager::Tick(float DeltaTime)
{
	// Update Tracking Device Update timer
	if (TrackingDeviceUpdateTimer > 0.0f)
	{
		TrackingDeviceUpdateTimer -= DeltaTime;
		if (TrackingDeviceUpdateTimer <= 0.0f)
		{
			DoTrackingDeviceUpdate();
		}
	}
}

bool UManusTrackingManager::IsTickable() const
{
	return HasAnyFlags(RF_ClassDefaultObject) == false;
}

TStatId UManusTrackingManager::GetStatId() const
{
	return GetStatID(false);
}

ETickableTickType UManusTrackingManager::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

bool UManusTrackingManager::IsTickableWhenPaused() const
{
	return true;
}

bool UManusTrackingManager::IsTickableInEditor() const
{
	return true;
}

void UManusTrackingManager::DoTrackingDeviceUpdate()
{
	// Update Vive Trackers
	UpdateViveTrackers();

	// Restart timer
	TrackingDeviceUpdateTimer = GetDefault<UManusSettings>()->TrackingManagerDeviceUpdateFrequency;
}

UWorld* UManusTrackingManager::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

void UManusTrackingManager::Initialize()
{
	// Initialize the tracking devices array.
	for (uint32_t i = 0; i < MAX_NUM_STEAM_VR_DEVICES; i++)
	{
		TrackingDevices[i].bIsAvailable = false;
		TrackingDevices[i].bWasAtSomePointAvailable = false;

		TrackingDevices[i].Type = EManusTrackingDeviceType::Invalid;
		TrackingDevices[i].BodyPart = EManusTrackingBodyPart::None;
	}

	// There is no way to check if the HMD is available. It is therefore assumed to always be available.
	int32 HmdIndex = static_cast<int32>(vr::k_unTrackedDeviceIndex_Hmd);
	TrackingDevices[HmdIndex].bIsAvailable = true;
	TrackingDevices[HmdIndex].bWasAtSomePointAvailable = true;
	TrackingDevices[HmdIndex].Type = EManusTrackingDeviceType::Hmd;
	TrackingDevices[HmdIndex].BodyPart = EManusTrackingBodyPart::Head;

	// Start updating tracking devices
	DoTrackingDeviceUpdate();
}

#ifdef MANUS_PLUGIN_USE_STEAMVR
#if STEAMVR_SUPPORTED_PLATFORMS
static FString GetFStringTrackedDeviceProperty(vr::IVRSystem* VRSystem, uint32 DeviceIndex, vr::ETrackedDeviceProperty Property)
{
	check(VRSystem != nullptr);

	vr::TrackedPropertyError Error;
	TArray<char> Buffer;
	Buffer.AddUninitialized(vr::k_unMaxPropertyStringSize);

	int Size = VRSystem->GetStringTrackedDeviceProperty(DeviceIndex, Property, Buffer.GetData(), Buffer.Num(), &Error);
	if (Error == vr::TrackedProp_BufferTooSmall)
	{
		Buffer.AddUninitialized(Size - Buffer.Num());
		Size = VRSystem->GetStringTrackedDeviceProperty(DeviceIndex, Property, Buffer.GetData(), Buffer.Num(), &Error);
	}

	if (Error == vr::TrackedProp_Success)
	{
		return UTF8_TO_TCHAR(Buffer.GetData());
	}
	else
	{
		return UTF8_TO_TCHAR(VRSystem->GetPropErrorNameFromEnum(Error));
	}
}
#endif //STEAMVR_SUPPORTED_PLATFORMS
#endif //MANUS_PLUGIN_USE_STEAMVR

void UManusTrackingManager::UpdateViveTrackers()
{
#ifdef MANUS_PLUGIN_USE_STEAMVR
	ISteamVRPlugin* SteamVRHMDModule = FModuleManager::LoadModulePtr<ISteamVRPlugin>(TEXT("SteamVR"));
	if (SteamVRHMDModule && SteamVRHMDModule->GetVRSystem())
	{
		vr::IVRSystem* VRSystem = SteamVRHMDModule->GetVRSystem();
		
		for (uint32_t i = 0; i < MAX_NUM_STEAM_VR_DEVICES; i++)
		{
			TrackingDevices[i].bIsAvailable = VRSystem->IsTrackedDeviceConnected(i);
			
			// Update available devices
			if (TrackingDevices[i].bIsAvailable)
			{
				TrackingDevices[i].bWasAtSomePointAvailable = true;

				// Serial number
				TrackingDevices[i].DeviceId = FName(*GetFStringTrackedDeviceProperty(VRSystem, i, vr::Prop_SerialNumber_String));

				// Device type
				TrackingDevices[i].Type = EManusTrackingDeviceType::GenericTracker;

				// Body part
				EManusTrackingBodyPart BodyPart = EManusTrackingBodyPart::None;
				FString ControllerType = GetFStringTrackedDeviceProperty(VRSystem, i, vr::Prop_ControllerType_String);
				if (ControllerType.Equals("vive_tracker_handed"))
				{
					int32_t ControllerRoleHint = VRSystem->GetInt32TrackedDeviceProperty(i, vr::Prop_ControllerRoleHint_Int32);
					if (ControllerRoleHint == 1)
					{
						BodyPart = EManusTrackingBodyPart::LeftHand;
					}
					else if (ControllerRoleHint == 2)
					{
						BodyPart = EManusTrackingBodyPart::RightHand;
					}
				}
				else if (ControllerType.Equals("vive_tracker_waist"))
				{
					BodyPart = EManusTrackingBodyPart::Waist;
				}
				else if (ControllerType.Equals("vive_tracker_left_foot"))
				{
					BodyPart = EManusTrackingBodyPart::LeftFoot;
				}
				else if (ControllerType.Equals("vive_tracker_right_foot"))
				{
					BodyPart = EManusTrackingBodyPart::RightFoot;
				}
				TrackingDevices[i].BodyPart = BodyPart;
			}
		}
	} 
	else 
	{
		SteamVRHMDModule->StartupModule();
		SteamVRHMDModule->Initialize();
	}
#endif //MANUS_PLUGIN_USE_STEAMVR
}

bool UManusTrackingManager::GetTrackingDevicePositionAndRotation(EManusTrackingBodyPart BodyPart, FVector& Position, FRotator& Rotation)
{
#ifdef MANUS_PLUGIN_USE_STEAMVR
	for (int32 i = 0; i < MAX_NUM_STEAM_VR_DEVICES; i++)
	{
		if (TrackingDevices[i].bWasAtSomePointAvailable		// Device is available.
		&&	TrackingDevices[i].BodyPart == BodyPart			// Device tracks the right body part.
		) {
			return USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(i, Position, Rotation);
		}
	}
#endif
	return false;
}

bool UManusTrackingManager::GetTrackingDevicePositionAndRotation(FName DeviceId, FVector& Position, FRotator& Rotation)
{
	if (DeviceId.IsNone())
	{
		return false;
	}

#ifdef MANUS_PLUGIN_USE_STEAMVR
	for (int32 i = 0; i < MAX_NUM_STEAM_VR_DEVICES; i++)
	{
		if (TrackingDevices[i].bWasAtSomePointAvailable		// Device is available.
		&&	TrackingDevices[i].DeviceId == DeviceId			// Device has the same device ID.
		) {
			return USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(i, Position, Rotation);
		}
	}
#endif
	return false;
}

bool UManusTrackingManager::GetFirstTrackingDeviceId(EManusTrackingBodyPart BodyPart, FName& DeviceId)
{
#ifdef MANUS_PLUGIN_USE_STEAMVR
	for (int32 i = 0; i < MAX_NUM_STEAM_VR_DEVICES; i++)
	{
		if (TrackingDevices[i].bWasAtSomePointAvailable		// Device is available.
		&&	TrackingDevices[i].BodyPart == BodyPart			// Device tracks the right body part.
		) {
			DeviceId = TrackingDevices[i].DeviceId;
			return true;
		}
	}
#endif
	return false;
}

void UManusTrackingManager::GetTrackingDeviceIds(EManusTrackingBodyPart BodyPart, TArray<FName>& DeviceIds)
{
	DeviceIds.Reset();

#ifdef MANUS_PLUGIN_USE_STEAMVR
	for (int32 i = 0; i < MAX_NUM_STEAM_VR_DEVICES; i++)
	{
		if (TrackingDevices[i].bWasAtSomePointAvailable		// Device is available.
			&& TrackingDevices[i].BodyPart == BodyPart		// Device tracks the right body part.
			) {
			DeviceIds.Add(TrackingDevices[i].DeviceId);
		}
	}
#endif
}
