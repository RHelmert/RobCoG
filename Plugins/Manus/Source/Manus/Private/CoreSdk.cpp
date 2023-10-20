// Copyright 2015-2022 Manus

#include "CoreSdk.h"
#include "Manus.h"
#include "ManusSkeleton.h"
#include "ManusClientSkeleton.h"
#include "ManusBlueprintTypes.h"

#include "Misc/FileHelper.h"

#include "ManusConvert.h"
#include "ManusSDK.h"

#include "Interfaces/IPluginManager.h"
#include "AnimationRuntime.h"
#include "Misc/MessageDialog.h"
#include "Kismet/KismetMathLibrary.h"

//C:\Program Files\Epic Games\UE_5.2\Engine\Source\Runtime\Core\Public\Misc\Paths.h
#include "Runtime/Core/Public/Misc/Paths.h"

#include <limits>

#ifdef _WIN64
	#define PLATFORM_STRING "Win64"
#else
	#error Unknown platform.
#endif

#define RETURN_IF_NOT_INITIALISED(FunctionName, ReturnValue) \
	if (!s_FunctionPointers)		\
	{							\
		UE_LOG(LogManus, Error, TEXT("Tried to use the Manus SDK function \"%s\", but the SDK was not initialised."), FunctionName); \
		return ReturnValue;		\
	}							\

#define RETURN_IF_NOT_INITIALISED_SILENT(FunctionName, ReturnValue) \
	if (!s_FunctionPointers)		\
	{							\
		return ReturnValue;		\
	}	

DECLARE_CYCLE_STAT(TEXT("CoreSDK Initialize"),								STAT_CoreSDK_Initialize, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK CoreSdk_ConnectGRPC"),						STAT_CoreSDK_ConnectGRPC, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK CoreSdk_ConnectToHost"),					STAT_CoreSDK_ConnectToHost, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK CoreSdk_LookForHosts"),					STAT_CoreSDK_LookForHosts, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK CoreSdk_GetNumberOfAvailableHostsFound"),  STAT_CoreSDK_GetNumberOfAvailableHostsFound, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK CoreSdk_GetAvailableHostsFound"),			STAT_CoreSDK_GetAvailableHostsFound, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK CoreSdk_GetIsConnectedToCore"),			STAT_CoreSDK_GetIsConnectedToCore, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK CoreSdk_GetSessionId"),					STAT_CoreSDK_GetSessionId, STATGROUP_Manus);


DECLARE_CYCLE_STAT(TEXT("CoreSDK ShutDown"),								STAT_CoreSDK_ShutDown, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK GetVersionsAndCheckCompatibility"),		STAT_CoreSDK_GetVersionsAndCheckCompatibility, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK VibrateFingers"),							STAT_CoreSDK_VibrateFingers, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK VibrateFingersForSkeleton"),				STAT_CoreSDK_VibrateFingersForSkeleton, STATGROUP_Manus);
//todo add new functions 

DECLARE_CYCLE_STAT(TEXT("CoreSDK GetGloveIdOfUser_UsingUserId"),			STAT_CoreSDK_GetGloveIdOfUser_UsingUserId, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK GetNumberOfAvailableUsers"),				STAT_CoreSDK_GetNumberOfAvailableUsers, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetIdsOfAvailableUsers"),					STAT_CoreSDK_GetIdsOfAvailableUsers, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK GetNumberOfAvailableGloves"),				STAT_CoreSDK_GetNumberOfAvailableGloves, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetIdsOfAvailableGloves"),					STAT_CoreSDK_GetIdsOfAvailableGloves, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK GetDataForGlove_UsingGloveId"),			STAT_CoreSDK_GetDataForGlove_UsingGloveId, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK GetNumberOfHapticsDongles"),				STAT_CoreSDK_GetNumberOfHapticsDongles, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetHapticsDongleIds"),						STAT_CoreSDK_GetHapticsDongleIds, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK GetNumberOfAvailableTrackers"),			STAT_CoreSDK_GetNumberOfAvailableTrackers, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetIdsOfAvailableTrackers"),				STAT_CoreSDK_GetIdsOfAvailableTrackers, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetNumberOfAvailableTrackersForUserId"),	STAT_CoreSDK_GetNumberOfAvailableTrackersForUserId, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetIdsOfAvailableTrackersForUserId"),		STAT_CoreSDK_GetIdsOfAvailableTrackersForUserId, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetDataForTracker_UsingTrackerId"),		STAT_CoreSDK_GetDataForTracker_UsingTrackerId, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetDataForTracker_UsingIdAndType"),		STAT_CoreSDK_GetDataForTracker_UsingIdAndType, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK SendDataForTrackers"),						STAT_CoreSDK_SendDataForTrackers, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK AssignTrackerToUser"),						STAT_CoreSDK_AssignTrackerToUser, STATGROUP_Manus);



DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL Initialize"),								STAT_CoreSDK_DLL_Initialize, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL InitializeCoordinateSystemWithVUH"),		STAT_CoreSDK_DLL_InitializeCoordinateSystemWithVUH, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL ConnectGRPC"),								STAT_CoreSDK_DLL_ConnectGRPC, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL ConnectToHost"),							STAT_CoreSDK_DLL_ConnectToHost, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL LookForHosts"),							STAT_CoreSDK_DLL_LookForHosts, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetNumberOfAvailableHostsFound"),			STAT_CoreSDK_DLL_GetNumberOfAvailableHostsFound, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetAvailableHostsFound"),					STAT_CoreSDK_DLL_GetAvailableHostsFound, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetIsConnectedToCore"),					STAT_CoreSDK_DLL_GetIsConnectedToCore, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetSessionId"),							STAT_CoreSDK_DLL_GetSessionId, STATGROUP_Manus);


DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL ShutDown"),								STAT_CoreSDK_DLL_ShutDown, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetVersionsAndCheckCompatibility"),		STAT_CoreSDK_DLL_GetVersionsAndCheckCompatibility, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL VibrateFingers"),							STAT_CoreSDK_DLL_VibrateFingers, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL VibrateFingersForSkeleton"),				STAT_CoreSDK_DLL_VibrateFingersForSkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetGloveIdOfUser_UsingUserId"),			STAT_CoreSDK_DLL_GetGloveIdOfUser_UsingUserId, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetNumberOfAvailableUsers"),						STAT_CoreSDK_DLL_GetNumberOfAvailableUsers, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetIdsOfAvailableUsers"),							STAT_CoreSDK_DLL_GetIdsOfAvailableUsers, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetNumberOfAvailableGloves"),				STAT_CoreSDK_DLL_GetNumberOfAvailableGloves, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetIdsOfAvailableGloves"),					STAT_CoreSDK_DLL_GetIdsOfAvailableGloves, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetDataForGlove_UsingGloveId"),			STAT_CoreSDK_DLL_GetDataForGlove_UsingGloveId, STATGROUP_Manus);


DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetNumberOfHapticsDongles"),				STAT_CoreSDK_DLL_GetNumberOfHapticsDongles, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetHapticsDongleIds"),						STAT_CoreSDK_DLL_GetHapticsDongleIds, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetNumberOfAvailableTrackers"),			STAT_CoreSDK_DLL_GetNumberOfAvailableTrackers, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetIdsOfAvailableTrackers"),				STAT_CoreSDK_DLL_GetIdsOfAvailableTrackers, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetNumberOfAvailableTrackersForUserId"),	STAT_CoreSDK_DLL_GetNumberOfAvailableTrackersForUserId, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetIdsOfAvailableTrackersForUserId"),		STAT_CoreSDK_DLL_GetIdsOfAvailableTrackersForUserId, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetDataForTracker_UsingTrackerId"),		STAT_CoreSDK_DLL_GetDataForTracker_UsingTrackerId, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetDataForTracker_UsingIdAndType"),		STAT_CoreSDK_DLL_GetDataForTracker_UsingIdAndType, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL SendDataForTrackers"),						STAT_CoreSDK_DLL_SendDataForTrackers, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL AssignTrackerToUser"),						STAT_CoreSDK_DLL_AssignTrackerToUser, STATGROUP_Manus);

 
//Callbacks
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL RegisterCallbackForOnConnect"),			STAT_CoreSDK_DLL_RegisterCallbackForOnConnect, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL RegisterCallbackForOnDisconnect"),			STAT_CoreSDK_DLL_RegisterCallbackForOnDisconnect, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL RegisterCallbackForSkeletonStream"),		STAT_CoreSDK_DLL_RegisterCallbackForSkeletonStream, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL RegisterCallbackForErgonomicsStream"),		STAT_CoreSDK_DLL_RegisterCallbackForErgonomicsStream, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL RegisterCallbackForSystemStream"),			STAT_CoreSDK_DLL_RegisterCallbackForSystemStream, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL RegisterCallbackForGestureStream"),		STAT_CoreSDK_DLL_RegisterCallbackForGestureStream, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL RegisterCallbackForLandscapeStream"),      STAT_CoreSDK_DLL_RegisterCallbackForLandscapeStream, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetGestureStreamData"),                    STAT_CoreSDK_DLL_GetGestureStreamData, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetGestureLandscapeData"),                 STAT_CoreSDK_DLL_GetGestureLandscapeData, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetSkeletonInfo"),							STAT_CoreSDK_DLL_GetSkeletonInfo, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetSkeletonData"),							STAT_CoreSDK_DLL_GetSkeletonData, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL CreateSkeletonSetup"),						STAT_CoreSDK_DLL_CreateSkeletonSetup, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL ClearAllTemporarySkeletons"),				STAT_CoreSDK_DLL_ClearAllTemporarySkeletons, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL ClearTemporarySkeleton"),					STAT_CoreSDK_DLL_ClearTemporarySkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL AddNodeToSkeletonSetup"),					STAT_CoreSDK_DLL_AddNodeToSkeletonSetup, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL AddChainToSkeletonSetup"),					STAT_CoreSDK_DLL_AddChainToSkeletonSetup, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL AddMeshSetupToSkeletonSetup"),             STAT_CoreSDK_DLL_AddMeshSetupToSkeletonSetup, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL AddVertexToMeshSetup"),                    STAT_CoreSDK_DLL_AddVertexToMeshSetup, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL AddTriangleToMeshSetup"),                  STAT_CoreSDK_DLL_AddTriangleToMeshSetup, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL OverwriteSkeletonSetup"),					STAT_CoreSDK_DLL_OverwriteSkeletonSetup, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL OverwriteNodeToSkeletonSetup"),			STAT_CoreSDK_DLL_OverwriteNodeToSkeletonSetup, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL OverwriteChainToSkeletonSetup"),			STAT_CoreSDK_DLL_OverwriteChainToSkeletonSetup, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetSkeletonSetupArraySizes"),				STAT_CoreSDK_DLL_GetSkeletonSetupArraySizes, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL AllocateChainsForSkeletonSetup"),			STAT_CoreSDK_DLL_AllocateChainsForSkeletonSetup, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetSkeletonSetupInfo"),					STAT_CoreSDK_DLL_GetSkeletonSetupInfo, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetSkeletonSetupChains"),					STAT_CoreSDK_DLL_GetSkeletonSetupChains, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetSkeletonSetupNodes"),					STAT_CoreSDK_DLL_GetSkeletonSetupNodes, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL LoadSkeleton"),							STAT_CoreSDK_DLL_LoadSkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL UnloadSkeleton"),							STAT_CoreSDK_DLL_UnloadSkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL SaveTemporarySkeleton"),					STAT_CoreSDK_DLL_SaveTemporarySkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetTemporarySkeleton"),					STAT_CoreSDK_DLL_GetTemporarySkeleton, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL CompressTemporarySkeletonAndGetSize"),		STAT_CoreSDK_DLL_CompressTemporarySkeletonAndGetSize, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetCompressedTemporarySkeletonData"),		STAT_CoreSDK_DLL_GetCompressedTemporarySkeletonData, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetTemporarySkeletonFromCompressedData"),	STAT_CoreSDK_DLL_GetTemporarySkeletonFromCompressedData, STATGROUP_Manus);

// Forward-declarations for local functions.
static bool ConvertSdkSkeletonDataToBp(const ClientSkeleton &p_SdkInput, FManusMetaSkeleton& p_BpOutput);
static bool ConvertSdkTrackerDataToBp(const TrackerData & p_SdkInput, FManusTracker & p_BpOutput);
static void OnConnectedToCore(void); // todo obsolete/unneeded?
//static void OnDisconnectedFromCore(void); // TBD

// Static member variables
CoreSdk::CoreFunctionPointers* CoreSdk::s_FunctionPointers = nullptr;
void* CoreSdk::s_DllHandle = nullptr;

FCriticalSection* CoreSdk::s_CriticalSectionSkeletons = new FCriticalSection(); 
ClientSkeletonCollection* CoreSdk::s_NextSkeletons = nullptr;

FCriticalSection* CoreSdk::s_CriticalSectionErgonomics = new FCriticalSection();
ErgonomicsStream* CoreSdk::s_ErgonomicsData = nullptr;

FCriticalSection* CoreSdk::s_CriticalSectionLandscape = new FCriticalSection();
Landscape CoreSdk::s_LandscapeData;

FCriticalSection* CoreSdk::s_CriticalSectionGesture = new FCriticalSection();
std::vector<ClientGestures> CoreSdk::s_GestureData;

std::vector<GestureLandscapeData> CoreSdk::s_GestureLandscapeData;

FCriticalSection* CoreSdk::s_CriticalSectionSystem = new FCriticalSection();
uint32_t CoreSdk::s_LastModifiedSkeletonIndex = UINT32_MAX;

uint32_t CoreSdk::s_SecondsToFindHosts = 1;

/**
 * Stores all the function pointers to functions in the Core SDK wrapper.
 * Doing this here makes it possible to keep SDK wrapper types and functions contained to this .cpp file.
 */
