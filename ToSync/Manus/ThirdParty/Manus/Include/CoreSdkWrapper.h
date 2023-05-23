#ifndef H_CORESDKWRAPPER
#define H_CORESDKWRAPPER

// Set up a Doxygen group.
/** @addtogroup CoreSdkWrapper
 *  @{
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include "CoreSdkWrapperTypes.h"

#include <stdint.h>

#ifdef _WIN32
	// CORESDKWRAPPER_EXPORTS defines if the functions are exported to or
	// imported from a DLL.
	#ifdef CORESDKWRAPPER_EXPORTS
		#define CORESDK_API __declspec(dllexport)
	#else
		#define CORESDK_API __declspec(dllimport)
	#endif
#elif defined(__linux__)
	#define CORESDK_API
#else
	#error Unrecognised platform.
#endif

/******************************************************************************
 * Wrapper startup and shutdown.
 *****************************************************************************/

/// @brief Initialise the wrapper. Call this before using the wrapper.
CORESDK_API WrapperReturnCode_t CoreSdk_Initialise(ClientType a_TypeOfClient);
/// @brief Shut down the wrapper. This needs to be called last.
CORESDK_API WrapperReturnCode_t CoreSdk_ShutDown(void);

/******************************************************************************
 * Utility functions.
 *****************************************************************************/

/// @brief Check if the wrapper DLL was built in the debug configuration.
CORESDK_API WrapperReturnCode_t CoreSdk_WasDllBuiltInDebugConfiguration(bool& a_WasBuiltInDebugConfiguration);

/******************************************************************************
 * Connection handling.
 *****************************************************************************/

/// @brief Start a background task that looks for hosts running Manus Core.
///
/// Call this first when looking for hosts to connect to.
CORESDK_API WrapperReturnCode_t CoreSdk_StartLookingForHosts(void);
/// @brief Stop the background task that looks for hosts running Manus Core.
///
/// The task must be stopped before a connection can be made.
/// This is the second function to call when looking for hosts to connect to.
CORESDK_API WrapperReturnCode_t CoreSdk_StopLookingForHosts(void);
/// @brief Get the number of hosts running Manus Core that were found.
///
/// This is the third function to call when looking for hosts to connect to.
CORESDK_API WrapperReturnCode_t CoreSdk_GetNumberOfAvailableHostsFound(uint32_t& a_NumberOfAvailableHostsFound);
/// @brief Fill the given array with information on the hosts that were found.
///
/// This is the fourth and final function to call when looking for hosts to
/// connect to
CORESDK_API WrapperReturnCode_t CoreSdk_GetAvailableHostsFound(ManusHost* a_AvailableHostsFound, const uint32_t a_NumberOfHostsThatFitInArray);
CORESDK_API WrapperReturnCode_t CoreSdk_GetIsConnectedToCore(bool& a_ConnectedToCore);

/// @brief Connect to Manus Core on the same device the wrapper is running on.
CORESDK_API WrapperReturnCode_t CoreSdk_ConnectLocally(void);
/// @brief Connect to a host using the given host information.
CORESDK_API WrapperReturnCode_t CoreSdk_ConnectToHost(ManusHost a_Host);

/// @brief Check if the connection to Manus Core was lost due to a timeout.
CORESDK_API WrapperReturnCode_t CoreSdk_WasConnectionLostDueToTimeout(bool& a_WasConnectionLost);
CORESDK_API WrapperReturnCode_t CoreSdk_GetVersionsAndCheckCompatibility(ManusVersion& a_WrapperVersion, ManusVersion& a_SdkVersion, ManusVersion& a_CoreVersion, bool& a_AreVersionsCompatible);

// Callbacks
CORESDK_API WrapperReturnCode_t CoreSdk_RegisterCallbackForOnConnect(ConnectedToCoreCallback_t a_ConnectedCallback);
CORESDK_API WrapperReturnCode_t CoreSdk_RegisterCallbackForOnDisconnect(DisconnectedFromCoreCallback_t a_DisconnectedCallback);

/******************************************************************************
 * Basic glove interaction.
 *****************************************************************************/