struct CoreSdk::CoreFunctionPointers
{
	/*      return value				type name													function arguments */
	typedef SDKReturnCode(*WasDllBuiltInDebugConfiguration_FunctionPointer)			(bool* p_WasDllBuiltInDebugConfiguration);
	typedef SDKReturnCode(*GetTimestampInfo_FunctionPointer)						(ManusTimestamp p_Timestamp, ManusTimestampInfo* p_Info);
	typedef SDKReturnCode(*SetTimestampInfo_FunctionPointer)						(ManusTimestamp* p_Timestamp, ManusTimestampInfo p_Info);
	typedef SDKReturnCode(*Initialize_FunctionPointer)								(SessionType p_TypeOfSession);
	typedef SDKReturnCode(*InitializeCoordinateSystemWithVUH_FunctionPointer)		(CoordinateSystemVUH p_CoordinateSystem , bool p_UseWorldCoordinates); // todo direction version

	typedef SDKReturnCode(*ConnectGRPC_FunctionPointer)								(void);
	typedef SDKReturnCode(*ConnectToHost_FunctionPointer)							(ManusHost p_Host);
	typedef SDKReturnCode(*LookForHosts_FunctionPointer)							(uint32_t p_WaitSeconds, bool p_LoopbackOnly);
	typedef SDKReturnCode(*GetNumberOfAvailableHostsFound_FunctionPointer)			(uint32_t* p_NumberOfAvailableHostsFound);
	typedef SDKReturnCode(*GetAvailableHostsFound_FunctionPointer)					(ManusHost* p_AvailableHostsFound, const uint32_t p_NumberOfHostsThatFitInArray);
	typedef SDKReturnCode(*GetIsConnectedToCore_FunctionPointer)					(bool* p_IsConnectedToCore);
	typedef SDKReturnCode(*GetSessionId_FunctionPointer)							(uint32_t* p_SessionId);
	

	typedef SDKReturnCode(*ShutDown_FunctionPointer)								(void);
	typedef SDKReturnCode(*GetVersionsAndCheckCompatibility_FunctionPointer)		(ManusVersion* p_SdkVersion, ManusVersion* p_CoreVersion, bool* p_Compatible);
	typedef SDKReturnCode(*VibrateFingers_FunctionPointer)							(uint32_t p_DongleId, Side p_HandType, float* p_Powers);
	typedef SDKReturnCode(*VibrateFingersForSkeleton_FunctionPointer)				(uint32_t p_SkeletonId, Side p_HandType, float* p_Powers);
	typedef SDKReturnCode(*GetGloveIdOfUser_UsingUserId_FunctionPointer)			(uint32_t p_UserId, Side p_HandType, uint32_t* p_GloveId);

	typedef SDKReturnCode(*GetNumberOfAvailableUsers_FunctionPointer)						(uint32_t* p_NumberOfUsers);
	typedef SDKReturnCode(*GetIdsOfAvailableUsers_FunctionPointer)							(uint32_t* p_IdsOfUsers, uint32_t p_NumberOfIdsThatFitInArray);

	typedef SDKReturnCode(*GetNumberOfAvailableGloves_FunctionPointer)				(uint32_t* p_NumberOfAvailableGloves);
	typedef SDKReturnCode(*GetIdsOfAvailableGloves_FunctionPointer)					(uint32_t* p_IdsOfAvailableGloves, uint32_t p_NumberOfIdsThatFitInArray);
	typedef SDKReturnCode(*GetDataForGlove_UsingGloveId_FunctionPointer)							(uint32_t p_GloveId, GloveLandscapeData* p_GloveData);
		
	typedef SDKReturnCode(*GetNumberOfHapticsDongles_FunctionPointer)				(uint32_t* p_NumberOfHapticsDongles);
	typedef SDKReturnCode(*GetHapticsDongleIds_FunctionPointer)						(uint32_t* p_HapticsDongleIds, uint32_t p_NumberOfIdsThatFitInArray);
	typedef SDKReturnCode(*GetNumberOfAvailableTrackers_FunctionPointer)			(uint32_t* p_NumberOfAvailableTrackers);
	typedef SDKReturnCode(*GetIdsOfAvailableTrackers_FunctionPointer)				(TrackerId* p_IdsOfAvailableTrackers, uint32_t p_NumberOfIdsThatFitInArray);
	typedef SDKReturnCode(*GetNumberOfAvailableTrackersForUserId_FunctionPointer)	(uint32_t* p_NumberOfAvailableTrackers, uint32_t p_UserId);
	typedef SDKReturnCode(*GetIdsOfAvailableTrackersForUserId_FunctionPointer)      (TrackerId* p_IdsOfAvailableTrackers, uint32_t p_UserId, uint32_t p_NumberOfIdsThatFitInArray);
	typedef SDKReturnCode(*GetDataForTracker_UsingTrackerId_FunctionPointer)		(TrackerId p_TrackerId, TrackerData* p_TrackerData);
	typedef SDKReturnCode(*GetDataForTracker_UsingIdAndType_FunctionPointer)		(uint32_t p_UserId, uint32_t p_TrackerType, TrackerData* p_TrackerData);
	typedef SDKReturnCode(*SendDataForTrackers_FunctionPointer)						(const TrackerData* p_TrackerData, uint32_t p_NumberOfTrackers);
	typedef SDKReturnCode(*AssignTrackerToUser_FunctionPointer)						(char* p_TrackerId, uint32_t p_UserId);
	
	// Callbacks
	typedef SDKReturnCode(*RegisterCallbackForOnConnect_FunctionPointer)			(ConnectedToCoreCallback_t p_OnConnectedCallback);
	typedef SDKReturnCode(*RegisterCallbackForOnDisconnect_FunctionPointer)			(DisconnectedFromCoreCallback_t p_OnDisconnectedCallback);
	typedef SDKReturnCode(*RegisterCallbackForSkeletonStream_FunctionPointer)	   	(SkeletonStreamCallback_t p_OnSkeletonsCallback);
	typedef SDKReturnCode(*RegisterCallbackForErgonomicsStream_FunctionPointer)		(ErgonomicsStreamCallback_t p_OnErgonomicsCallbackStream);
	typedef SDKReturnCode(*RegisterCallbackForSystemStream_FunctionPointer)			(SystemStreamCallback_t p_OnSystemCallbackStream);
	//typedef SDKReturnCode(*RegisterCallbackForOnExtraData_FunctionPointer)	   	 	(ExtraDataCallback_t p_OnExtraDataCallback); // TBD
    typedef SDKReturnCode(*RegisterCallbackForLandscapeStream_FunctionPointer)		(LandscapeStreamCallback_t p_OnLandscapeCallbackStream);
    typedef SDKReturnCode(*RegisterCallbackForGestureStream_FunctionPointer)		(GestureStreamCallback_t p_OnGestureCallbackStream);


    typedef SDKReturnCode(*GetGestureStreamData_FunctionPointer)                    (uint32_t p_GestureStreamDataIndex, uint32_t p_StartDataIndex, GestureProbabilities* p_GestureProbabilitiesCollection);
    typedef SDKReturnCode(*GetGestureLandscapeData_FunctionPointer)                 (GestureLandscapeData* p_LandscapeDataArray, uint32_t p_ArraySize);

    typedef SDKReturnCode(*GetSkeletonInfo_FunctionPointer)	   	 					(uint32_t p_SkeletonIndex, SkeletonInfo* p_Skeletons);
	typedef SDKReturnCode(*GetSkeletonData_FunctionPointer)	   	 					(uint32_t p_SkeletonIndex, SkeletonNode* p_Nodes, uint32_t p_NodeCount);
	
	
	typedef SDKReturnCode(*CreateSkeletonSetup_FunctionPointer)	   	 				(SkeletonSetupInfo p_Skeleton, uint32_t* p_SkeletonSetupIndex);
	typedef SDKReturnCode(*ClearAllTemporarySkeletons_FunctionPointer)	   	 		(void);
	typedef SDKReturnCode(*ClearTemporarySkeleton_FunctionPointer)	   	 			(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId);
	typedef SDKReturnCode(*AddNodeToSkeletonSetup_FunctionPointer)	   	 			(uint32_t p_SkeletonSetupIndex, NodeSetup p_Node);
	typedef SDKReturnCode(*AddChainToSkeletonSetup_FunctionPointer)	   	 			(uint32_t p_SkeletonSetupIndex, ChainSetup p_chain);
	

    
    typedef SDKReturnCode(*AddMeshSetupToSkeletonSetup_FunctionPointer)	 			(uint32_t p_SkeletonSetupIndex, uint32_t p_NodeID, uint32_t* p_MeshSetupIndex);
    typedef SDKReturnCode(*AddVertexToMeshSetup_FunctionPointer)	   	 			(uint32_t p_SkeletonSetupIndex, uint32_t p_MeshSetupIndex, Vertex p_Vertex);
    typedef SDKReturnCode(*AddTriangleToMeshSetup_FunctionPointer)	   	 			(uint32_t p_SkeletonSetupIndex, uint32_t p_MeshSetupIndex, Triangle p_Triangle);

	typedef SDKReturnCode(*OverwriteSkeletonSetup_FunctionPointer)	   	 			(uint32_t p_SkeletonIndex, SkeletonSetupInfo p_Skeleton);
	typedef SDKReturnCode(*OverwriteNodeToSkeletonSetup_FunctionPointer)	   	 	(uint32_t p_SkeletonIndex, NodeSetup p_Node);
	typedef SDKReturnCode(*OverwriteChainToSkeletonSetup_FunctionPointer)	   	 	(uint32_t p_SkeletonIndex, ChainSetup p_Chain);
	typedef SDKReturnCode(*GetSkeletonSetupArraySizes_FunctionPointer)	   	 		(uint32_t p_SkeletonSetupIndex, SkeletonSetupArraySizes* p_SkeletonInfo);
	typedef SDKReturnCode(*AllocateChainsForSkeletonSetup_FunctionPointer)	   	 	(uint32_t p_SkeletonIndex);

    typedef SDKReturnCode(*GetSkeletonSetupInfo_FunctionPointer)	   	 			(uint32_t p_SkeletonSetupIndex, SkeletonSetupInfo* p_SDK);
    typedef SDKReturnCode(*GetSkeletonSetupChains_FunctionPointer)	   	 			(uint32_t p_SkeletonIndex, ChainSetup* p_SDK);
    typedef SDKReturnCode(*GetSkeletonSetupNodes_FunctionPointer)	   	 			(uint32_t p_SkeletonIndex, NodeSetup* p_SDK);
	typedef SDKReturnCode(*LoadSkeleton_FunctionPointer)	   	 					(uint32_t p_SkeletonIndex, uint32_t* p_SkeletonId);
	typedef SDKReturnCode(*UnloadSkeleton_FunctionPointer)	   	 					(uint32_t p_SkeletonId);
	typedef SDKReturnCode(*SaveTemporarySkeleton_FunctionPointer) 					(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId, bool p_IsSkeletonModified);

	typedef SDKReturnCode(*GetTemporarySkeleton_FunctionPointer)					(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId);
	typedef SDKReturnCode(*CompressTemporarySkeletonAndGetSize_FunctionPointer) 	(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId, uint32_t* p_TemporarySkeletonLengthInBytes);
	typedef SDKReturnCode(*GetCompressedTemporarySkeletonData_FunctionPointer) 		(unsigned char* p_TemporarySkeletonData, uint32_t p_TemporarySkeletonLengthInBytes);
	typedef SDKReturnCode(*GetTemporarySkeletonFromCompressedData_FunctionPointer) 	(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId, unsigned char* p_TemporarySkeletonData, uint32_t p_TemporarySkeletonLengthInBytes);                                                                                    
		
	WasDllBuiltInDebugConfiguration_FunctionPointer				WasDllBuiltInDebugConfiguration = nullptr;
	GetTimestampInfo_FunctionPointer							GetTimestampInfo = nullptr;
	SetTimestampInfo_FunctionPointer							SetTimestampInfo = nullptr;
	Initialize_FunctionPointer									Initialize = nullptr;
	InitializeCoordinateSystemWithVUH_FunctionPointer           InitializeCoordinateSystemWithVUH = nullptr;
	ConnectGRPC_FunctionPointer									ConnectGRPC = nullptr;
	ConnectToHost_FunctionPointer								ConnectToHost = nullptr;
	LookForHosts_FunctionPointer								LookForHosts = nullptr;
	GetNumberOfAvailableHostsFound_FunctionPointer				GetNumberOfAvailableHostsFound = nullptr;
	GetAvailableHostsFound_FunctionPointer						GetAvailableHostsFound = nullptr;
	GetIsConnectedToCore_FunctionPointer						GetIsConnectedToCore = nullptr;
	GetSessionId_FunctionPointer								GetSessionId = nullptr;
		
	ShutDown_FunctionPointer									ShutDown = nullptr;
	GetVersionsAndCheckCompatibility_FunctionPointer			GetVersionsAndCheckCompatibility = nullptr;
	VibrateFingers_FunctionPointer								VibrateFingers = nullptr;
	VibrateFingersForSkeleton_FunctionPointer					VibrateFingersForSkeleton = nullptr;
	GetGloveIdOfUser_UsingUserId_FunctionPointer				GetGloveIdOfUser_UsingUserId = nullptr;

	GetNumberOfAvailableUsers_FunctionPointer					GetNumberOfAvailableUsers = nullptr;
	GetIdsOfAvailableUsers_FunctionPointer						GetIdsOfAvailableUsers = nullptr;

	GetNumberOfAvailableGloves_FunctionPointer					GetNumberOfAvailableGloves = nullptr;
	GetIdsOfAvailableGloves_FunctionPointer						GetIdsOfAvailableGloves = nullptr;
	GetDataForGlove_UsingGloveId_FunctionPointer				GetDataForGlove_UsingGloveId = nullptr;
	
	GetNumberOfHapticsDongles_FunctionPointer					GetNumberOfHapticsDongles = nullptr;
	GetHapticsDongleIds_FunctionPointer							GetHapticsDongleIds = nullptr;

	GetNumberOfAvailableTrackers_FunctionPointer				GetNumberOfAvailableTrackers = nullptr;
	GetIdsOfAvailableTrackers_FunctionPointer					GetIdsOfAvailableTrackers = nullptr;
	GetNumberOfAvailableTrackersForUserId_FunctionPointer		GetNumberOfAvailableTrackersForUserId = nullptr;
	GetIdsOfAvailableTrackersForUserId_FunctionPointer          GetIdsOfAvailableTrackersForUserId = nullptr;
	GetDataForTracker_UsingTrackerId_FunctionPointer			GetDataForTracker_UsingTrackerId = nullptr;
	GetDataForTracker_UsingIdAndType_FunctionPointer			GetDataForTracker_UsingIdAndType = nullptr;
	SendDataForTrackers_FunctionPointer							SendDataForTrackers = nullptr;
	AssignTrackerToUser_FunctionPointer							AssignTrackerToUser = nullptr;

	// Callbacks
	RegisterCallbackForOnConnect_FunctionPointer				RegisterCallbackForOnConnect = nullptr;
	RegisterCallbackForOnDisconnect_FunctionPointer				RegisterCallbackForOnDisconnect = nullptr;
	RegisterCallbackForSkeletonStream_FunctionPointer		    RegisterCallbackForSkeletonStream = nullptr;
	RegisterCallbackForErgonomicsStream_FunctionPointer			RegisterCallbackForErgonomicsStream = nullptr;
	RegisterCallbackForSystemStream_FunctionPointer			    RegisterCallbackForSystemStream = nullptr;
	//RegisterCallbackForOnExtraData_FunctionPointer				RegisterCallbackForOnExtraData = nullptr; // TBD
    RegisterCallbackForGestureStream_FunctionPointer			RegisterCallbackForGestureStream = nullptr;
    RegisterCallbackForLandscapeStream_FunctionPointer			RegisterCallbackForLandscapeStream = nullptr;

    GetGestureStreamData_FunctionPointer			            GetGestureStreamData = nullptr;
    GetGestureLandscapeData_FunctionPointer                     GetGestureLandscapeData = nullptr;

    GetSkeletonInfo_FunctionPointer								GetSkeletonInfo = nullptr;
	GetSkeletonData_FunctionPointer								GetSkeletonData = nullptr;

	ClearAllTemporarySkeletons_FunctionPointer	   	 			ClearAllTemporarySkeletons = nullptr;
	ClearTemporarySkeleton_FunctionPointer	   	 				ClearTemporarySkeleton = nullptr;
	CreateSkeletonSetup_FunctionPointer	   	 					CreateSkeletonSetup = nullptr;
	AddNodeToSkeletonSetup_FunctionPointer	   	 				AddNodeToSkeletonSetup = nullptr;
	AddChainToSkeletonSetup_FunctionPointer	   	 				AddChainToSkeletonSetup = nullptr;

    
    AddMeshSetupToSkeletonSetup_FunctionPointer				    AddMeshSetupToSkeletonSetup = nullptr;
    AddVertexToMeshSetup_FunctionPointer						AddVertexToMeshSetup = nullptr;
    AddTriangleToMeshSetup_FunctionPointer						AddTriangleToMeshSetup = nullptr;


	OverwriteSkeletonSetup_FunctionPointer						OverwriteSkeletonSetup = nullptr;
	OverwriteNodeToSkeletonSetup_FunctionPointer	   	 		OverwriteNodeToSkeletonSetup = nullptr;
	OverwriteChainToSkeletonSetup_FunctionPointer	   	 		OverwriteChainToSkeletonSetup = nullptr;
	GetSkeletonSetupArraySizes_FunctionPointer	   	 			GetSkeletonSetupArraySizes = nullptr;
	AllocateChainsForSkeletonSetup_FunctionPointer	   	 		AllocateChainsForSkeletonSetup = nullptr;
    GetSkeletonSetupInfo_FunctionPointer	   	 				GetSkeletonSetupInfo = nullptr;
	GetSkeletonSetupChains_FunctionPointer	   	 				GetSkeletonSetupChains = nullptr;
	GetSkeletonSetupNodes_FunctionPointer	   	 				GetSkeletonSetupNodes = nullptr;
	LoadSkeleton_FunctionPointer	   	 						LoadSkeleton = nullptr;
	UnloadSkeleton_FunctionPointer	   	 						UnloadSkeleton = nullptr;
	SaveTemporarySkeleton_FunctionPointer	   	 				SaveTemporarySkeleton = nullptr;
	GetTemporarySkeleton_FunctionPointer	   	 				GetTemporarySkeleton = nullptr;
	
	CompressTemporarySkeletonAndGetSize_FunctionPointer	   	 	CompressTemporarySkeletonAndGetSize = nullptr;
	GetCompressedTemporarySkeletonData_FunctionPointer			GetCompressedTemporarySkeletonData = nullptr;
	GetTemporarySkeletonFromCompressedData_FunctionPointer		GetTemporarySkeletonFromCompressedData = nullptr;
};

EManusRet CoreSdk::IsInitialized()
{
	if	(s_FunctionPointers != nullptr) return EManusRet::Success;
	return EManusRet::SDKNotAvailable;
}