/// @brief Vibrate the motor on the back of the glove.
CORESDK_API WrapperReturnCode_t CoreSdk_VibrateWristOfGlove(uint32_t a_GloveId, float a_UnitStrength, uint16_t a_DurationInMilliseconds);
/// @brief Vibrate the motor on the given finger of a haptic glove.
CORESDK_API WrapperReturnCode_t CoreSdk_VibrateFingers(uint32_t a_DongleId, HandType a_HandType, const float* a_Powers);

/// @brief Get a glove ID for the given hand of the given Polygon user.
///
/// The user index refers to the list of users shown in the dashboard under the
/// Polygon tab. The first user shown is user index 0, the second is number 1,
/// and so on. This is converted to the hexadecimal user ID internally.
/// Note that the list of users can be edited from the dashboard, so it can
/// change at any time.
CORESDK_API WrapperReturnCode_t CoreSdk_GetGloveIdOfUser_UsingUserIndex(uint32_t a_UserIndex, HandType a_HandType, uint32_t& a_GloveId);
/// @brief Get the number of gloves that are available.
CORESDK_API WrapperReturnCode_t CoreSdk_GetNumberOfAvailableGloves(uint32_t& a_NumberOfAvailableGloves);
/// @brief Fill the given array with the IDs of all available gloves.
///
/// The size of the given array must match the number of available gloves.
/// Note that the number of available gloves can change at any time.
CORESDK_API WrapperReturnCode_t CoreSdk_GetIdsOfAvailableGloves(uint32_t* a_IdsOfAvailableGloves, uint32_t a_NumberOfIdsThatFitInArray);
/// @brief Get the hand type of the glove with the given glove ID.
CORESDK_API WrapperReturnCode_t CoreSdk_GetHandTypeOfGlove(uint32_t a_GloveId, HandType& a_HandType);
/// @brief Get the glove ID of the first available glove of the given type.
CORESDK_API WrapperReturnCode_t CoreSdk_GetIdOfFirstAvailableGloveOfType(HandType a_HandType, uint32_t& a_GloveId);

/// @brief Get data for the glove with the given glove ID.
CORESDK_API WrapperReturnCode_t CoreSdk_GetDataForGlove_UsingGloveId(uint32_t a_GloveId, GloveData& a_GloveData);
/// @brief Get data for the first available glove with the given hand type.
CORESDK_API WrapperReturnCode_t CoreSdk_GetDataForGlove_UsingHandType(HandType a_HandType, GloveData& a_GloveData);

/******************************************************************************
 * Haptics module.
 *****************************************************************************/

/// @brief Get the number of available haptics dongles.
CORESDK_API WrapperReturnCode_t CoreSdk_GetNumberOfHapticsDongles(uint32_t& a_NumberOfHapticsDongles);
/// @brief Fill the given array with the IDs of all available haptics dongles.
///
/// The size of the given array must match the number of available haptics
/// dongles.
/// Note that the number of available haptics dongles can change at any time.
CORESDK_API WrapperReturnCode_t CoreSdk_GetHapticsDongleIds(uint32_t* a_HapticsDongleIds, uint32_t a_NumberOfIdsThatFitInArray);

/******************************************************************************
 * Polygon.
 *****************************************************************************/

/// @brief Get the number of available Polygon users.
CORESDK_API WrapperReturnCode_t CoreSdk_GetNumberOfAvailablePolygonUsers(uint32_t& a_NumberOfAvailableUsers);
/// @brief Fill the given array with the IDs of all available Polygon users.
///
/// The size of the given array must match the number of available Polygon
/// users.
/// Note that the number of available Polygon users can change at any time.
CORESDK_API WrapperReturnCode_t CoreSdk_GetIdsOfAvailablePolygonUsers(int32_t* a_IdsOfAvailablePolygonUsers, uint32_t a_NumberOfIdsThatFitInArray);