EManusRet CoreSdk::Initialize()
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_Initialize);

	if (s_FunctionPointers)
	{
		return EManusRet::Success;
	}

	EManusRet t_ReturnCode = EManusRet::Success;
	FString t_ResultOfError(TEXT("Manus glove support will not be available."));

	s_FunctionPointers = new CoreFunctionPointers();

	// Get a handle to the DLL 
	const FString t_PluginDir = IPluginManager::Get().FindPlugin(TEXT("Manus"))->GetBaseDir();
	FString t_FilePath = FPaths::Combine(*t_PluginDir, TEXT("ThirdParty/Manus/lib"), TEXT(PLATFORM_STRING), TEXT("ManusSdk.dll"));
	if (!FPaths::FileExists(t_FilePath))
	{
		UE_LOG(LogManus, Error, TEXT("Could not find the Manus SDK DLL at %s. %s"), *t_FilePath, *t_ResultOfError);
		t_ReturnCode = EManusRet::SDKNotAvailable;
	}

	s_DllHandle = nullptr;
	if (t_ReturnCode == EManusRet::Success)
	{
		s_DllHandle = FPlatformProcess::GetDllHandle(*t_FilePath);
		if (!s_DllHandle)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to obtain a DLL handle for %s. %s"), *t_FilePath, *t_ResultOfError);
			t_ReturnCode = EManusRet::SDKNotAvailable;
		}
	}

	// Load the SDK functions.
	if (t_ReturnCode == EManusRet::Success)
	{
		s_FunctionPointers->WasDllBuiltInDebugConfiguration =
			(CoreFunctionPointers::WasDllBuiltInDebugConfiguration_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_WasDllBuiltInDebugConfiguration"));
		s_FunctionPointers->GetTimestampInfo =
			(CoreFunctionPointers::GetTimestampInfo_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetTimestampInfo"));
		s_FunctionPointers->SetTimestampInfo =
			(CoreFunctionPointers::SetTimestampInfo_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_SetTimestampInfo"));
		s_FunctionPointers->Initialize =
			(CoreFunctionPointers::Initialize_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_Initialize"));
		s_FunctionPointers->InitializeCoordinateSystemWithVUH =
			(CoreFunctionPointers::InitializeCoordinateSystemWithVUH_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_InitializeCoordinateSystemWithVUH"));
		s_FunctionPointers->ShutDown =
			(CoreFunctionPointers::ShutDown_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_ShutDown"));
		s_FunctionPointers->ConnectGRPC =
			(CoreFunctionPointers::ConnectGRPC_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_ConnectGRPC"));
		s_FunctionPointers->ConnectToHost =
			(CoreFunctionPointers::ConnectToHost_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_ConnectToHost"));
		s_FunctionPointers->LookForHosts =
			(CoreFunctionPointers::LookForHosts_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_LookForHosts"));
		
		s_FunctionPointers->GetNumberOfAvailableHostsFound =
			(CoreFunctionPointers::GetNumberOfAvailableHostsFound_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetNumberOfAvailableHostsFound"));
		s_FunctionPointers->GetAvailableHostsFound =
			(CoreFunctionPointers::GetAvailableHostsFound_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetAvailableHostsFound"));

		s_FunctionPointers->GetIsConnectedToCore =
			(CoreFunctionPointers::GetIsConnectedToCore_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetIsConnectedToCore"));

		s_FunctionPointers->GetSessionId =
			(CoreFunctionPointers::GetSessionId_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetSessionId"));

		s_FunctionPointers->GetVersionsAndCheckCompatibility =
			(CoreFunctionPointers::GetVersionsAndCheckCompatibility_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetVersionsAndCheckCompatibility"));
		s_FunctionPointers->VibrateFingers =
			(CoreFunctionPointers::VibrateFingers_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_VibrateFingers"));
		s_FunctionPointers->VibrateFingersForSkeleton =
			(CoreFunctionPointers::VibrateFingersForSkeleton_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_VibrateFingersForSkeleton"));
		s_FunctionPointers->GetGloveIdOfUser_UsingUserId =
			(CoreFunctionPointers::GetGloveIdOfUser_UsingUserId_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetGloveIdOfUser_UsingUserId"));
						                                    
		s_FunctionPointers->GetNumberOfAvailableUsers =
			(CoreFunctionPointers::GetNumberOfAvailableUsers_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetNumberOfAvailableUsers"));
		s_FunctionPointers->GetIdsOfAvailableUsers =
			(CoreFunctionPointers::GetIdsOfAvailableUsers_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetIdsOfAvailableUsers"));

		s_FunctionPointers->GetNumberOfAvailableGloves =
			(CoreFunctionPointers::GetNumberOfAvailableGloves_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetNumberOfAvailableGloves"));
		s_FunctionPointers->GetIdsOfAvailableGloves =
			(CoreFunctionPointers::GetIdsOfAvailableGloves_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetIdsOfAvailableGloves"));
		s_FunctionPointers->GetDataForGlove_UsingGloveId =
			(CoreFunctionPointers::GetDataForGlove_UsingGloveId_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetDataForGlove_UsingGloveId"));

		
		s_FunctionPointers->GetNumberOfHapticsDongles =
			(CoreFunctionPointers::GetNumberOfHapticsDongles_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetNumberOfHapticsDongles"));
		s_FunctionPointers->GetHapticsDongleIds =
			(CoreFunctionPointers::GetHapticsDongleIds_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetHapticsDongleIds"));

		s_FunctionPointers->GetNumberOfAvailableTrackers =
			(CoreFunctionPointers::GetNumberOfAvailableTrackers_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetNumberOfAvailableTrackers"));
		s_FunctionPointers->GetIdsOfAvailableTrackers =
			(CoreFunctionPointers::GetIdsOfAvailableTrackers_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetIdsOfAvailableTrackers"));
		s_FunctionPointers->GetNumberOfAvailableTrackersForUserId =
			(CoreFunctionPointers::GetNumberOfAvailableTrackersForUserId_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetNumberOfAvailableTrackersForUserId"));
		s_FunctionPointers->GetIdsOfAvailableTrackersForUserId =
			(CoreFunctionPointers::GetIdsOfAvailableTrackersForUserId_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetIdsOfAvailableTrackersForUserId"));
		s_FunctionPointers->GetDataForTracker_UsingTrackerId =
			(CoreFunctionPointers::GetDataForTracker_UsingTrackerId_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetDataForTracker_UsingTrackerId"));
		s_FunctionPointers->GetDataForTracker_UsingIdAndType =
			(CoreFunctionPointers::GetDataForTracker_UsingIdAndType_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetDataForTracker_UsingIdAndType"));
		s_FunctionPointers->SendDataForTrackers =
			(CoreFunctionPointers::SendDataForTrackers_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_SendDataForTrackers"));
		s_FunctionPointers->AssignTrackerToUser =
			(CoreFunctionPointers::AssignTrackerToUser_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_AssignTrackerToUser"));

		

		// Callbacks
		s_FunctionPointers->RegisterCallbackForOnConnect =
			(CoreFunctionPointers::RegisterCallbackForOnConnect_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_RegisterCallbackForOnConnect"));
		s_FunctionPointers->RegisterCallbackForOnDisconnect =
			(CoreFunctionPointers::RegisterCallbackForOnDisconnect_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_RegisterCallbackForOnDisconnect"));

		s_FunctionPointers->RegisterCallbackForSkeletonStream =
			(CoreFunctionPointers::RegisterCallbackForSkeletonStream_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_RegisterCallbackForSkeletonStream"));

		s_FunctionPointers->RegisterCallbackForErgonomicsStream =
			(CoreFunctionPointers::RegisterCallbackForErgonomicsStream_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_RegisterCallbackForErgonomicsStream"));

		s_FunctionPointers->RegisterCallbackForSystemStream =
			(CoreFunctionPointers::RegisterCallbackForSystemStream_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_RegisterCallbackForSystemStream"));
		
        s_FunctionPointers->RegisterCallbackForGestureStream =
            (CoreFunctionPointers::RegisterCallbackForGestureStream_FunctionPointer)
            FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_RegisterCallbackForGestureStream"));

        s_FunctionPointers->RegisterCallbackForLandscapeStream =
            (CoreFunctionPointers::RegisterCallbackForLandscapeStream_FunctionPointer)
            FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_RegisterCallbackForLandscapeStream"));

        s_FunctionPointers->GetSkeletonInfo =
            (CoreFunctionPointers::GetSkeletonInfo_FunctionPointer)
            FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetSkeletonInfo"));

        s_FunctionPointers->GetGestureStreamData =
            (CoreFunctionPointers::GetGestureStreamData_FunctionPointer)
            FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetGestureStreamData"));
        
        s_FunctionPointers->GetGestureLandscapeData =
            (CoreFunctionPointers::GetGestureLandscapeData_FunctionPointer)
            FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetGestureLandscapeData"));

        

        s_FunctionPointers->GetSkeletonData =
			(CoreFunctionPointers::GetSkeletonData_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetSkeletonData"));

		s_FunctionPointers->CreateSkeletonSetup =
			(CoreFunctionPointers::CreateSkeletonSetup_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_CreateSkeletonSetup"));

		s_FunctionPointers->ClearAllTemporarySkeletons =
			(CoreFunctionPointers::ClearAllTemporarySkeletons_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_ClearAllTemporarySkeletons"));

		s_FunctionPointers->ClearTemporarySkeleton =
			(CoreFunctionPointers::ClearTemporarySkeleton_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_ClearTemporarySkeleton"));

		s_FunctionPointers->AddNodeToSkeletonSetup =
			(CoreFunctionPointers::AddNodeToSkeletonSetup_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_AddNodeToSkeletonSetup"));

		s_FunctionPointers->AddChainToSkeletonSetup =
			(CoreFunctionPointers::AddChainToSkeletonSetup_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_AddChainToSkeletonSetup"));


        


        s_FunctionPointers->AddMeshSetupToSkeletonSetup =
            (CoreFunctionPointers::AddMeshSetupToSkeletonSetup_FunctionPointer)
            FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_AddMeshSetupToSkeletonSetup"));

        s_FunctionPointers->AddVertexToMeshSetup =
            (CoreFunctionPointers::AddVertexToMeshSetup_FunctionPointer)
            FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_AddVertexToMeshSetup"));

        s_FunctionPointers->AddTriangleToMeshSetup =
            (CoreFunctionPointers::AddTriangleToMeshSetup_FunctionPointer)
            FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_AddTriangleToMeshSetup"));

        s_FunctionPointers->OverwriteSkeletonSetup =
			(CoreFunctionPointers::OverwriteSkeletonSetup_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_OverwriteSkeletonSetup"));

		s_FunctionPointers->OverwriteNodeToSkeletonSetup =
			(CoreFunctionPointers::OverwriteNodeToSkeletonSetup_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_OverwriteNodeToSkeletonSetup"));

		s_FunctionPointers->OverwriteChainToSkeletonSetup =
			(CoreFunctionPointers::OverwriteChainToSkeletonSetup_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_OverwriteChainToSkeletonSetup"));

		s_FunctionPointers->GetSkeletonSetupArraySizes =
			(CoreFunctionPointers::GetSkeletonSetupArraySizes_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetSkeletonSetupArraySizes"));

		s_FunctionPointers->AllocateChainsForSkeletonSetup =
			(CoreFunctionPointers::AllocateChainsForSkeletonSetup_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_AllocateChainsForSkeletonSetup"));

        
        s_FunctionPointers->GetSkeletonSetupInfo =
            (CoreFunctionPointers::GetSkeletonSetupInfo_FunctionPointer)
            FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetSkeletonSetupInfo"));

		s_FunctionPointers->GetSkeletonSetupChains =
			(CoreFunctionPointers::GetSkeletonSetupChains_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetSkeletonSetupChains"));

		s_FunctionPointers->GetSkeletonSetupNodes =
			(CoreFunctionPointers::GetSkeletonSetupNodes_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetSkeletonSetupNodes"));

		s_FunctionPointers->LoadSkeleton =
			(CoreFunctionPointers::LoadSkeleton_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_LoadSkeleton"));

		s_FunctionPointers->UnloadSkeleton =
			(CoreFunctionPointers::UnloadSkeleton_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_UnloadSkeleton"));

		s_FunctionPointers->SaveTemporarySkeleton =
			(CoreFunctionPointers::SaveTemporarySkeleton_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_SaveTemporarySkeleton"));

		s_FunctionPointers->GetTemporarySkeleton =
			(CoreFunctionPointers::GetTemporarySkeleton_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetTemporarySkeleton"));

		s_FunctionPointers->CompressTemporarySkeletonAndGetSize =
			(CoreFunctionPointers::CompressTemporarySkeletonAndGetSize_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_CompressTemporarySkeletonAndGetSize"));

		s_FunctionPointers->GetCompressedTemporarySkeletonData =
			(CoreFunctionPointers::GetCompressedTemporarySkeletonData_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetCompressedTemporarySkeletonData"));

		s_FunctionPointers->GetTemporarySkeletonFromCompressedData =
			(CoreFunctionPointers::GetTemporarySkeletonFromCompressedData_FunctionPointer)
			FPlatformProcess::GetDllExport(s_DllHandle, TEXT("CoreSdk_GetTemporarySkeletonFromCompressedData"));

		
		if (   !s_FunctionPointers->WasDllBuiltInDebugConfiguration
			|| !s_FunctionPointers->Initialize
			|| !s_FunctionPointers->InitializeCoordinateSystemWithVUH
			|| !s_FunctionPointers->ShutDown
			|| !s_FunctionPointers->ConnectGRPC 
			|| !s_FunctionPointers->ConnectToHost 
			|| !s_FunctionPointers->LookForHosts

			|| !s_FunctionPointers->GetNumberOfAvailableHostsFound
			|| !s_FunctionPointers->GetAvailableHostsFound

			|| !s_FunctionPointers->GetVersionsAndCheckCompatibility
			|| !s_FunctionPointers->VibrateFingers
			|| !s_FunctionPointers->VibrateFingersForSkeleton
			|| !s_FunctionPointers->GetGloveIdOfUser_UsingUserId

			|| !s_FunctionPointers->GetNumberOfAvailableUsers
			|| !s_FunctionPointers->GetIdsOfAvailableUsers

			|| !s_FunctionPointers->GetNumberOfAvailableGloves
			|| !s_FunctionPointers->GetIdsOfAvailableGloves
			|| !s_FunctionPointers->GetNumberOfHapticsDongles
			|| !s_FunctionPointers->GetHapticsDongleIds
			|| !s_FunctionPointers->GetNumberOfAvailableTrackers
			|| !s_FunctionPointers->GetIdsOfAvailableTrackers
			|| !s_FunctionPointers->GetNumberOfAvailableTrackersForUserId
			|| !s_FunctionPointers->GetIdsOfAvailableTrackersForUserId
			|| !s_FunctionPointers->GetDataForTracker_UsingTrackerId
			|| !s_FunctionPointers->GetDataForTracker_UsingIdAndType
			|| !s_FunctionPointers->SendDataForTrackers
			|| !s_FunctionPointers->AssignTrackerToUser

			|| !s_FunctionPointers->RegisterCallbackForOnConnect
			|| !s_FunctionPointers->RegisterCallbackForOnDisconnect
			|| !s_FunctionPointers->RegisterCallbackForSkeletonStream
			|| !s_FunctionPointers->RegisterCallbackForErgonomicsStream
            || !s_FunctionPointers->RegisterCallbackForSystemStream
            || !s_FunctionPointers->RegisterCallbackForGestureStream
            || !s_FunctionPointers->RegisterCallbackForLandscapeStream

            || !s_FunctionPointers->GetGestureStreamData
            || !s_FunctionPointers->GetGestureLandscapeData

			|| !s_FunctionPointers->GetSkeletonInfo
			|| !s_FunctionPointers->CreateSkeletonSetup
			|| !s_FunctionPointers->ClearAllTemporarySkeletons
			|| !s_FunctionPointers->ClearTemporarySkeleton
			|| !s_FunctionPointers->AddNodeToSkeletonSetup
			|| !s_FunctionPointers->AddChainToSkeletonSetup
			

            || !s_FunctionPointers->AddMeshSetupToSkeletonSetup
            || !s_FunctionPointers->AddVertexToMeshSetup
            || !s_FunctionPointers->AddTriangleToMeshSetup

			|| !s_FunctionPointers->OverwriteSkeletonSetup
			|| !s_FunctionPointers->OverwriteNodeToSkeletonSetup
			|| !s_FunctionPointers->OverwriteChainToSkeletonSetup
			|| !s_FunctionPointers->GetSkeletonSetupArraySizes
			|| !s_FunctionPointers->AllocateChainsForSkeletonSetup
            || !s_FunctionPointers->GetSkeletonSetupInfo
			|| !s_FunctionPointers->GetSkeletonSetupChains
			|| !s_FunctionPointers->GetSkeletonSetupNodes
			|| !s_FunctionPointers->LoadSkeleton
			|| !s_FunctionPointers->UnloadSkeleton
			|| !s_FunctionPointers->SaveTemporarySkeleton
			|| !s_FunctionPointers->GetTemporarySkeleton
			|| !s_FunctionPointers->CompressTemporarySkeletonAndGetSize
			|| !s_FunctionPointers->GetCompressedTemporarySkeletonData
			|| !s_FunctionPointers->GetTemporarySkeletonFromCompressedData
			)
			
		{
			UE_LOG(LogManus, Error, TEXT("Failed to get DLL exports for %s. %s"), *t_FilePath, *t_ResultOfError);
			t_ReturnCode = EManusRet::SDKNotAvailable;
		}
	}
	
	if (t_ReturnCode == EManusRet::Success)
	{
		{
			SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_Initialize);
			t_ReturnCode = (EManusRet)s_FunctionPointers->Initialize(SessionType::SessionType_UnrealPlugin);

#if !UE_BUILD_SHIPPING
			bool t_WasDllBuiltInDebug = false;
			s_FunctionPointers->WasDllBuiltInDebugConfiguration(&t_WasDllBuiltInDebug);

			if (t_WasDllBuiltInDebug)
			{
				UE_LOG(LogManus, Warning, TEXT("The DLL was built and included in with a debug configuration, please rebuild the DLL in release"));
			}
#endif
			{ // tbd if we still want/need this.
				//SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_RegisterCallbackForOnConnect);
				//EManusRet RegisterReturn = (EManusRet)s_FunctionPointers->RegisterCallbackForOnConnect(*OnConnectedToCore);
			}

			{
				SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_RegisterCallbackForOnDisconnect);
				EManusRet RegisterReturn = (EManusRet)s_FunctionPointers->RegisterCallbackForOnDisconnect(*OnDisconnectedFromCore);
			}

			{
				SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_RegisterCallbackForSkeletonStream);
				EManusRet RegisterReturn = (EManusRet)s_FunctionPointers->RegisterCallbackForSkeletonStream(*OnSkeletonsCallback);
			}

			{
				SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_RegisterCallbackForErgonomicsStream);
				EManusRet RegisterReturn = (EManusRet)s_FunctionPointers->RegisterCallbackForErgonomicsStream(*OnErgonomicsCallback);
			}

			{
				SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_RegisterCallbackForSystemStream);
				EManusRet RegisterReturn = (EManusRet)s_FunctionPointers->RegisterCallbackForSystemStream(*OnSystemCallback);
			}

            {
                SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_RegisterCallbackForGestureStream);
                EManusRet RegisterReturn = (EManusRet)s_FunctionPointers->RegisterCallbackForGestureStream(*OnGestureCallback);
            }

            {
                SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_RegisterCallbackForLandscapeStream);
                EManusRet RegisterReturn = (EManusRet)s_FunctionPointers->RegisterCallbackForLandscapeStream(*OnLandscapeCallback);
            }
			/* TBD if we want extra data
			{
				SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_RegisterCallbackForOnExtraData);
				EManusRet RegisterReturn = (EManusRet)s_FunctionPointers->RegisterCallbackForOnExtraData(*OnExtraDataCallback);
			}
			*/
		}

		if (t_ReturnCode == EManusRet::Success)
		{
			UE_LOG(LogManus, Warning, TEXT("starting coordinate system initialization"));
			CoordinateSystemVUH t_VUH;
			t_VUH.handedness = Side::Side_Left;
			t_VUH.up = AxisPolarity::AxisPolarity_PositiveZ;
			t_VUH.view = AxisView::AxisView_XFromViewer;
			t_VUH.unitScale = 0.01f;
			
			t_ReturnCode = (EManusRet)s_FunctionPointers->InitializeCoordinateSystemWithVUH(t_VUH , false);
			/// this is an example if you ant to use the other coordinate system instead of VUH
			//CoordinateSystemDirection t_Direction;
			//t_Direction.x = AxisDirection::AD_Right;
			//t_Direction.y = AxisDirection::AD_Up;
			//t_Direction.z = AxisDirection::AD_Forward;
			//const SDKReturnCode t_IntialiseResult = CoreSdk_InitializeCoordinateSystemWithDirection(t_Direction);
		}

		if (t_ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to initialize the Manus DLL. %s"), *t_ResultOfError);
			t_ReturnCode = EManusRet::SDKNotAvailable;
		}
		else
		{
			UE_LOG(LogManus, Log, TEXT("Successfully initialised the Manus DLL."));
		}
	}

	// Clean up.
	if (t_ReturnCode != EManusRet::Success)
	{
		if (s_DllHandle)
		{
			FPlatformProcess::FreeDllHandle(s_DllHandle);
			s_DllHandle = nullptr;

		}

		delete s_FunctionPointers;
		s_FunctionPointers = nullptr;
	}

	return t_ReturnCode;
}

EManusRet CoreSdk::FindRemoteHosts(TArray<FString>& p_Hosts)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_LookForHosts);
	RETURN_IF_NOT_INITIALISED(TEXT("LookForHosts"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	if (t_ReturnCode == EManusRet::Success)
	{
		// first search for hosts.
		t_ReturnCode = (EManusRet)s_FunctionPointers->LookForHosts(s_SecondsToFindHosts, false);
		if (t_ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to look for remote hosts."));
			return t_ReturnCode;
		}

		uint32_t t_NumberOfHostsFound = 0;
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetNumberOfAvailableHostsFound(&t_NumberOfHostsFound);
		if (t_ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to get the number of available remote hosts."));
			return t_ReturnCode;
		}

		if (t_NumberOfHostsFound == 0)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to find any hosts remotely."));
			return EManusRet::NoHostFound;
		}

		ManusHost* t_Hosts = new ManusHost[t_NumberOfHostsFound];
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetAvailableHostsFound(t_Hosts, t_NumberOfHostsFound);
		if (t_ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to get the available remote hosts."));
			return t_ReturnCode;
		}

		if (t_ReturnCode == EManusRet::Success)
		{
			for (uint32_t i = 0; i < t_NumberOfHostsFound; i++)
			{
				FString t_Host = FString(ANSI_TO_TCHAR(t_Hosts[i].hostName));
				FString t_Ip = FString(ANSI_TO_TCHAR(t_Hosts[i].ipAddress));
				
                if (!t_Ip.IsEmpty()) // there is always an address. if not. what would hostname even be pointing too ?
                {
                    p_Hosts.Add(t_Ip); // this will cause duplicates. BUT they will also have clarity if they recognize on or the other value.
                    if (!t_Host.IsEmpty())
                    {
                        p_Hosts.Add(t_Host);
                    }
                    else // however, there is not always a host name. very annoying.
                    {
                        p_Hosts.Add(t_Ip); // add the address again 
                    }
                }
			}
		}
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::ConnectRemotely(FString p_Host)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_ConnectToHost);
	RETURN_IF_NOT_INITIALISED(TEXT("ConnectToHost"), EManusRet::Error);
    
    UE_LOG(LogManus, Warning, TEXT("start to connect remotely for SDK"));
	
    EManusRet t_ReturnCode = EManusRet::Success;
	if (t_ReturnCode == EManusRet::Success)
	{
		uint32_t t_NumberOfHostsFound = 0;
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetNumberOfAvailableHostsFound(&t_NumberOfHostsFound);
		if (t_ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to get the number of available hosts."));
			return t_ReturnCode;
		}

		if (t_NumberOfHostsFound == 0)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to find any hosts. Manus glove support will not be available."));
			return EManusRet::NoHostFound;
		}

		ManusHost* t_Hosts = new ManusHost[t_NumberOfHostsFound];
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetAvailableHostsFound(t_Hosts, t_NumberOfHostsFound);
		if (t_ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to get the available hosts."));
			return t_ReturnCode;
		}
		if (t_ReturnCode == EManusRet::Success)
		{
			int t_Index = -1;
			// verify if our given p_Host matches any of the hosts found.
			for (uint32_t i = 0; i < t_NumberOfHostsFound; i++)
			{
				FString t_Host = FString(ANSI_TO_TCHAR(t_Hosts[i].hostName));
				FString t_Ip = FString(ANSI_TO_TCHAR(t_Hosts[i].ipAddress));
				if ((t_Host.Equals(p_Host, ESearchCase::IgnoreCase)) ||
					(t_Ip.Equals(p_Host, ESearchCase::IgnoreCase)))
				{
					t_Index = (int)i;
					break;
				}
			}
			if (t_Index == -1)
			{
				// no host found.
				UE_LOG(LogManus, Error, TEXT("Failed to find host to Manus-Core remotely. Manus glove support will not be available."));
				return EManusRet::NoHostFound;
			}

			SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_Initialize);
			t_ReturnCode = (EManusRet)s_FunctionPointers->ConnectToHost(t_Hosts[t_Index]);
			return t_ReturnCode;
		}

		if (t_ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to connect to Manus-Core remotely. Manus glove support will not be available."));
		}
		else
		{
			UE_LOG(LogManus, Log, TEXT("Successfully connected to Manus-Core remotely."));
		}
	}

	return t_ReturnCode;
}

EManusRet CoreSdk::ConnectLocally() // TODO simplify this with the connectremotely version ?
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_ConnectToHost);
	RETURN_IF_NOT_INITIALISED(TEXT("ConnectToHost"), EManusRet::Error);

    UE_LOG(LogManus, Warning, TEXT("start to connect locally for SDK"));

	EManusRet t_ReturnCode = EManusRet::Success;
	if (t_ReturnCode == EManusRet::Success)
	{
		// first search for local host.

		t_ReturnCode = (EManusRet)s_FunctionPointers->LookForHosts(s_SecondsToFindHosts, true);
		if (t_ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to look for hosts."));
			return t_ReturnCode;
		}

		uint32_t t_NumberOfHostsFound = 0;
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetNumberOfAvailableHostsFound(&t_NumberOfHostsFound);
		if (t_ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to get the number of available hosts."));
			return t_ReturnCode;
		}

		if (t_NumberOfHostsFound == 0)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to find any local hosts. Manus glove support will not be available."));
			return EManusRet::NoHostFound;
		}

		ManusHost* t_Hosts = new ManusHost[t_NumberOfHostsFound];
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetAvailableHostsFound(t_Hosts, t_NumberOfHostsFound);
		if (t_ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to get the available hosts."));
			return t_ReturnCode;
		}

		if (t_ReturnCode == EManusRet::Success)
		{
			SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_Initialize); 
			t_ReturnCode = (EManusRet)s_FunctionPointers->ConnectToHost(t_Hosts[0]);
		}

		if (t_ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to connect to Manus-Core locally. Manus glove support will not be available."));
		}
		else
		{
			UE_LOG(LogManus, Log, TEXT("Successfully connected to Manus-Core locally."));
		}
	}

	return t_ReturnCode;
}

EManusRet CoreSdk::ShutDown()
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_ShutDown);

	UE_LOG(LogManus, Warning, TEXT("trying to shut down the Manus DLL."));

	EManusRet t_ReturnCode = EManusRet::Success;
	if (!s_FunctionPointers)
	{
		return t_ReturnCode;
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_ShutDown);
		t_ReturnCode = (EManusRet)s_FunctionPointers->ShutDown();
	}

	if (t_ReturnCode != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to shut down the Manus DLL."));
	}

	UE_LOG(LogManus, Warning, TEXT("sdk is shutdown."));

	delete s_FunctionPointers;
	s_FunctionPointers = nullptr;
	
	UE_LOG(LogManus, Warning, TEXT("function pointers are gone. freeing sdk dll now."));

	if (s_DllHandle)
	{
		//FPlatformProcess::FreeDllHandle(s_DllHandle); 
		// the dll handle cannot immediately be deleted. After a lot of checking we did see everything shutdown correctly in the SDK dll(and no other external calls being made).
		// however, it too depends upon several external libraries that may lag behind. so it cannot directly be freed
		// however, the dll cleans up a few seconds later anyway (due to its shutdown call) and no memory is being leaked even after multiple re-activations
		// IRL this is not an issue.		

		s_DllHandle = nullptr;
	}
	UE_LOG(LogManus, Warning, TEXT("sdk dll is gone."));

	return t_ReturnCode;
}

EManusRet CoreSdk::CheckConnection()
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetIsConnectedToCore);
    RETURN_IF_NOT_INITIALISED_SILENT(TEXT("GetIsConnectedToCore"), EManusRet::Error); // this can happen when disconnected, so do not spam

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetIsConnectedToCore);
		bool t_IsConnected = false;
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetIsConnectedToCore(&t_IsConnected);
		if ((t_ReturnCode == EManusRet::Success) && !t_IsConnected)
		{
			t_ReturnCode = EManusRet::NotConnected;
		}
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::GetSessionId(uint32_t& p_SessionId)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetSessionId);
	RETURN_IF_NOT_INITIALISED(TEXT("GetSessionId"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetSessionId);
		bool t_IsConnected = false;
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetSessionId(&p_SessionId);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::CheckCompatibility()
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetVersionsAndCheckCompatibility);
	RETURN_IF_NOT_INITIALISED(TEXT("GetVersionsAndCheckCompatibility"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetVersionsAndCheckCompatibility);

		ManusVersion t_SdkVersion;
		ManusVersion t_CoreVersion;
		bool t_IsCompatible = false;

		t_ReturnCode = (EManusRet)s_FunctionPointers->GetVersionsAndCheckCompatibility(&t_SdkVersion, &t_CoreVersion, &t_IsCompatible);

		if (t_ReturnCode == EManusRet::Success)
		{
			FString t_PluginVersion;
			FPluginDescriptor t_PluginData;
			if (FManusModule::GetPluginData(t_PluginData))
			{
				t_PluginVersion = "v" + t_PluginData.VersionName;
			}
			else 
			{
				t_PluginVersion = "Invalid";
			}

			FString t_Version =
				" Plugin Version: " + t_PluginVersion +
				", Sdk Version: " + FString(t_SdkVersion.versionInfo) +
				", Core version: " + FString(t_CoreVersion.versionInfo) + ".";


			// Check plugin version compatibility with connected core
			if (t_IsCompatible)
			{
				UE_LOG(LogManus, Log, TEXT("Versions are compatible. %s"), *t_Version);
			}
			else 
			{
				UE_LOG(LogManus, Warning, TEXT("Versions are not compatible. %s"), *t_Version);
#if UE_BUILD_SHIPPING
				FText DialogTitle = FText::FromString("Versions uncompatible");
				FText DialogText = FText::FromString("The plugin version is not compatible with the currently connected Manus Core. Please make sure the versions match" + t_Version);
				FMessageDialog::Open(EAppMsgType::Ok, DialogText, &DialogTitle);
#endif // !DEBUG
			}
		}
	}

	return t_ReturnCode;
}

EManusRet CoreSdk::VibrateFingers(int64 p_DongleId, EManusHandType p_HandTypeOfGlove, TArray<float> p_Powers)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_VibrateFingers);
	RETURN_IF_NOT_INITIALISED(TEXT("VibrateFingers"), EManusRet::Error);

	if (p_DongleId == 0)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Tried to vibrate the fingers of the glove using a dongle with ID %u, but this dongle ID is not valid.")
			, static_cast<unsigned int>(p_DongleId));

		return EManusRet::InvalidArgument;
	}

	Side t_SdkHandType;
	EManusRet t_ReturnCode = ManusConvert::BpHandTypeToSdk(p_HandTypeOfGlove, t_SdkHandType);
	if (t_ReturnCode != EManusRet::Success)
	{
		return t_ReturnCode;
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_VibrateFingers);
		t_ReturnCode = (EManusRet)s_FunctionPointers->VibrateFingers(p_DongleId, t_SdkHandType, p_Powers.GetData());
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to rumble the fingers of the glove of hand type %s using a dongle with ID %u.")
			, (p_HandTypeOfGlove == EManusHandType::Left ? TEXT("left") : TEXT("right"))
			, static_cast<unsigned int>(p_DongleId));
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::VibrateFingersForSkeleton(int64 p_SkeletonId, EManusHandType p_HandTypeOfGlove, TArray<float> p_Powers)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_VibrateFingersForSkeleton);
	RETURN_IF_NOT_INITIALISED(TEXT("VibrateFingersForSkeleton"), EManusRet::Error);

	if (p_SkeletonId == 0)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Tried to vibrate the fingers of the glove for skeleton with ID %u, but this skeleton ID is not valid.")
			, static_cast<unsigned int>(p_SkeletonId));

		return EManusRet::InvalidArgument;
	}

	Side t_SdkHandType;
	EManusRet t_ReturnCode = ManusConvert::BpHandTypeToSdk(p_HandTypeOfGlove, t_SdkHandType);
	if (t_ReturnCode != EManusRet::Success)
	{
		return t_ReturnCode;
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_VibrateFingersForSkeleton);
		t_ReturnCode = (EManusRet)s_FunctionPointers->VibrateFingersForSkeleton(p_SkeletonId, t_SdkHandType, p_Powers.GetData());
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to rumble the fingers of the glove of hand type %s for the skeleton with ID %u.")
			, (p_HandTypeOfGlove == EManusHandType::Left ? TEXT("left") : TEXT("right"))
			, static_cast<unsigned int>(p_SkeletonId));
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::GetGloveIdOfUser_UsingUserIndex(int32 p_UserIndex, EManusHandType p_HandTypeOfGlove, int64& p_GloveId)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetGloveIdOfUser_UsingUserId);
	RETURN_IF_NOT_INITIALISED(TEXT("GetGloveIdOfUser_UsingUserId"), EManusRet::Error);

	// temp solution. get lists of users, and pick the user from that list with index. this is not safe and should be reworked.
	// for 99% of customers this works for now, but when using multiple users, will quickly get borked.
	TArray<int64> t_IdsOfUsers;
	EManusRet t_ReturnCode = GetIdsOfUsers(t_IdsOfUsers);
	if (t_ReturnCode != EManusRet::Success)
	{
		// An error message will be logged in the convert function, so don't print anything here.
		return t_ReturnCode;
	}

	if (t_IdsOfUsers.Num() < (p_UserIndex + 1) )
	{
		return EManusRet::DataNotAvailable;
	}

	Side t_SdkHandType;
	t_ReturnCode = ManusConvert::BpHandTypeToSdk(p_HandTypeOfGlove, t_SdkHandType);
	if (t_ReturnCode != EManusRet::Success)
	{
		// An error message will be logged in the convert function, so don't print anything here.
		return t_ReturnCode;
	}

	uint32_t t_Id = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetGloveIdOfUser_UsingUserId);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetGloveIdOfUser_UsingUserId(t_IdsOfUsers[p_UserIndex], t_SdkHandType, &t_Id); // hack.. so much hack
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		return t_ReturnCode;
	}

	p_GloveId = t_Id;

	return EManusRet::Success;
}

// this is the preferred way of getting the data.
EManusRet CoreSdk::GetGloveIdOfUser_UsingUserId(int32 UserId, EManusHandType p_HandTypeOfGlove, int64& p_GloveId)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetGloveIdOfUser_UsingUserId);
	RETURN_IF_NOT_INITIALISED(TEXT("GetGloveIdOfUser_UsingUserId"), EManusRet::Error);

	Side t_SdkHandType;
	EManusRet t_ReturnCode = ManusConvert::BpHandTypeToSdk(p_HandTypeOfGlove, t_SdkHandType);
	if (t_ReturnCode != EManusRet::Success)
	{
		// An error message will be logged in the convert function, so don't print anything here.
		return t_ReturnCode;
	}

	uint32_t Id = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetGloveIdOfUser_UsingUserId);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetGloveIdOfUser_UsingUserId(UserId, t_SdkHandType, &Id);
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		return t_ReturnCode;
	}

	p_GloveId = Id;

	return EManusRet::Success;
}

EManusRet CoreSdk::GetNumberOfAvailableUsers(int32& NumberOfUsers)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetNumberOfAvailableUsers);
	RETURN_IF_NOT_INITIALISED(TEXT("GetNumberOfAvailableUsers"), EManusRet::Error); // sus

	EManusRet t_ReturnCode = EManusRet::Success;
	uint32_t t_NumUsers = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetNumberOfAvailableUsers);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetNumberOfAvailableUsers(&t_NumUsers);
	}

	NumberOfUsers = static_cast<int32>(t_NumUsers);

	return t_ReturnCode;
}

// redo this without the requirement of pre allocating the idsofusers collection?
EManusRet CoreSdk::GetIdsOfUsers(TArray<int64>& IdsOfUsers) // optimize this and lose the above function...
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetIdsOfAvailableUsers);
	RETURN_IF_NOT_INITIALISED(TEXT("GetIdsOfAvailableUsers"), EManusRet::Error);
	/*
	if (IdsOfUsers.Num() != 0)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Attempted to get an array of glove IDs with a non-empty array."));

		return EManusRet::InvalidArgument;
	}
	*/
	EManusRet t_ReturnCode = EManusRet::Success;
	uint32_t NumUsers = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetNumberOfAvailableUsers);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetNumberOfAvailableUsers(&NumUsers);
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the number of available users from the SDK."));

		return t_ReturnCode;
	}

	if (NumUsers == 0)
	{
		// Nothing left to do here, so just return.
		return EManusRet::Success;
	}

	IdsOfUsers.Reserve(NumUsers);

	uint32_t* t_IdsOfUsers = new uint32_t[NumUsers];
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetIdsOfAvailableUsers);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetIdsOfAvailableUsers(t_IdsOfUsers, NumUsers);
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the IDs of available users from the SDK."));

		return t_ReturnCode;
	}

	IdsOfUsers.Reset(static_cast<int32>(NumUsers));

	for (uint32_t Id = 0; Id < NumUsers; Id++)
	{
		IdsOfUsers.Add(t_IdsOfUsers[Id]);
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::GetNumberOfAvailableGloves(int32 &NumberOfAvailableGloves)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetNumberOfAvailableGloves);
	RETURN_IF_NOT_INITIALISED(TEXT("GetNumberOfAvailableGloves"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	uint32_t NumGloves = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetNumberOfAvailableGloves);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetNumberOfAvailableGloves(&NumGloves);
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		return EManusRet::Error;
	}

	NumberOfAvailableGloves = static_cast<int32>(NumGloves);
	
	return EManusRet::Success;
}

EManusRet CoreSdk::GetIdsOfAvailableGloves(TArray<int64> &IdsOfAvailableGloves)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetIdsOfAvailableGloves);
	RETURN_IF_NOT_INITIALISED(TEXT("GetIdsOfAvailableGloves"), EManusRet::Error);

	if (IdsOfAvailableGloves.Num() != 0)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Attempted to get an array of glove IDs with a non-empty array."));

		return EManusRet::InvalidArgument;
	}

	EManusRet t_ReturnCode = EManusRet::Success;
	uint32_t NumGloves = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetNumberOfAvailableGloves);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetNumberOfAvailableGloves(&NumGloves);
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the number of available gloves from the SDK."));

		return t_ReturnCode;
	}

	if (NumGloves == 0)
	{
		// Nothing left to do here, so just return.

		return EManusRet::Success;
	}

	uint32_t *IdsOfGloves = new uint32_t[NumGloves];
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetIdsOfAvailableGloves);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetIdsOfAvailableGloves(IdsOfGloves, NumGloves);
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the IDs of available gloves from the SDK."));

		return t_ReturnCode;
	}

	IdsOfAvailableGloves.Reset(static_cast<int32>(NumGloves));

	for (uint32_t Id = 0; Id < NumGloves; Id++)
	{
		IdsOfAvailableGloves.Add(IdsOfGloves[Id]);
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::GetDataForGlove_UsingGloveId(uint32_t p_GloveId, GloveLandscapeData& p_GloveData)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetDataForGlove_UsingGloveId);
	RETURN_IF_NOT_INITIALISED(TEXT("GetDataForGlove_UsingGloveId"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetDataForGlove_UsingGloveId);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetDataForGlove_UsingGloveId(p_GloveId, &p_GloveData);
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		return t_ReturnCode;
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::GetNumberOfHapticDongles(int32& NumberOfHapticsDongles)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetNumberOfHapticsDongles);
	RETURN_IF_NOT_INITIALISED(TEXT("GetNumberOfHapticsDongles"), EManusRet::Error);

	uint32_t NumHapticsDongles = 0;
	EManusRet t_ReturnCode = (EManusRet)s_FunctionPointers->GetNumberOfHapticsDongles(&NumHapticsDongles);
	if (t_ReturnCode != EManusRet::Success)
	{
		return t_ReturnCode;
	}

	NumberOfHapticsDongles = static_cast<int32>(NumHapticsDongles);

	return EManusRet::Success;
}

EManusRet CoreSdk::GetHapticDongleIds(TArray<int64>& HapticsDongleIds)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetHapticsDongleIds);
	RETURN_IF_NOT_INITIALISED(TEXT("GetHapticsDongleIds"), EManusRet::Error);

	if (HapticsDongleIds.Num() != 0)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Attempted to get an array of haptic dongle IDs with a non-empty array."));

		return EManusRet::InvalidArgument;
	}

	EManusRet t_ReturnCode = EManusRet::Success;
	uint32_t NumHapticsDongleIds = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetNumberOfHapticsDongles);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetNumberOfHapticsDongles(&NumHapticsDongleIds);
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the number of haptic dongle ids from the SDK."));

		return t_ReturnCode;
	}

	if (NumHapticsDongleIds == 0)
	{
		return EManusRet::Success;
	}

	uint32_t* IdsOfHapticsDongles = new uint32_t[NumHapticsDongleIds];
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetHapticsDongleIds);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetHapticsDongleIds(IdsOfHapticsDongles, NumHapticsDongleIds);
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the IDs of available gloves from the SDK."));

		return t_ReturnCode;
	}

	HapticsDongleIds.Reset(static_cast<int32>(NumHapticsDongleIds));

	for (uint32_t Id = 0; Id < NumHapticsDongleIds; Id++)
	{
		HapticsDongleIds.Add(IdsOfHapticsDongles[Id]);
	}

	return EManusRet::Success;
}