/// @brief Send skeleton data to Core. If new, the skeleton will be added.
CORESDK_API WrapperReturnCode_t CoreSdk_AddOrUpdatePolygonSkeleton(const PolygonSkeletonData& a_PolygonSkeletonData);
/// @brief Remove the skeleton described by the arguments.
CORESDK_API WrapperReturnCode_t CoreSdk_RemovePolygonSkeleton(const RemoveSkeletonArgs& a_RemoveSkeletonArgs);
/// @brief Set the target for the given Polygon skeleton.
CORESDK_API WrapperReturnCode_t CoreSdk_SetPolygonSkeletonTarget(const PolygonTargetArgs& a_SetSkeletonTargetArgs);
/// @brief Set the retargeting settings for the given Polygon skeleton.
CORESDK_API WrapperReturnCode_t CoreSdk_SetPolygonSkeletonRetargetingSettings(const RetargetingSettingsArgs& a_RetargetingSettingsArgs);
/// @brief Check if the given Polygon skeleton ID is valid.
CORESDK_API WrapperReturnCode_t CoreSdk_IsPolygonSkeletonIdValid(uint32_t a_PolygonSkeletonId, bool& a_IsValid);
/// @brief Get the number of available Polygon skeletons.
CORESDK_API WrapperReturnCode_t CoreSdk_GetNumberOfAvailablePolygonSkeletons(uint32_t& a_NumberOfAvailablePolygonSkeletons);
/// @brief Fill the given array with the IDs of available Polygon skeletons.
///
/// The size of the given array must match the number of available Polygon
/// skeletons.
/// Note that the number of available Polygon skeletons can change at any time.
CORESDK_API WrapperReturnCode_t CoreSdk_GetIdsOfAvailablePolygonSkeletons(uint32_t* a_IdsOfAvailablePolygonSkeletons, uint32_t a_NumberOfIdsThatFitInArray);
/// @brief Get the ID of the first Polygon skeleton available.
CORESDK_API WrapperReturnCode_t CoreSdk_GetIdOfFirstAvailablePolygonSkeleton(uint32_t& a_PolygonSkeletonId);
/// @brief Get data for the given Polygon skeleton ID.
CORESDK_API WrapperReturnCode_t CoreSdk_GetDataForPolygonSkeleton(uint32_t a_PolygonSkeletonId, PolygonSkeletonData& a_PolygonSkeletonData);

/******************************************************************************
 * Users.
 *****************************************************************************/
 /// @brief Get number of users. (that were created in the dashboard)
CORESDK_API WrapperReturnCode_t CoreSdk_GetNumberOfUsers(uint32_t& a_NumberOfUsers);

/******************************************************************************
 * Tracking.
 *****************************************************************************/

/// @brief Get the number of available trackers.
CORESDK_API WrapperReturnCode_t CoreSdk_GetNumberOfAvailableTrackers(uint32_t& a_NumberOfAvailableTrackers);
/// @brief Fill the given array with the IDs of available trackers.
///
/// The size of the given array must match the number of available trackers.
/// Note that the number of available trackers can change at any time.
CORESDK_API WrapperReturnCode_t CoreSdk_GetIdsOfAvailableTrackers(TrackerId* a_IdsOfAvailableTrackers, uint32_t a_NumberOfIdsThatFitInArray);

/// @brief Get the number of available trackers in a user.
CORESDK_API WrapperReturnCode_t CoreSdk_GetNumberOfAvailableTrackersForUserIndex(uint32_t& a_NumberOfAvailableTrackers, uint32_t a_UserIndex);

/// @brief Fill the given array with the IDs of available trackers in a user.
///
/// The size of the given array must match the number of available trackers.
/// Note that the number of available trackers can change at any time.
CORESDK_API WrapperReturnCode_t CoreSdk_GetIdsOfAvailableTrackersForUserIndex(TrackerId* a_IdsOfAvailableTrackers, uint32_t a_UserIndex, uint32_t a_NumberOfIdsThatFitInArray);

/// @brief Get data for the tracker with the given tracker ID.
CORESDK_API WrapperReturnCode_t CoreSdk_GetDataForTracker_UsingTrackerId(TrackerId a_TrackerId, TrackerData& a_TrackerData);
/// @brief Get data for the tracker with the given Polygon user index and type.
CORESDK_API WrapperReturnCode_t CoreSdk_GetDataForTracker_UsingIndexAndType(uint32_t a_UserIndex, uint32_t a_TrackerType, TrackerData& a_TrackerData);
/// @brief Send data to Core for a tracker.
CORESDK_API WrapperReturnCode_t CoreSdk_SendDataForTrackers(const TrackerData* a_TrackerData, uint32_t a_NumberOfTrackers);

#ifdef __cplusplus
} // extern "C"
#endif

// Close the Doxygen group.
/** @} */

#endif // #ifndef H_CORESDKWRAPPER