bool CoreSdk::GetGloveRotationForSkeletonNode(int64 p_SkeletonId, int p_NodeId, ManusQuaternion& p_Rotation)
{
    bool t_Found = false;
    s_CriticalSectionSkeletons->Lock();

    if (s_NextSkeletons != nullptr)
    {
        for (int i = 0; i < s_NextSkeletons->skeletons.size(); i++)
        {
            if (s_NextSkeletons->skeletons[i].info.id == p_SkeletonId) 
            {
                for (uint32_t j = 0; j < s_NextSkeletons->skeletons[i].info.nodesCount; j++)
                {
                    if (s_NextSkeletons->skeletons[i].nodes[j].id == p_NodeId)
                    {
                        p_Rotation.w = s_NextSkeletons->skeletons[i].nodes[j].transform.rotation.w;
                        p_Rotation.x = s_NextSkeletons->skeletons[i].nodes[j].transform.rotation.x;
                        p_Rotation.y = s_NextSkeletons->skeletons[i].nodes[j].transform.rotation.y;
                        p_Rotation.z = s_NextSkeletons->skeletons[i].nodes[j].transform.rotation.z;
                        t_Found = true;
                        break;
                    }
                }
            }
            if (t_Found) break;
        }
    }

    s_CriticalSectionSkeletons->Unlock();
    return t_Found;
}


EManusRet CoreSdk::GetDataForSkeleton(int64 SkeletonId, FManusMetaSkeleton& DataForSkeleton)
{
	RETURN_IF_NOT_INITIALISED(TEXT("GetDataForSkeleton"), EManusRet::Error);
	
	if (SkeletonId == 0)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Tried to get data for a skeleton with ID %u, but this skeleton ID is not valid.")
			, static_cast<unsigned int>(SkeletonId));

		return EManusRet::InvalidArgument;
	}

	EManusRet t_ReturnCode = EManusRet::Success;

	s_CriticalSectionSkeletons->Lock();

	if (s_NextSkeletons == nullptr)
	{
		s_CriticalSectionSkeletons->Unlock();
		return EManusRet::FunctionCalledAtWrongTime;
	}
	ClientSkeleton t_Data;
	bool t_Result = s_NextSkeletons->CopySkeleton(SkeletonId, &t_Data);

	s_CriticalSectionSkeletons->Unlock();

	if (!t_Result)
	{
        /*
        UE_LOG(
            LogManus
            , Warning
            , TEXT("Tried to get data for a skeleton with ID %u, but there is no data.")
            , static_cast<unsigned int>(SkeletonId));
        */
		return EManusRet::DataNotAvailable;
	}

	if (!ConvertSdkSkeletonDataToBp(t_Data, DataForSkeleton))
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed convert CoreSDK skeleton data to blueprint-compatible skeleton data for the skeleton with ID %u.")
			, static_cast<unsigned int>(SkeletonId));

		return EManusRet::Error;
	}

	return EManusRet::Success;
	
}

EManusRet CoreSdk::GetTrackerIds(TArray<FString>& p_TrackerIds)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetNumberOfAvailableTrackersForUserId);
	RETURN_IF_NOT_INITIALISED(TEXT("GetNumberOfAvailableTrackersForUserId"), EManusRet::Error);
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetIdsOfAvailableTrackersForUserId);
	RETURN_IF_NOT_INITIALISED(TEXT("GetIdsOfAvailableTrackersForUserId"), EManusRet::Error);


	uint32_t t_TrackerCount = 0;
	EManusRet t_ReturnCode = (EManusRet)s_FunctionPointers->GetNumberOfAvailableTrackers(&t_TrackerCount);
	
	if (t_ReturnCode != EManusRet::Success)
	{
		return t_ReturnCode;
	}
	if (t_TrackerCount == 0) return EManusRet::DataNotAvailable; 

	TrackerId* t_TrackerIds = new TrackerId[t_TrackerCount];

	t_ReturnCode = (EManusRet)s_FunctionPointers->GetIdsOfAvailableTrackers(t_TrackerIds, t_TrackerCount);
	if (t_ReturnCode != EManusRet::Success)
	{
		delete[] t_TrackerIds;
		return t_ReturnCode;
	}

	for (uint32_t i = 0; i < t_TrackerCount; i++)
	{
		p_TrackerIds.Add(FString(ANSI_TO_TCHAR(t_TrackerIds[i].id)));
	}

	delete[] t_TrackerIds;
	return t_ReturnCode;
}

EManusRet CoreSdk::AssignTrackerToUser(char* p_TrackerId, int64 p_UserId)
{
	EManusRet t_ReturnCode = EManusRet::Error;

	t_ReturnCode = (EManusRet)s_FunctionPointers->AssignTrackerToUser(p_TrackerId,(uint32_t)p_UserId);
	if (t_ReturnCode != EManusRet::Success)
	{
		return t_ReturnCode;
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::GetDataForTracker(uint32 p_UserId, EManusHandType HandTypeOfTracker, FManusTracker& DataForTracker)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetNumberOfAvailableTrackersForUserId);
	RETURN_IF_NOT_INITIALISED(TEXT("GetNumberOfAvailableTrackersForUserId"), EManusRet::Error);
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetIdsOfAvailableTrackersForUserId);
	RETURN_IF_NOT_INITIALISED(TEXT("GetIdsOfAvailableTrackersForUserId"), EManusRet::Error);
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetDataForTracker_UsingTrackerId);
	RETURN_IF_NOT_INITIALISED(TEXT("GetDataForTracker_UsingTrackerId"), EManusRet::Error);
	
	uint32_t SdkTrackerType;
	EManusRet t_ReturnCode = ManusConvert::BpHandTypeToSdkTrackerType(HandTypeOfTracker, SdkTrackerType);
	if (t_ReturnCode != EManusRet::Success)
	{
		return t_ReturnCode;
	}

	TrackerData Data;
	uint32_t t_TrackerCount = 0;
	{
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetNumberOfAvailableTrackersForUserId(&t_TrackerCount, p_UserId);
		if (t_ReturnCode != EManusRet::Success)
		{
			return t_ReturnCode;
		}
		if (t_TrackerCount == 0) return EManusRet::DataNotAvailable; // temp hack

		TrackerId* t_TrackerIds = new TrackerId[t_TrackerCount];

		t_ReturnCode = (EManusRet)s_FunctionPointers->GetIdsOfAvailableTrackersForUserId(t_TrackerIds, p_UserId, t_TrackerCount);
		if (t_ReturnCode != EManusRet::Success)
		{
			return t_ReturnCode;
		}

		int t_TrackerIndex = -1;
		for (size_t i = 0; i < t_TrackerCount; i++)
		{
			t_TrackerIndex = i;

			t_ReturnCode = (EManusRet)s_FunctionPointers->GetDataForTracker_UsingTrackerId(t_TrackerIds[t_TrackerIndex], &Data);
			if (t_ReturnCode == EManusRet::Success)
			{
				if (ConvertSdkTrackerDataToBp(Data, DataForTracker))
				{
					if ((HandTypeOfTracker == EManusHandType::Left) &&
						(Data.trackerType == TrackerType::TrackerType_LeftHand))
					{
						return EManusRet::Success;
					}
					else if ((HandTypeOfTracker == EManusHandType::Right) &&
						(Data.trackerType == TrackerType::TrackerType_RightHand))
					{
						return EManusRet::Success;
					}
				}
				else
				{
					// soft error
					break;
				}
			}
			else
			{
				return t_ReturnCode;
			}

		}
		if (t_TrackerIndex == -1) return EManusRet::InvalidArgument;
	}
	return EManusRet::DataNotAvailable;
	
}

bool ConvertSdkSkeletonDataToBp(const ClientSkeleton& p_SdkInput, FManusMetaSkeleton& p_BpOutput)
{
	//BpOutput.LastUpdateTime = SdkInput.lastUpdateTime; // TODO is this important? probably. but currently unused. so not that important.

	p_BpOutput.Bones.SetNum(p_SdkInput.info.nodesCount);
	
	for (uint32_t t_BoneIndex = 0; t_BoneIndex < p_SdkInput.info.nodesCount; t_BoneIndex++)
	{
		FManusBone NewBone;

		NewBone.Validity = true;

		float t_Scale = p_SdkInput.nodes[t_BoneIndex].transform.scale.x *
						p_SdkInput.nodes[t_BoneIndex].transform.scale.y *
						p_SdkInput.nodes[t_BoneIndex].transform.scale.z;

		FVector t_ScaleVec(1.0f, 1.0f, 1.0f);
		if (t_Scale > 0.0f)
		{
			t_ScaleVec.X = p_SdkInput.nodes[t_BoneIndex].transform.scale.x;
			t_ScaleVec.Y = p_SdkInput.nodes[t_BoneIndex].transform.scale.y;
			t_ScaleVec.Z = p_SdkInput.nodes[t_BoneIndex].transform.scale.z;
		}

        NewBone.Transform = FTransform();
        NewBone.Transform.SetLocation(FVector(p_SdkInput.nodes[t_BoneIndex].transform.position.x,
            p_SdkInput.nodes[t_BoneIndex].transform.position.y,
            p_SdkInput.nodes[t_BoneIndex].transform.position.z));
        NewBone.Transform.SetRotation(FQuat(p_SdkInput.nodes[t_BoneIndex].transform.rotation.x,
            p_SdkInput.nodes[t_BoneIndex].transform.rotation.y,
            p_SdkInput.nodes[t_BoneIndex].transform.rotation.z,
            p_SdkInput.nodes[t_BoneIndex].transform.rotation.w));
        NewBone.Transform.SetScale3D(t_ScaleVec);

		NewBone.BoneId = p_SdkInput.nodes[t_BoneIndex].id;
		p_BpOutput.Bones[t_BoneIndex] = NewBone;
	}

	p_BpOutput.SkeletonId = p_SdkInput.info.id;
	p_BpOutput.LastUpdateTime = p_SdkInput.info.publishTime.time;
	return true;
}

bool ConvertSdkTrackerDataToBp(const TrackerData& p_SdkInput, FManusTracker& p_BpOutput)
{
	p_BpOutput.LastUpdateTime = p_SdkInput.lastUpdateTime.time; // going from uint64 to 64. because unreal does not support uint64 in blueprints. reee

	p_BpOutput.TrackerId = FString(ANSI_TO_TCHAR(p_SdkInput.trackerId.id));

	p_BpOutput.Transform = FTransform(
		FQuat(p_SdkInput.rotation.x, p_SdkInput.rotation.y, p_SdkInput.rotation.z, p_SdkInput.rotation.w),
		FVector(p_SdkInput.position.x, p_SdkInput.position.y, p_SdkInput.position.z));

	p_BpOutput.UserId = p_SdkInput.userId;

	if (ManusConvert::SdkTrackerTypeToBpHandType(p_SdkInput.trackerType, p_BpOutput.HandType) != EManusRet::Success)
	{
		return false;
	}

	return true;
}

// Callbacks
void OnConnectedToCore()
{
	UE_LOG(LogManus, Log, TEXT("Connected to Manus Core")); // todo move this to after login.
	
	CoreSdk::CheckCompatibility();
}

void CoreSdk::OnDisconnectedFromCore(const ManusHost* const p_Host)
{
	UE_LOG(LogManus, Log, TEXT("Disconnected from Manus Core")); 
}

void CoreSdk::OnSkeletonsCallback(const SkeletonStreamInfo* const p_Skeletons)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetSkeletonInfo);
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetSkeletonData);

	ClientSkeletonCollection* t_NxtSkeletons = new ClientSkeletonCollection();
	t_NxtSkeletons->skeletons.resize(p_Skeletons->skeletonsCount);
	
    //UE_LOG(LogManus, Warning, TEXT("getting SkeletonsData during callback")); // this can get a bit spammy.

	// Get the timestamp info just for easier interpretation
	ManusTimestampInfo t_Info;
	(EManusRet)s_FunctionPointers->GetTimestampInfo(p_Skeletons->publishTime, &t_Info);

	EManusRet t_ReturnCode = EManusRet::Success;
	for (uint32_t i = 0; i < p_Skeletons->skeletonsCount; i++)
	{
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetSkeletonInfo(i, &t_NxtSkeletons->skeletons[i].info);
		if (t_ReturnCode != EManusRet::Success) break;

       // UE_LOG(LogManus, Warning, TEXT("getting SkeletonsData during callback A %u"), t_NxtSkeletons->skeletons[i].info.id); // this can get a bit spammy.

		t_NxtSkeletons->skeletons[i].nodes = new SkeletonNode[t_NxtSkeletons->skeletons[i].info.nodesCount];
		t_NxtSkeletons->skeletons[i].info.publishTime = p_Skeletons->publishTime;
		
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetSkeletonData(i, t_NxtSkeletons->skeletons[i].nodes, t_NxtSkeletons->skeletons[i].info.nodesCount);
		if (t_ReturnCode != EManusRet::Success) break;
		/* // leaving this in, cause we DID see some weird stuff and this is a quick and dirty yet fast check to see it happen.
		// do a quick test that the data is valid. if we ever get the identity rotation consider it bad. you can never get that with a normal use anyway.
		if ((t_NxtSkeletons->skeletons[i].nodes[0].transform.rotation.x == 0.0f) &&
			(t_NxtSkeletons->skeletons[i].nodes[0].transform.rotation.y == 0.0f) &&
			(t_NxtSkeletons->skeletons[i].nodes[0].transform.rotation.z == 0.0f) &&
			(t_NxtSkeletons->skeletons[i].nodes[0].transform.rotation.w == 1.0f))
		{
			//t_ReturnCode = EManusRet::DataNotAvailable;
		}
		*/
        //UE_LOG(LogManus, Warning, TEXT("getting SkeletonsData during callback B %u") , t_NxtSkeletons->skeletons[i].info.id); // this can get a bit spammy.
	}
	if (t_ReturnCode != EManusRet::Success)
	{
		// log error
		UE_LOG(LogManus, Warning, TEXT("Error getting SkeletonsData during callback")); // this can get a bit spammy.
		return;
	}

	s_CriticalSectionSkeletons->Lock();

	if (s_NextSkeletons != nullptr) delete s_NextSkeletons;
	s_NextSkeletons = t_NxtSkeletons;

	s_CriticalSectionSkeletons->Unlock();
}

EManusRet CoreSdk::CreateSkeletonSetup(const SkeletonSetupInfo& p_Skeleton, uint32_t& p_SkeletonSetupIndex)
{
	RETURN_IF_NOT_INITIALISED(TEXT("CreateSkeletonSetup"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_CreateSkeletonSetup);
		t_ReturnCode = (EManusRet)s_FunctionPointers->CreateSkeletonSetup(p_Skeleton, &p_SkeletonSetupIndex);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::ClearAllTemporarySkeletons()
{
	RETURN_IF_NOT_INITIALISED(TEXT("ClearAllTemporarySkeletons"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_ClearAllTemporarySkeletons);
		t_ReturnCode = (EManusRet)s_FunctionPointers->ClearAllTemporarySkeletons();
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::ClearTemporarySkeleton(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId)
{
	RETURN_IF_NOT_INITIALISED(TEXT("ClearTemporarySkeleton"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_ClearTemporarySkeleton);
		t_ReturnCode = (EManusRet)s_FunctionPointers->ClearTemporarySkeleton(p_SkeletonSetupIndex, p_SessionId);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::AddNodeToSkeletonSetup(uint32_t p_SkeletonSetupIndex, const NodeSetup& p_Node)
{
	RETURN_IF_NOT_INITIALISED(TEXT("AddNodeToSkeletonSetup"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_AddNodeToSkeletonSetup);
		t_ReturnCode = (EManusRet)s_FunctionPointers->AddNodeToSkeletonSetup(p_SkeletonSetupIndex, p_Node);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::AddChainToSkeletonSetup(uint32_t p_SkeletonSetupIndex, const ChainSetup& p_Chain)
{
	RETURN_IF_NOT_INITIALISED(TEXT("AddChainToSkeletonSetup"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_AddChainToSkeletonSetup);
		t_ReturnCode = (EManusRet)s_FunctionPointers->AddChainToSkeletonSetup(p_SkeletonSetupIndex, p_Chain);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::AddMeshSetupToSkeletonSetup(uint32_t p_SkeletonSetupIndex, uint32_t p_NodeID, uint32_t* p_MeshSetupIndex)
{
    RETURN_IF_NOT_INITIALISED(TEXT("AddMeshSetupToSkeletonSetup"), EManusRet::Error);
    
    EManusRet t_ReturnCode = EManusRet::Success;
    {
        SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_AddMeshSetupToSkeletonSetup);
        t_ReturnCode = (EManusRet)s_FunctionPointers->AddMeshSetupToSkeletonSetup(p_SkeletonSetupIndex, p_NodeID, p_MeshSetupIndex);
    }
    return t_ReturnCode;
}

EManusRet CoreSdk::AddVertexToMeshSetup(uint32_t p_SkeletonSetupIndex, uint32_t p_MeshSetupIndex, Vertex p_Vertex)
{
    RETURN_IF_NOT_INITIALISED(TEXT("AddVertexToMeshSetup"), EManusRet::Error);
    EManusRet t_ReturnCode = EManusRet::Success;
    {
        SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_AddVertexToMeshSetup);
        t_ReturnCode = (EManusRet)s_FunctionPointers->AddVertexToMeshSetup(p_SkeletonSetupIndex, p_MeshSetupIndex, p_Vertex);
    }
    return t_ReturnCode;
}

EManusRet CoreSdk::AddTriangleToMeshSetup(uint32_t p_SkeletonSetupIndex, uint32_t p_MeshSetupIndex, Triangle p_Triangle)
{
    RETURN_IF_NOT_INITIALISED(TEXT("AddTriangleToMeshSetup"), EManusRet::Error);
    
    EManusRet t_ReturnCode = EManusRet::Success;
    {
        SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_AddTriangleToMeshSetup);
        t_ReturnCode = (EManusRet)s_FunctionPointers->AddTriangleToMeshSetup(p_SkeletonSetupIndex, p_MeshSetupIndex, p_Triangle);
    }
    return t_ReturnCode;
}



EManusRet CoreSdk::OverwriteSkeletonSetup(uint32_t p_SkeletonIndex, const SkeletonSetupInfo& p_Skeleton)
{
	RETURN_IF_NOT_INITIALISED(TEXT("OverwriteSkeletonSetup"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_OverwriteSkeletonSetup);
		t_ReturnCode = (EManusRet)s_FunctionPointers->OverwriteSkeletonSetup(p_SkeletonIndex, p_Skeleton);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::OverwriteNodeToSkeletonSetup(uint32_t p_SkeletonIndex, const NodeSetup& p_Node)
{
	RETURN_IF_NOT_INITIALISED(TEXT("OverwriteNodeToSkeletonSetup"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_OverwriteNodeToSkeletonSetup);
		t_ReturnCode = (EManusRet)s_FunctionPointers->OverwriteNodeToSkeletonSetup(p_SkeletonIndex, p_Node);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::OverwriteChainToSkeletonSetup(uint32_t p_SkeletonIndex, const ChainSetup& p_Chain) 
{
	RETURN_IF_NOT_INITIALISED(TEXT("OverwriteChainToSkeletonSetup"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_OverwriteChainToSkeletonSetup);
		t_ReturnCode = (EManusRet)s_FunctionPointers->OverwriteChainToSkeletonSetup(p_SkeletonIndex, p_Chain);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::GetSkeletonSetupArraySizes(uint32_t p_SkeletonSetupIndex, SkeletonSetupArraySizes& p_SkeletonInfo)
{
	RETURN_IF_NOT_INITIALISED(TEXT("GetSkeletonSetupArraySizes"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetSkeletonSetupArraySizes);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetSkeletonSetupArraySizes(p_SkeletonSetupIndex, &p_SkeletonInfo);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::AllocateChainsForSkeletonSetup(uint32_t p_SkeletonIndex)
{
	RETURN_IF_NOT_INITIALISED(TEXT("AllocateChainsForSkeletonSetup"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_AllocateChainsForSkeletonSetup);
		t_ReturnCode = (EManusRet)s_FunctionPointers->AllocateChainsForSkeletonSetup(p_SkeletonIndex);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::GetSkeletonSetupInfo(uint32_t p_SkeletonSetupIndex, SkeletonSetupInfo* p_SDK)
{
    RETURN_IF_NOT_INITIALISED(TEXT("GetSkeletonSetupInfo"), EManusRet::Error);

    EManusRet t_ReturnCode = EManusRet::Success;
    {
        SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetSkeletonSetupInfo);
        t_ReturnCode = (EManusRet)s_FunctionPointers->GetSkeletonSetupInfo(p_SkeletonSetupIndex, p_SDK);
    }
    return t_ReturnCode;
}

EManusRet CoreSdk::GetSkeletonSetupChains(uint32_t p_SkeletonIndex, ChainSetup* p_SDK)
{
	RETURN_IF_NOT_INITIALISED(TEXT("GetSkeletonSetupChains"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetSkeletonSetupChains);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetSkeletonSetupChains(p_SkeletonIndex, p_SDK);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::GetSkeletonSetupNodes(uint32_t p_SkeletonIndex, NodeSetup* p_SDK)
{
	RETURN_IF_NOT_INITIALISED(TEXT("GetSkeletonSetupNodes"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetSkeletonSetupNodes);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetSkeletonSetupNodes(p_SkeletonIndex, p_SDK);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::LoadSkeleton(uint32_t p_SkeletonIndex, uint32_t& p_SkeletonId)
{
	RETURN_IF_NOT_INITIALISED(TEXT("LoadSkeleton"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_LoadSkeleton);
		t_ReturnCode = (EManusRet)s_FunctionPointers->LoadSkeleton(p_SkeletonIndex, &p_SkeletonId);
	}
	return t_ReturnCode;
}


EManusRet CoreSdk::CompressTemporarySkeletonAndGetSize(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId, uint32_t& p_TemporarySkeletonLengthInBytes)
{
	RETURN_IF_NOT_INITIALISED(TEXT("CompressTemporarySkeletonAndGetSize"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_CompressTemporarySkeletonAndGetSize);
		t_ReturnCode = (EManusRet)s_FunctionPointers->CompressTemporarySkeletonAndGetSize(p_SkeletonSetupIndex, p_SessionId, &p_TemporarySkeletonLengthInBytes);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::GetCompressedTemporarySkeletonData(unsigned char* p_TemporarySkeletonData, uint32_t p_TemporarySkeletonLengthInBytes)
{
	RETURN_IF_NOT_INITIALISED(TEXT("GetCompressedTemporarySkeletonData"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetCompressedTemporarySkeletonData);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetCompressedTemporarySkeletonData(p_TemporarySkeletonData, p_TemporarySkeletonLengthInBytes);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::GetTemporarySkeletonFromCompressedData(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId, unsigned char* p_TemporarySkeletonData, uint32_t p_TemporarySkeletonLengthInBytes)
{
	RETURN_IF_NOT_INITIALISED(TEXT("GetTemporarySkeletonFromCompressedData"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetTemporarySkeletonFromCompressedData);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetTemporarySkeletonFromCompressedData(p_SkeletonSetupIndex, p_SessionId, p_TemporarySkeletonData, p_TemporarySkeletonLengthInBytes);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::SaveTemporarySkeleton(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId, bool p_IsSkeletonModified)
{
	RETURN_IF_NOT_INITIALISED(TEXT("SaveTemporarySkeleton"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_SaveTemporarySkeleton);
		t_ReturnCode = (EManusRet)s_FunctionPointers->SaveTemporarySkeleton(p_SkeletonSetupIndex, p_SessionId, p_IsSkeletonModified);
	}
	return t_ReturnCode;
}


EManusRet CoreSdk::GetTemporarySkeleton(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId)
{
	RETURN_IF_NOT_INITIALISED(TEXT("GetTemporarySkeleton"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetTemporarySkeleton);
		t_ReturnCode = (EManusRet)s_FunctionPointers->GetTemporarySkeleton(p_SkeletonSetupIndex, p_SessionId);
	}
	return t_ReturnCode;
}

EManusRet CoreSdk::UnloadSkeleton(uint32_t p_SkeletonId)
{
	RETURN_IF_NOT_INITIALISED(TEXT("UnloadSkeleton"), EManusRet::Error);

	EManusRet t_ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_UnloadSkeleton);
		t_ReturnCode = (EManusRet)s_FunctionPointers->UnloadSkeleton( p_SkeletonId);
	}
	return t_ReturnCode;
}

void CoreSdk::OnErgonomicsCallback(const ErgonomicsStream* const p_Ergonomics)
{	
	s_CriticalSectionErgonomics->Lock();

	if (s_ErgonomicsData != nullptr) delete s_ErgonomicsData;
	// copy data
	s_ErgonomicsData = new ErgonomicsStream();
	s_ErgonomicsData->dataCount = p_Ergonomics->dataCount;
	for (uint32_t i = 0; i < p_Ergonomics->dataCount; i++)
	{
		s_ErgonomicsData->data[i].id = p_Ergonomics->data[i].id;
		s_ErgonomicsData->data[i].isUserID = p_Ergonomics->data[i].isUserID;
		for (uint32_t j = 0;  j < ErgonomicsDataType::ErgonomicsDataType_MAX_SIZE;  j++)
		{
			s_ErgonomicsData->data[i].data[j] = p_Ergonomics->data[i].data[j];
		}
	}
	s_CriticalSectionErgonomics->Unlock();
}

EManusRet CoreSdk::GetErgonomicsDataForGlove(FManusErgonomicsData& p_Data, uint32_t p_GloveId)
{
	s_CriticalSectionErgonomics->Lock();

	if (s_ErgonomicsData == nullptr)
	{
		s_CriticalSectionErgonomics->Unlock();
		return EManusRet::FunctionCalledAtWrongTime;
	}

	for (uint32_t i = 0; i < s_ErgonomicsData->dataCount; i++)
	{
		if (s_ErgonomicsData->data[i].id == p_GloveId)
		{
			p_Data.Id = s_ErgonomicsData->data[i].id;
			p_Data.IsUserID = s_ErgonomicsData->data[i].isUserID;
			for (uint32_t j = 0; j < ErgonomicsDataType::ErgonomicsDataType_MAX_SIZE; j++)
			{
				p_Data.Data.Add( s_ErgonomicsData->data[i].data[j]);
			}
			break;
		}
	}
	s_CriticalSectionErgonomics->Unlock();

	return EManusRet::Success;
}

void CoreSdk::OnGestureCallback(const GestureStreamInfo* const p_GestureStream)
{
    s_CriticalSectionGesture->Lock();
        
    if (s_GestureData.size() > 0) s_GestureData.clear();

    for (uint32_t i = 0; i < p_GestureStream->gestureProbabilitiesCount; i++)
    {
        GestureProbabilities t_Probs;
        if (GetGestureStreamData(i, 0, &t_Probs) != EManusRet::Success) continue;
        if (t_Probs.isUserID) continue; // we dont care about users.....for now

        ClientGestures t_Gest;
        t_Gest.info = t_Probs; // info.id is a glove id! so we need to match that correctly.
        t_Gest.probabilities.reserve(t_Gest.info.totalGestureCount);
        uint32_t t_BatchCount = (t_Gest.info.totalGestureCount / MAX_GESTURE_DATA_CHUNK_SIZE) + 1;
        uint32_t t_ProbabilityIdx = 0;
        for (uint32_t b = 0; b < t_BatchCount; b++)
        {
            for (uint32_t j = 0; j < t_Probs.gestureCount; j++)
            {
                t_Gest.probabilities.push_back(t_Probs.gestureData[j]);
            }
            t_ProbabilityIdx += t_Probs.gestureCount;
            GetGestureStreamData(i, t_ProbabilityIdx, &t_Probs); //this will get more data, if needed for the next iteration.
        }
        s_GestureData.push_back(t_Gest); // verify
    }
    s_CriticalSectionGesture->Unlock();
}

EManusRet CoreSdk::GetGestureStreamData(uint32_t p_GestureStreamDataIndex, uint32_t p_StartDataIndex, GestureProbabilities* p_GestureProbabilitiesCollection)
{
    RETURN_IF_NOT_INITIALISED(TEXT("GetGestureStreamData"), EManusRet::Error);

    EManusRet t_ReturnCode = EManusRet::Success;
    {
        SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_SaveTemporarySkeleton);
        t_ReturnCode = (EManusRet)s_FunctionPointers->GetGestureStreamData(p_GestureStreamDataIndex, p_StartDataIndex, p_GestureProbabilitiesCollection);
    }
    return t_ReturnCode;
}

void CoreSdk::GetGestureProbabilities(std::vector<ClientGestures>& p_ClientGestures)
{
    s_CriticalSectionGesture->Lock();
    p_ClientGestures = s_GestureData; // copies it all.
    s_CriticalSectionGesture->Unlock();
}

bool CoreSdk::IsGesturePastTreshold(int64 p_GestureId, float p_Treshold, int64 p_GloveId)
{
    s_CriticalSectionGesture->Lock();
    bool t_Found = false;
    bool t_TresholdReached = false;
    for (uint32_t i = 0; i < s_GestureData.size(); i++)
    {
        if (s_GestureData[i].info.id == p_GloveId) 
        {
            for (uint32_t j = 0; j < s_GestureData[i].probabilities.size(); j++)
            {
                if (p_GestureId == s_GestureData[i].probabilities[j].id)
                {
                    t_Found = true;
                    if (s_GestureData[i].probabilities[j].percent >= p_Treshold) t_TresholdReached = true;
                    break;
                }
            }            
        }
        if (t_Found) break;
    }

    s_CriticalSectionGesture->Unlock();
    return t_TresholdReached;
}

// we only need this to check the number of gestures.
// then we can call the CoreSdk_GetGestureLandscapeData if the number ever changes.
void CoreSdk::OnLandscapeCallback(const Landscape* const p_Landscape)
{
    s_CriticalSectionLandscape->Lock();    
    if ((p_Landscape->gestureCount > 0) &&
        (p_Landscape->gestureCount != s_GestureLandscapeData.size()))
    {
        std::vector<GestureLandscapeData> t_GestureLandscapeData;
        // alright get new gestures.
        t_GestureLandscapeData.clear();
        t_GestureLandscapeData.resize(p_Landscape->gestureCount);
        GetGestureLandscapeData(t_GestureLandscapeData.data(), p_Landscape->gestureCount);
        s_GestureLandscapeData = t_GestureLandscapeData;
        int blargh = s_GestureLandscapeData.size();
        //uint32_t t_ttt = s_GestureLandscapeData[0].id;
        blargh = 0;
    }
    
    s_LandscapeData = *p_Landscape;

    s_CriticalSectionLandscape->Unlock();
}

EManusRet CoreSdk::GetGestureLandscapeData(GestureLandscapeData* p_LandscapeDataArray, uint32_t p_ArraySize)
{
    RETURN_IF_NOT_INITIALISED(TEXT("GetGestureLandscapeData"), EManusRet::Error);

    EManusRet t_ReturnCode = EManusRet::Success;
    {
        SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetGestureLandscapeData);
        t_ReturnCode = (EManusRet)s_FunctionPointers->GetGestureLandscapeData(p_LandscapeDataArray, p_ArraySize);
    }
    return t_ReturnCode;
}

void CoreSdk::GetGestures(std::vector<GestureLandscapeData>& p_ClientGestures)
{
    s_CriticalSectionLandscape->Lock();
    p_ClientGestures = s_GestureLandscapeData; // copies it all.
    s_CriticalSectionLandscape->Unlock();
}

uint32_t CoreSdk::GetGloveIdForSkeleton(uint32_t p_SkeletonId, bool p_Left)
{
    uint32_t t_GloveId = 0;
    s_CriticalSectionLandscape->Lock();
    for (uint32_t i = 0; i < s_LandscapeData.skeletons.skeletonCount; i++)
    {
        if (s_LandscapeData.skeletons.skeletons[i].id == p_SkeletonId) 
        {
            uint32_t t_UserId = s_LandscapeData.skeletons.skeletons[i].userId;
            for (uint32_t j = 0; j < s_LandscapeData.users.userCount; j++)
            {
                if (s_LandscapeData.users.users[j].id == t_UserId)
                {
                    if (p_Left) t_GloveId = s_LandscapeData.users.users[j].leftGloveID;
                    else t_GloveId = s_LandscapeData.users.users[j].rightGloveID;
                    break;
                }
            }
        }
        if (t_GloveId != 0) break;
    }

    s_CriticalSectionLandscape->Unlock();
    return t_GloveId;
}

bool CoreSdk::DoesSkeletonHaveHaptics(int64 p_SkeletonId, bool p_Left)
{
    uint32_t t_GloveId = GetGloveIdForSkeleton(p_SkeletonId, p_Left);
    if (t_GloveId == 0) return false; // weird but its probably borked so dont even try haptics

    // ok we got a glove id
    bool t_HasHaptics = false;
    s_CriticalSectionLandscape->Lock();
    for (uint32_t  i = 0; i < s_LandscapeData.gloveDevices.gloveCount; i++)
    {
        if (s_LandscapeData.gloveDevices.gloves[i].id == t_GloveId)
        {
            t_HasHaptics = s_LandscapeData.gloveDevices.gloves[i].isHaptics;
            break;
        }
    }
    s_CriticalSectionLandscape->Unlock();
    return t_HasHaptics;
}
/* TBD if we want extra data here
void CoreSdk::OnExtraDataCallback(const ExtraDataStream& p_ExtraData)
{
	s_CriticalSectionExtraData->Lock();
	
	if (s_ExtraDataData != nullptr) delete s_ExtraDataData;
	// copy data
	s_ExtraDataData = new ExtraDataStream();
	s_ExtraDataData->dataCount = p_ExtraData.dataCount;
	for (uint32_t i = 0; i < p_ExtraData.dataCount; i++)
	{
		s_ExtraDataData->data[i].id = p_ExtraData.data[i].id;
		s_ExtraDataData->data[i].isUserID = p_ExtraData.data[i].isUserID;
		for (uint32_t j = 0; j < ExtraDataDataType::ExtraDataDataType_MaxSize; j++)
		{
			s_ExtraDataData->data[i].data[j] = p_ExtraData.data[i].data[j];
		}
	}
	
	s_CriticalSectionExtraData->Unlock();
}
*/

void CoreSdk::OnSystemCallback(const SystemMessage* const p_SystemMessage)
{
	s_CriticalSectionSystem->Lock();
	
	switch (p_SystemMessage->type)
	{
		case SystemMessageType::SystemMessageType_TemporarySkeletonModified:
		{
			// if the message was triggered by a temporary skeleton being modified save the skeleton index,
			// this information will be used to load the skeleton
			s_LastModifiedSkeletonIndex = p_SystemMessage->infoUInt;
			// go through all the manus skeletons and update it.
			break;
		}
		default:
		{
			// todo if we want system messages responses.
			//s_Instance->m_SystemMessageCode = p_SystemMessage->type;
			//s_Instance->m_SystemMessage = p_SystemMessage->infoString;
			break;
		}
	}
	
	s_CriticalSectionSystem->Unlock();
}

void CoreSdk::GetLastModifiedSkeletonIndex(uint32_t &p_Index)
{
	p_Index = s_LastModifiedSkeletonIndex;
	s_LastModifiedSkeletonIndex = UINT32_MAX;
}
