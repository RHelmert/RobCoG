// Copyright 2015-2020 Manus

#include "CoreSdk.h"
#include "Manus.h"
#include "ManusSkeleton.h"

#include "Misc/FileHelper.h"

#include "ManusConvert.h"
#include "CoreSdkWrapper.h"

#include "Interfaces/IPluginManager.h"
#include "AnimationRuntime.h"
#include "Misc/MessageDialog.h"
#include "Kismet/KismetMathLibrary.h"

#include <limits>

#ifdef _WIN64
	#define PLATFORM_STRING "Win64"
#else
	#error Unknown platform.
#endif

#define RETURN_IF_NOT_INITIALISED(FunctionName, ReturnValue) \
	if (!FunctionPointers)		\
	{							\
		UE_LOG(LogManus, Error, TEXT("Tried to use the Core SDK function \"%s\", but the SDK was not initialised."), FunctionName); \
		return ReturnValue;		\
	}							\


DECLARE_CYCLE_STAT(TEXT("CoreSDK Initialize"),								STAT_CoreSDK_Initialize, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK ConnectLocally"),							STAT_CoreSDK_ConnectLocally, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK ShutDown"),								STAT_CoreSDK_ShutDown, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK WasConnectionLostDueToTimeout"),			STAT_CoreSDK_WasConnectionLostDueToTimeout, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetVersionsAndCheckCompatibility"),		STAT_CoreSDK_GetVersionsAndCheckCompatibility, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK VibrateWristOfGlove"),						STAT_CoreSDK_VibrateWristOfGlove, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK VibrateFingers"),							STAT_CoreSDK_VibrateFingers, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetGloveIdOfUser_UsingUserIndex"),			STAT_CoreSDK_GetGloveIdOfUser_UsingUserIndex, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetNumberOfAvailableGloves"),				STAT_CoreSDK_GetNumberOfAvailableGloves, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetIdsOfAvailableGloves"),					STAT_CoreSDK_GetIdsOfAvailableGloves, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetHandTypeOfGlove"),						STAT_CoreSDK_GetHandTypeOfGlove, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetIdOfFirstAvailableGloveOfType"),		STAT_CoreSDK_GetIdOfFirstAvailableGloveOfType, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetDataForGlove_UsingGloveId"),			STAT_CoreSDK_GetDataForGlove_UsingGloveId, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetDataForGlove_UsingHandType"),			STAT_CoreSDK_GetDataForGlove_UsingHandType, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetNumberOfHapticsDongles"),				STAT_CoreSDK_GetNumberOfHapticsDongles, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetHapticsDongleIds"),						STAT_CoreSDK_GetHapticsDongleIds, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK AddOrUpdatePolygonSkeleton"),				STAT_CoreSDK_AddOrUpdatePolygonSkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK RemovePolygonSkeleton"),					STAT_CoreSDK_RemovePolygonSkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK SetPolygonSkeletonTarget"),				STAT_CoreSDK_SetPolygonSkeletonTarget, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK SetRetargetingSettings"),					STAT_CoreSDK_SetRetargetingSettings, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK IsPolygonSkeletonIdValid"),				STAT_CoreSDK_IsPolygonSkeletonIdValid, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetNumberOfAvailablePolygonSkeletons"),	STAT_CoreSDK_GetNumberOfAvailablePolygonSkeletons, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetIdsOfAvailablePolygonSkeletons"),		STAT_CoreSDK_GetIdsOfAvailablePolygonSkeletons, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetIdOfFirstAvailablePolygonSkeleton"),	STAT_CoreSDK_GetIdOfFirstAvailablePolygonSkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetDataForPolygonSkeleton"),				STAT_CoreSDK_GetDataForPolygonSkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK GetDataForTracker_UsingIndexAndType"),		STAT_CoreSDK_GetDataForTracker_UsingIndexAndType, STATGROUP_Manus);

DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL Initialise"),								STAT_CoreSDK_DLL_Initialise, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL ConnectLocally"),							STAT_CoreSDK_DLL_ConnectLocally, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL ShutDown"),								STAT_CoreSDK_DLL_ShutDown, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL WasConnectionLostDueToTimeout"),			STAT_CoreSDK_DLL_WasConnectionLostDueToTimeout, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetVersionsAndCheckCompatibility"),		STAT_CoreSDK_DLL_GetVersionsAndCheckCompatibility, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL VibrateWristOfGlove"),						STAT_CoreSDK_DLL_VibrateWristOfGlove, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL VibrateFingers"),							STAT_CoreSDK_DLL_VibrateFingers, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetGloveIdOfUser_UsingUserIndex"),			STAT_CoreSDK_DLL_GetGloveIdOfUser_UsingUserIndex, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetNumberOfAvailableGloves"),				STAT_CoreSDK_DLL_GetNumberOfAvailableGloves, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetIdsOfAvailableGloves"),					STAT_CoreSDK_DLL_GetIdsOfAvailableGloves, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetHandTypeOfGlove"),						STAT_CoreSDK_DLL_GetHandTypeOfGlove, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetIdOfFirstAvailableGloveOfType"),		STAT_CoreSDK_DLL_GetIdOfFirstAvailableGloveOfType, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetDataForGlove_UsingGloveId"),			STAT_CoreSDK_DLL_GetDataForGlove_UsingGloveId, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetDataForGlove_UsingHandType"),			STAT_CoreSDK_DLL_GetDataForGlove_UsingHandType, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetNumberOfHapticsDongles"),				STAT_CoreSDK_DLL_GetNumberOfHapticsDongles, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetHapticsDongleIds"),						STAT_CoreSDK_DLL_GetHapticsDongleIds, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL AddOrUpdatePolygonSkeleton"),				STAT_CoreSDK_DLL_AddOrUpdatePolygonSkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL RemovePolygonSkeleton"),					STAT_CoreSDK_DLL_RemovePolygonSkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL SetPolygonSkeletonTarget"),				STAT_CoreSDK_DLL_SetPolygonSkeletonTarget, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL SetRetargetingSettings"),					STAT_CoreSDK_DLL_SetRetargetingSettings, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL IsPolygonSkeletonIdValid"),				STAT_CoreSDK_DLL_IsPolygonSkeletonIdValid, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetNumberOfAvailablePolygonSkeletons"),	STAT_CoreSDK_DLL_GetNumberOfAvailablePolygonSkeletons, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetIdsOfAvailablePolygonSkeletons"),		STAT_CoreSDK_DLL_GetIdsOfAvailablePolygonSkeletons, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetIdOfFirstAvailablePolygonSkeleton"),	STAT_CoreSDK_DLL_GetIdOfFirstAvailablePolygonSkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetDataForPolygonSkeleton"),				STAT_CoreSDK_DLL_GetDataForPolygonSkeleton, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL GetDataForTracker_UsingIndexAndType"),		STAT_CoreSDK_DLL_GetDataForTracker_UsingIndexAndType, STATGROUP_Manus);
//Callbacks
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL RegisterCallbackForOnConnect"),			STAT_CoreSDK_DLL_RegisterCallbackForOnConnect, STATGROUP_Manus);
DECLARE_CYCLE_STAT(TEXT("CoreSDK DLL RegisterCallbackForOnDisconnect"),			STAT_CoreSDK_DLL_RegisterCallbackForOnDisconnect, STATGROUP_Manus);


// Forward-declarations for local functions.
static bool ConvertSdkGloveDataToBp(const GloveData &SdkInput, FManusGlove &BpOutput);
static bool ConvertSdkPolygonSkeletonDataToBp(const PolygonSkeletonData &SdkInput, FManusPolygonSkeleton &BpOutput);
static int ConvertSdkPolygonSkeletonBoneTypeToBp(int BoneType);
static bool ConvertSdkTrackerDataToBp(const TrackerData & SdkInput, FManusTracker & BpOutput);
static void OnConnectedToCore(void);

// Static member variables
CoreSdk::CoreFunctionPointers* CoreSdk::FunctionPointers = nullptr;
void* CoreSdk::DllHandle = nullptr;

/**
 * Stores all the function pointers to functions in the Core SDK wrapper.
 * Doing this here makes it possible to keep SDK wrapper types and functions contained to this .cpp file.
 */
struct CoreSdk::CoreFunctionPointers
{
	/*      return value				type name													function arguments */
	typedef WrapperReturnCode(*WasDllBuiltInDebugConfiguration_FunctionPointer)			(bool& a_WasDllBuiltInDebugConfiguration);
	typedef WrapperReturnCode(*Initialise_FunctionPointer)								(ClientType a_TypeOfClient);
	typedef WrapperReturnCode(*ConnectLocally_FunctionPointer)							(void);
	typedef WrapperReturnCode(*ShutDown_FunctionPointer)								(void);
	typedef WrapperReturnCode(*WasConnectionLostDueToTimeout_FunctionPointer)			(bool& a_ConnectionWasLost);
	typedef WrapperReturnCode(*GetVersionsAndCheckCompatibility_FunctionPointer)		(ManusVersion& a_WrapperVersion, ManusVersion& a_SdkVersion, ManusVersion& a_CoreVersion, bool& a_Compatible);
	typedef WrapperReturnCode(*VibrateWristOfGlove_FunctionPointer)						(uint32_t a_GloveId, float a_UnitStrength, uint16_t a_DurationInMilliseconds);
	typedef WrapperReturnCode(*VibrateFingers_FunctionPointer)							(uint32_t a_DongleId, HandType a_HandType, float* a_Powers);
	typedef WrapperReturnCode(*GetGloveIdOfUser_UsingUserIndex_FunctionPointer)			(uint32_t a_UserIndex, HandType a_HandType, uint32_t& a_GloveId);
	typedef WrapperReturnCode(*GetNumberOfAvailableGloves_FunctionPointer)				(uint32_t& a_NumberOfAvailableGloves);
	typedef WrapperReturnCode(*GetIdsOfAvailableGloves_FunctionPointer)					(uint32_t* a_IdsOfAvailableGloves, uint32_t a_NumberOfIdsThatFitInArray);
	typedef WrapperReturnCode(*GetHandTypeOfGlove_FunctionPointer)						(uint32_t a_GloveId, HandType& a_HandType);
	typedef WrapperReturnCode(*GetIdOfFirstAvailableGloveOfType_FunctionPointer)		(HandType a_HandType, uint32_t& a_GloveId);
	typedef WrapperReturnCode(*GetDataForGlove_UsingGloveId_FunctionPointer)			(uint32_t a_GloveId, GloveData& a_GloveData);
	typedef WrapperReturnCode(*GetDataForGlove_UsingHandType_FunctionPointer)			(HandType a_HandType, GloveData& a_GloveData);
	typedef WrapperReturnCode(*GetNumberOfHapticsDongles_FunctionPointer)				(uint32_t& a_NumberOfHapticsDongles);
	typedef WrapperReturnCode(*GetHapticsDongleIds_FunctionPointer)						(uint32_t* a_HapticsDongleIds, uint32_t a_NumberOfIdsThatFitInArray);
	typedef WrapperReturnCode(*AddOrUpdatePolygonSkeleton_FunctionPointer)				(PolygonSkeletonData& a_PolygonSkeletonData);
	typedef WrapperReturnCode(*RemovePolygonSkeleton_FunctionPointer)					(RemoveSkeletonArgs& a_RemovePolygonArgs);
	typedef WrapperReturnCode(*SetPolygonSkeletonTarget_FunctionPointer)				(PolygonTargetArgs& a_SetSkeletonTargetArgs);
	typedef WrapperReturnCode(*SetPolygonRetargetingSettings_FunctionPointer)			(RetargetingSettingsArgs& a_RetargetSettingsArgs);
	typedef WrapperReturnCode(*IsPolygonSkeletonIdValid_FunctionPointer)				(uint32_t a_PolygonSkeletonId, bool& a_IsValid);
	typedef WrapperReturnCode(*GetNumberOfAvailablePolygonSkeletons_FunctionPointer)	(uint32_t& a_NumberOfAvailablePolygonSkeletons);
	typedef WrapperReturnCode(*GetIdsOfAvailablePolygonSkeletons_FunctionPointer)		(uint32_t* a_IdsOfAvailablePolygonSkeletons, uint32_t a_NumberOfIdsThatFitInArray);
	typedef WrapperReturnCode(*GetIdOfFirstAvailablePolygonSkeleton_FunctionPointer)	(uint32_t& a_PolygonSkeletonId);
	typedef WrapperReturnCode(*GetDataForPolygonSkeleton_FunctionPointer)				(uint32_t a_PolygonSkeletonId, PolygonSkeletonData& a_PolygonSkeletonData);
	typedef WrapperReturnCode(*GetDataForTracker_UsingIndexAndType_FunctionPointer)		(uint32_t a_UserIndex, uint32_t a_TrackerType, TrackerData& a_TrackerData);

	// Callbacks
	typedef WrapperReturnCode(*RegisterCallbackForOnConnect_FunctionPointer)			(ConnectedToCoreCallback_t a_OnConnectedCallback);
	typedef WrapperReturnCode(*RegisterCallbackForOnDisconnect_FunctionPointer)			(DisconnectedFromCoreCallback_t a_OnDisconnectedCallback);

	WasDllBuiltInDebugConfiguration_FunctionPointer				WasDllBuiltInDebugConfiguration = nullptr;
	Initialise_FunctionPointer									Initialize = nullptr;
	ConnectLocally_FunctionPointer								ConnectLocally = nullptr;
	ShutDown_FunctionPointer									ShutDown = nullptr;
	WasConnectionLostDueToTimeout_FunctionPointer				WasConnectionLostDueToTimeout = nullptr;
	GetVersionsAndCheckCompatibility_FunctionPointer			GetVersionsAndCheckCompatibility = nullptr;
	VibrateWristOfGlove_FunctionPointer							VibrateWristOfGlove = nullptr;
	VibrateFingers_FunctionPointer								VibrateFingers = nullptr;
	GetGloveIdOfUser_UsingUserIndex_FunctionPointer				GetGloveIdOfUser_UsingUserIndex = nullptr;
	GetNumberOfAvailableGloves_FunctionPointer					GetNumberOfAvailableGloves = nullptr;
	GetIdsOfAvailableGloves_FunctionPointer						GetIdsOfAvailableGloves = nullptr;
	GetHandTypeOfGlove_FunctionPointer							GetHandTypeOfGlove = nullptr;
	GetIdOfFirstAvailableGloveOfType_FunctionPointer			GetIdOfFirstAvailableGloveOfType = nullptr;
	GetDataForGlove_UsingGloveId_FunctionPointer				GetDataForGlove_UsingGloveId = nullptr;
	GetDataForGlove_UsingHandType_FunctionPointer				GetDataForGlove_UsingHandType = nullptr;
	GetNumberOfHapticsDongles_FunctionPointer					GetNumberOfHapticsDongles = nullptr;
	GetHapticsDongleIds_FunctionPointer							GetHapticsDongleIds = nullptr;
	AddOrUpdatePolygonSkeleton_FunctionPointer					AddOrUpdatePolygonSkeleton = nullptr;
	RemovePolygonSkeleton_FunctionPointer						RemovePolygonSkeleton = nullptr;
	SetPolygonSkeletonTarget_FunctionPointer					SetPolygonSkeletonTarget = nullptr;
	SetPolygonRetargetingSettings_FunctionPointer				SetPolygonRetargetingSettings = nullptr;
	IsPolygonSkeletonIdValid_FunctionPointer					IsPolygonSkeletonIdValid = nullptr;
	GetNumberOfAvailablePolygonSkeletons_FunctionPointer		GetNumberOfAvailablePolygonSkeletons = nullptr;
	GetIdsOfAvailablePolygonSkeletons_FunctionPointer			GetIdsOfAvailablePolygonSkeletons = nullptr;
	GetIdOfFirstAvailablePolygonSkeleton_FunctionPointer		GetIdOfFirstAvailablePolygonSkeleton = nullptr;
	GetDataForPolygonSkeleton_FunctionPointer					GetDataForPolygonSkeleton = nullptr;
	GetDataForTracker_UsingIndexAndType_FunctionPointer			GetDataForTracker_UsingIndexAndType = nullptr;

	// Callbacks
	RegisterCallbackForOnConnect_FunctionPointer				RegisterCallbackForOnConnect = nullptr;
	RegisterCallbackForOnDisconnect_FunctionPointer				RegisterCallbackForOnDisconnect = nullptr;
};

EManusRet CoreSdk::Initialize()
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_Initialize);

	if (FunctionPointers)
	{
		return EManusRet::Success;
	}

	EManusRet ReturnCode = EManusRet::Success;
	FString ResultOfError(TEXT("Manus glove support will not be available."));

	FunctionPointers = new CoreFunctionPointers();

	// Get a handle to the DLL 
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("Manus"))->GetBaseDir();
	FString FilePath = FPaths::Combine(*PluginDir, TEXT("ThirdParty/Manus/lib"), TEXT(PLATFORM_STRING), TEXT("CoreSdkWrapper.dll"));
	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogManus, Error, TEXT("Could not find the Core SDK DLL at %s. %s"), *FilePath, *ResultOfError);
		ReturnCode = EManusRet::SDKNotAvailable;
	}

	DllHandle = nullptr;
	if (ReturnCode == EManusRet::Success)
	{
		DllHandle = FPlatformProcess::GetDllHandle(*FilePath);
		if (!DllHandle)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to obtain a DLL handle for %s. %s"), *FilePath, *ResultOfError);
			ReturnCode = EManusRet::SDKNotAvailable;
		}
	}

	// Load the SDK functions.
	if (ReturnCode == EManusRet::Success)
	{
		FunctionPointers->WasDllBuiltInDebugConfiguration =
			(CoreFunctionPointers::WasDllBuiltInDebugConfiguration_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_WasDllBuiltInDebugConfiguration"));
		FunctionPointers->Initialize =
			(CoreFunctionPointers::Initialise_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_Initialise"));
		FunctionPointers->ShutDown =
			(CoreFunctionPointers::ShutDown_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_ShutDown"));
		FunctionPointers->ConnectLocally =
			(CoreFunctionPointers::ConnectLocally_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_ConnectLocally"));
		FunctionPointers->WasConnectionLostDueToTimeout =
			(CoreFunctionPointers::WasConnectionLostDueToTimeout_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_WasConnectionLostDueToTimeout"));
		FunctionPointers->GetVersionsAndCheckCompatibility =
			(CoreFunctionPointers::GetVersionsAndCheckCompatibility_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetVersionsAndCheckCompatibility"));
		FunctionPointers->VibrateWristOfGlove =
			(CoreFunctionPointers::VibrateWristOfGlove_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_VibrateWristOfGlove"));
		FunctionPointers->VibrateFingers =
			(CoreFunctionPointers::VibrateFingers_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_VibrateFingers"));
		FunctionPointers->GetGloveIdOfUser_UsingUserIndex =
			(CoreFunctionPointers::GetGloveIdOfUser_UsingUserIndex_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetGloveIdOfUser_UsingUserIndex"));
		FunctionPointers->GetNumberOfAvailableGloves =
			(CoreFunctionPointers::GetNumberOfAvailableGloves_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetNumberOfAvailableGloves"));
		FunctionPointers->GetIdsOfAvailableGloves =
			(CoreFunctionPointers::GetIdsOfAvailableGloves_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetIdsOfAvailableGloves"));
		FunctionPointers->GetHandTypeOfGlove =
			(CoreFunctionPointers::GetHandTypeOfGlove_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetHandTypeOfGlove"));
		FunctionPointers->GetIdOfFirstAvailableGloveOfType =
			(CoreFunctionPointers::GetIdOfFirstAvailableGloveOfType_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetIdOfFirstAvailableGloveOfType"));
		FunctionPointers->GetDataForGlove_UsingGloveId =
			(CoreFunctionPointers::GetDataForGlove_UsingGloveId_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetDataForGlove_UsingGloveId"));
		FunctionPointers->GetDataForGlove_UsingHandType =
			(CoreFunctionPointers::GetDataForGlove_UsingHandType_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetDataForGlove_UsingHandType"));
		FunctionPointers->GetNumberOfHapticsDongles =
			(CoreFunctionPointers::GetNumberOfHapticsDongles_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetNumberOfHapticsDongles"));
		FunctionPointers->GetHapticsDongleIds =
			(CoreFunctionPointers::GetHapticsDongleIds_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetHapticsDongleIds"));
		FunctionPointers->AddOrUpdatePolygonSkeleton =
			(CoreFunctionPointers::AddOrUpdatePolygonSkeleton_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_AddOrUpdatePolygonSkeleton"));
		FunctionPointers->RemovePolygonSkeleton =
			(CoreFunctionPointers::RemovePolygonSkeleton_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_RemovePolygonSkeleton"));
		FunctionPointers->SetPolygonSkeletonTarget =
			(CoreFunctionPointers::SetPolygonSkeletonTarget_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_SetPolygonSkeletonTarget"));
		FunctionPointers->SetPolygonRetargetingSettings =
			(CoreFunctionPointers::SetPolygonRetargetingSettings_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_SetPolygonSkeletonRetargetingSettings"));
		FunctionPointers->IsPolygonSkeletonIdValid =
			(CoreFunctionPointers::IsPolygonSkeletonIdValid_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_IsPolygonSkeletonIdValid"));
		FunctionPointers->GetNumberOfAvailablePolygonSkeletons =
			(CoreFunctionPointers::GetNumberOfAvailablePolygonSkeletons_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetNumberOfAvailablePolygonSkeletons"));
		FunctionPointers->GetIdsOfAvailablePolygonSkeletons =
			(CoreFunctionPointers::GetIdsOfAvailablePolygonSkeletons_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetIdsOfAvailablePolygonSkeletons"));
		FunctionPointers->GetIdOfFirstAvailablePolygonSkeleton =
			(CoreFunctionPointers::GetIdOfFirstAvailablePolygonSkeleton_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetIdOfFirstAvailablePolygonSkeleton"));
		FunctionPointers->GetDataForPolygonSkeleton =
			(CoreFunctionPointers::GetDataForPolygonSkeleton_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetDataForPolygonSkeleton"));
		FunctionPointers->GetDataForTracker_UsingIndexAndType =
			(CoreFunctionPointers::GetDataForTracker_UsingIndexAndType_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_GetDataForTracker_UsingIndexAndType"));
		// Callbacks
		FunctionPointers->RegisterCallbackForOnConnect =
			(CoreFunctionPointers::RegisterCallbackForOnConnect_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_RegisterCallbackForOnConnect"));
		FunctionPointers->RegisterCallbackForOnDisconnect =
			(CoreFunctionPointers::RegisterCallbackForOnDisconnect_FunctionPointer)
			FPlatformProcess::GetDllExport(DllHandle, TEXT("CoreSdk_RegisterCallbackForOnDisconnect"));

		if (   !FunctionPointers->WasDllBuiltInDebugConfiguration  
			|| !FunctionPointers->Initialize
			|| !FunctionPointers->ShutDown
			|| !FunctionPointers->ConnectLocally
			|| !FunctionPointers->WasConnectionLostDueToTimeout
			|| !FunctionPointers->GetVersionsAndCheckCompatibility
			|| !FunctionPointers->VibrateWristOfGlove
			|| !FunctionPointers->VibrateFingers
			|| !FunctionPointers->GetGloveIdOfUser_UsingUserIndex
			|| !FunctionPointers->GetNumberOfAvailableGloves
			|| !FunctionPointers->GetIdsOfAvailableGloves
			|| !FunctionPointers->GetHandTypeOfGlove
			|| !FunctionPointers->GetIdOfFirstAvailableGloveOfType
			|| !FunctionPointers->GetDataForGlove_UsingGloveId
			|| !FunctionPointers->GetDataForGlove_UsingHandType
			|| !FunctionPointers->GetNumberOfHapticsDongles
			|| !FunctionPointers->GetHapticsDongleIds
			|| !FunctionPointers->AddOrUpdatePolygonSkeleton
			|| !FunctionPointers->RemovePolygonSkeleton
			|| !FunctionPointers->SetPolygonSkeletonTarget
			|| !FunctionPointers->SetPolygonRetargetingSettings
			|| !FunctionPointers->IsPolygonSkeletonIdValid
			|| !FunctionPointers->GetNumberOfAvailablePolygonSkeletons
			|| !FunctionPointers->GetIdsOfAvailablePolygonSkeletons
			|| !FunctionPointers->GetIdOfFirstAvailablePolygonSkeleton
			|| !FunctionPointers->GetDataForPolygonSkeleton
			|| !FunctionPointers->GetDataForTracker_UsingIndexAndType
			|| !FunctionPointers->RegisterCallbackForOnConnect
			|| !FunctionPointers->RegisterCallbackForOnDisconnect)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to get DLL exports for %s. %s"), *FilePath, *ResultOfError);
			ReturnCode = EManusRet::SDKNotAvailable;
		}
	}
	
	if (ReturnCode == EManusRet::Success)
	{
		{
			SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_Initialise);
			ReturnCode = (EManusRet)FunctionPointers->Initialize(ClientType::eCLIENTTYPE_UNREAL);

#if !UE_BUILD_SHIPPING
			bool WasDllBuiltInDebug = false;
			FunctionPointers->WasDllBuiltInDebugConfiguration(WasDllBuiltInDebug);

			if (WasDllBuiltInDebug)
			{
				UE_LOG(LogManus, Warning, TEXT("The DLL was built and included in with a debug configuration, please rebuild the DLL in release"));
			}
#endif
			{
				SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_RegisterCallbackForOnConnect);
				EManusRet RegisterReturn = (EManusRet)FunctionPointers->RegisterCallbackForOnConnect(*OnConnectedToCore);
			}

		}

		if (ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to initialise the Manus DLL. %s"), *ResultOfError);
			ReturnCode = EManusRet::SDKNotAvailable;
		}
		else
		{
			UE_LOG(LogManus, Log, TEXT("Successfully initialised the Manus DLL."));
		}
	}

	// Clean up.
	if (ReturnCode != EManusRet::Success)
	{
		if (DllHandle)
		{
			FPlatformProcess::FreeDllHandle(DllHandle);
		}

		delete FunctionPointers;
		FunctionPointers = nullptr;
	}

	return ReturnCode;
}

EManusRet CoreSdk::ConnectLocally() 
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_ConnectLocally);
	RETURN_IF_NOT_INITIALISED(TEXT("ConnectLocally"), EManusRet::Error);

	EManusRet ReturnCode = EManusRet::Success;
	if (ReturnCode == EManusRet::Success)
	{
		{
			SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_Initialise);
			ReturnCode = (EManusRet)FunctionPointers->ConnectLocally();
		}

		if (ReturnCode != EManusRet::Success)
		{
			UE_LOG(LogManus, Error, TEXT("Failed to connect to Manus-Core locally. Manus glove support will not be available."));
		}
		else
		{
			UE_LOG(LogManus, Log, TEXT("Successfully connected to Manus-Core locally."));
		}
	}

	return ReturnCode;
}

EManusRet CoreSdk::ShutDown()
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_ShutDown);

	EManusRet ReturnCode = EManusRet::Success;
	if (!FunctionPointers)
	{
		return ReturnCode;
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_ShutDown);
		ReturnCode = (EManusRet)FunctionPointers->ShutDown();
	}

	if (ReturnCode != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to shut down the Manus DLL."));
	}

	delete FunctionPointers;
	FunctionPointers = nullptr;

	FPlatformProcess::FreeDllHandle(DllHandle);
	DllHandle = nullptr;

	return ReturnCode;
}

EManusRet CoreSdk::CheckConnection()
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_WasConnectionLostDueToTimeout);
	RETURN_IF_NOT_INITIALISED(TEXT("WasConnectionLostDueToTimeout"), EManusRet::Error);

	EManusRet ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_WasConnectionLostDueToTimeout);
		bool ConnectionWasLost = false;
		ReturnCode = (EManusRet)FunctionPointers->WasConnectionLostDueToTimeout(ConnectionWasLost);
	}
	return ReturnCode;
}

EManusRet CoreSdk::CheckCompatibility()
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetVersionsAndCheckCompatibility);
	RETURN_IF_NOT_INITIALISED(TEXT("GetVersionsAndCheckCompatibility"), EManusRet::Error);

	EManusRet ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetVersionsAndCheckCompatibility);

		ManusVersion WrapperVersion;
		ManusVersion SdkVersion;
		ManusVersion CoreVersion;
		bool IsCompatible = false;

		ReturnCode = (EManusRet)FunctionPointers->GetVersionsAndCheckCompatibility(WrapperVersion, SdkVersion, CoreVersion, IsCompatible);

		if (ReturnCode == EManusRet::Success)
		{
			FString PluginVersion;
			FPluginDescriptor PluginData;
			if (FManusModule::GetPluginData(PluginData))
			{
				PluginVersion = "v" + PluginData.VersionName;
			}
			else 
			{
				PluginVersion = "Invalid";
			}

			FString Version = 
				" Plugin Version: " + PluginVersion +
				", Wrapper version: " + FString(WrapperVersion.m_String) + 
				", Sdk Version: " + FString(SdkVersion.m_String) + 
				", Core version: " + FString(CoreVersion.m_String) + ".";


			// Check plugin version compatibility with connected core
			if (IsCompatible) 
			{
				UE_LOG(LogManus, Log, TEXT("Versions are compatible. %s"), *Version);
			}
			else 
			{
				UE_LOG(LogManus, Warning, TEXT("Versions are not compatible. %s"), *Version);
				FText DialogTitle = FText::FromString("Versions uncompatible");
				FText DialogText = FText::FromString("The plugin version is not compatible with the currently connected manus core. Please make sure the versions match" + Version);
				FMessageDialog::Open(EAppMsgType::Ok, DialogText, &DialogTitle);
			}
		}
	}

	return ReturnCode;
}

EManusRet CoreSdk::VibrateWristOfGlove(int64 GloveId, float UnitStrength, int32 DurationInMilliseconds)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_VibrateWristOfGlove);
	RETURN_IF_NOT_INITIALISED(TEXT("VibrateWristOfGlove"), EManusRet::Error);

	if (GloveId == 0)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Tried to vibrate the wrist of a glove with ID %u, but this glove ID is not valid.")
			, static_cast<unsigned int>(GloveId));

		return EManusRet::InvalidArgument;
	}

	if (UnitStrength < 0.0f || UnitStrength > 1.0f)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Tried to vibrate the wrist of a glove with an invalid strength value of %f.")
			, UnitStrength);

		return EManusRet::InvalidArgument;
	}

	if (DurationInMilliseconds <= 0 || DurationInMilliseconds > std::numeric_limits<uint16_t>::max())
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Tried to vibrate the wrist of a glove with an invalid duration of %d.")
			, static_cast<int>(DurationInMilliseconds));

		return EManusRet::InvalidArgument;
	}

	EManusRet ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_VibrateWristOfGlove);
		ReturnCode = (EManusRet)FunctionPointers->VibrateWristOfGlove(GloveId, UnitStrength, static_cast<uint16_t>(DurationInMilliseconds));
	}
	if (ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to rumble the wrist of the glove with ID %u.")
			, static_cast<unsigned int>(GloveId));
	}

	return ReturnCode;
}

EManusRet CoreSdk::VibrateFingers(int64 DongleId, EManusHandType HandTypeOfGlove, TArray<float> Powers)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_VibrateFingers);
	RETURN_IF_NOT_INITIALISED(TEXT("VibrateFingers"), EManusRet::Error);

	if (DongleId == 0)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Tried to vibrate the fingers of the glove using a dongle with ID %u, but this dongle ID is not valid.")
			, static_cast<unsigned int>(DongleId));

		return EManusRet::InvalidArgument;
	}

	HandType SdkHandType;
	EManusRet ReturnCode = ManusConvert::BpHandTypeToSdk(HandTypeOfGlove, SdkHandType);
	if (ReturnCode != EManusRet::Success)
	{
		return ReturnCode;
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_VibrateFingers);
		ReturnCode = (EManusRet)FunctionPointers->VibrateFingers(DongleId, SdkHandType, Powers.GetData());
	}
	if (ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to rumble the fingers of the glove of hand type %s using a dongle with ID %u.")
			, (HandTypeOfGlove == EManusHandType::Left ? TEXT("left") : TEXT("right"))
			, static_cast<unsigned int>(DongleId));
	}

	return ReturnCode;
}

EManusRet CoreSdk::GetGloveIdOfUser_UsingUserIndex(int32 UserIndex, EManusHandType HandTypeOfGlove, int64& GloveId)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetGloveIdOfUser_UsingUserIndex);
	RETURN_IF_NOT_INITIALISED(TEXT("GetGloveIdOfUser_UsingUserIndex"), EManusRet::Error);

	HandType SdkHandType;
	EManusRet ReturnCode = ManusConvert::BpHandTypeToSdk(HandTypeOfGlove, SdkHandType);
	if (ReturnCode != EManusRet::Success)
	{
		// An error message will be logged in the convert function, so don't print anything here.
		return ReturnCode;
	}

	uint32_t Id = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetGloveIdOfUser_UsingUserIndex);
		ReturnCode = (EManusRet)FunctionPointers->GetGloveIdOfUser_UsingUserIndex(UserIndex, SdkHandType, Id);
	}
	if (ReturnCode != EManusRet::Success)
	{
		return ReturnCode;
	}

	GloveId = Id;

	return EManusRet::Success;
}

EManusRet CoreSdk::GetNumberOfAvailableGloves(int32 &NumberOfAvailableGloves)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetNumberOfAvailableGloves);
	RETURN_IF_NOT_INITIALISED(TEXT("GetGloveIdOfUser_UsingUserIndex"), EManusRet::Error);

	EManusRet ReturnCode = EManusRet::Success;
	uint32_t NumGloves = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetNumberOfAvailableGloves);
		ReturnCode = (EManusRet)FunctionPointers->GetNumberOfAvailableGloves(NumGloves);
	}
	if (ReturnCode != EManusRet::Success)
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

	EManusRet ReturnCode = EManusRet::Success;
	uint32_t NumGloves = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetNumberOfAvailableGloves);
		ReturnCode = (EManusRet)FunctionPointers->GetNumberOfAvailableGloves(NumGloves);
	}
	if (ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the number of available gloves from the SDK."));

		return ReturnCode;
	}

	if (NumGloves == 0)
	{
		// Nothing left to do here, so just return.

		return EManusRet::Success;
	}

	uint32_t *IdsOfGloves = new uint32_t[NumGloves];
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetIdsOfAvailableGloves);
		ReturnCode = (EManusRet)FunctionPointers->GetIdsOfAvailableGloves(IdsOfGloves, NumGloves);
	}
	if (ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the IDs of available gloves from the SDK."));

		return ReturnCode;
	}

	IdsOfAvailableGloves.Reset(static_cast<int32>(NumGloves));

	for (uint32_t Id = 0; Id < NumGloves; Id++)
	{
		IdsOfAvailableGloves.Add(IdsOfGloves[Id]);
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::GetHandTypeOfGlove(int64 GloveId, EManusHandType &HandTypeOfGlove)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetHandTypeOfGlove);
	RETURN_IF_NOT_INITIALISED(TEXT("GetHandTypeOfGlove"), EManusRet::Error);

	if (GloveId == 0)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Tried to get the hand type for a glove with ID %u, but this glove ID is not valid.")
			, static_cast<unsigned int>(GloveId));

		return EManusRet::InvalidArgument;
	}

	EManusRet ReturnCode = EManusRet::Success;
	HandType SdkHandType;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetHandTypeOfGlove);
		ReturnCode = (EManusRet)FunctionPointers->GetHandTypeOfGlove(GloveId, SdkHandType);
	}
	if (ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the hand type for a glove ID from the SDK."));

		return ReturnCode;
	}

	ReturnCode = ManusConvert::SdkHandTypeToBp(SdkHandType, HandTypeOfGlove);
	if (ReturnCode != EManusRet::Success)
	{
		// An error message will be logged in the convert function, so don't print anything here.

		return ReturnCode;
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::GetIdOfFirstAvailableGloveOfType(EManusHandType HandTypeOfGlove, int64 &GloveId)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetIdOfFirstAvailableGloveOfType);

	GloveId = 0;

	RETURN_IF_NOT_INITIALISED(TEXT("GetIdOfFirstAvailableGloveOfType"), EManusRet::Error);

	HandType SdkHandType;
	EManusRet ReturnCode = ManusConvert::BpHandTypeToSdk(HandTypeOfGlove, SdkHandType);
	if (ReturnCode != EManusRet::Success)
	{
		// An error message will be logged in the convert function, so don't print anything here.

		return ReturnCode;
	}

	uint32_t Id = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetIdOfFirstAvailableGloveOfType);
		ReturnCode = (EManusRet)FunctionPointers->GetIdOfFirstAvailableGloveOfType(SdkHandType, Id);
	}
	if (ReturnCode != EManusRet::Success)
	{
		// This can be intended behaviour, so dont log a message here.

		return ReturnCode;
	}

	GloveId = Id;

	return EManusRet::Success;
}

EManusRet CoreSdk::GetDataForGlove_UsingGloveId(int64 GloveId, FManusGlove &DataForGlove)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetDataForGlove_UsingGloveId);
	RETURN_IF_NOT_INITIALISED(TEXT("GetDataForGlove_UsingGloveId"), EManusRet::Error);

	if (GloveId == 0)
	{
		UE_LOG(
			LogManus
			, Verbose
			, TEXT("Tried to get data for a glove with ID %u, but this glove ID is not valid.")
			, static_cast<unsigned int>(GloveId));

		return EManusRet::InvalidArgument;
	}

	EManusRet ReturnCode = EManusRet::Success;
	GloveData Data;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetDataForGlove_UsingGloveId);
		ReturnCode = (EManusRet)FunctionPointers->GetDataForGlove_UsingGloveId(GloveId, Data);
	}
	if (ReturnCode != EManusRet::Success)
	{
		return ReturnCode;
	}

	for (unsigned int FingerNumber = 0; FingerNumber < NUM_FINGERS_ON_HAND; FingerNumber++)
	{
		UE_LOG(
			LogManus
			, Verbose
			, TEXT("[%d]: mcp %d%% q.z=%f, pip %d%% q.z=%f, dip %d%% q.z=%f")
			, FingerNumber
			, static_cast<int>(Data.m_Raw.m_FlexSensor[FingerNumber][0] * 100.0f)     // MCP raw
			, Data.m_Processed.m_Joints[FingerNumber][0].z  // MCP quat. Only quaternion z value printed due to limited space, similar for x/y/w.
			, static_cast<int>(Data.m_Raw.m_FlexSensor[FingerNumber][1] * 100.0f)     // PIP raw
			, Data.m_Processed.m_Joints[FingerNumber][1].z  // PIP quat
			, static_cast<int>(Data.m_Raw.m_FlexSensor[FingerNumber][1] * 100.0f)     // No sensor at DIP, so the PIP value is used...
			, Data.m_Processed.m_Joints[FingerNumber][2].z
		); // ...but the DIP quaternion tuned in the handmodel is different from PIP to make it look natural.
	}

	if (!ConvertSdkGloveDataToBp(Data, DataForGlove))
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed convert CoreSDK glove data to blueprint-compatible glove data for the glove with ID %u.")
			, static_cast<unsigned int>(GloveId));

		return EManusRet::Error;
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::GetDataForGlove_UsingHandType(EManusHandType HandTypeOfGlove, FManusGlove &DataForGlove)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetDataForGlove_UsingHandType);
	RETURN_IF_NOT_INITIALISED(TEXT("GetDataForGlove_UsingHandType"), EManusRet::Error);

	FString HandTypeString = HandTypeOfGlove == EManusHandType::Left ? TEXT("Left") : TEXT("Right");

	HandType SdkHandType;
	EManusRet ReturnCode = ManusConvert::BpHandTypeToSdk(HandTypeOfGlove, SdkHandType);
	if (ReturnCode != EManusRet::Success)
	{
		// An error message will be logged in the convert function, so don't print anything here.

		return ReturnCode;
	}

	GloveData Data;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetDataForGlove_UsingGloveId);
		ReturnCode = (EManusRet)FunctionPointers->GetDataForGlove_UsingHandType(SdkHandType, Data);
	}
	if (ReturnCode != EManusRet::Success)
	{
		return ReturnCode;
	}

	if (!ConvertSdkGloveDataToBp(Data, DataForGlove))
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed convert CoreSDK glove data to blueprint-compatible glove data for the first available glove of type %s.")
			, *HandTypeString);

		return EManusRet::Error;
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::GetNumberOfHapticDongles(int32& NumberOfHapticsDongles)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetNumberOfHapticsDongles);
	RETURN_IF_NOT_INITIALISED(TEXT("GetNumberOfHapticsDongles"), EManusRet::Error);

	uint32_t NumHapticsDongles = 0;
	EManusRet ReturnCode = (EManusRet)FunctionPointers->GetNumberOfHapticsDongles(NumHapticsDongles);
	if (ReturnCode != EManusRet::Success)
	{
		return ReturnCode;
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

	EManusRet ReturnCode = EManusRet::Success;
	uint32_t NumHapticsDongleIds = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetNumberOfHapticsDongles);
		ReturnCode = (EManusRet)FunctionPointers->GetNumberOfHapticsDongles(NumHapticsDongleIds);
	}
	if (ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the number of haptic dongle ids from the SDK."));

		return ReturnCode;
	}

	if (NumHapticsDongleIds == 0)
	{
		return EManusRet::Success;
	}

	uint32_t* IdsOfHapticsDongles = new uint32_t[NumHapticsDongleIds];
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetHapticsDongleIds);
		ReturnCode = (EManusRet)FunctionPointers->GetHapticsDongleIds(IdsOfHapticsDongles, NumHapticsDongleIds);
	}
	if (ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the IDs of available gloves from the SDK."));

		return ReturnCode;
	}

	HapticsDongleIds.Reset(static_cast<int32>(NumHapticsDongleIds));

	for (uint32_t Id = 0; Id < NumHapticsDongleIds; Id++)
	{
		HapticsDongleIds.Add(IdsOfHapticsDongles[Id]);
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::AddOrUpdatePolygonSkeleton(int64 UserIndex, int64 SkeletonId, UManusSkeleton* ManusSkeleton)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_AddOrUpdatePolygonSkeleton);
	RETURN_IF_NOT_INITIALISED(TEXT("AddOrUpdatePolygonSkeleton"), EManusRet::Error);

	TArray<FTransform> SkeletalMeshBoneTransforms = ManusSkeleton->GetSkeleton()->GetReferenceSkeleton().GetRefBonePose();
	TArray<FTransform> ComponentSpaceTransforms;
	FAnimationRuntime::FillUpComponentSpaceTransforms(ManusSkeleton->GetSkeleton()->GetReferenceSkeleton(), SkeletalMeshBoneTransforms, ComponentSpaceTransforms);

	// Convert Polygon skeleton data
	PolygonSkeletonData ConvertedPolygonSkeletonData;
	ConvertedPolygonSkeletonData.m_UserIndex = UserIndex;
	ConvertedPolygonSkeletonData.m_SkeletonId = SkeletonId;
	ConvertedPolygonSkeletonData.m_ScaleToUser = ManusSkeleton->ScaleToUser;
	ConvertedPolygonSkeletonData.m_Height = ManusSkeleton->FullBodyHeight;

	switch (ManusSkeleton->RetargetingTarget)
	{
		case EManusRetargetingTarget::BodyEstimation:
			ConvertedPolygonSkeletonData.m_Target.m_TargetCase = PolygonTargetCase::ePOLYGONTARGETCASE_BODYESTIMATION;
			ConvertedPolygonSkeletonData.m_Target.m_BodyEstimationTarget.m_UserIndex = UserIndex;
			break;
		case EManusRetargetingTarget::TargetSkeleton:
			ConvertedPolygonSkeletonData.m_Target.m_TargetCase = PolygonTargetCase::ePOLYGONTARGETCASE_TARGETSKELETON;

			char* TargetName = TCHAR_TO_ANSI(*ManusSkeleton->TargetName);
			if (strlen(TargetName) > MAX_NUM_CHARS_IN_TARGET_ID)
			{
				UE_LOG(LogManus, Warning, TEXT("Target skeleton name (%s) is too long (max characters is %i)"), *ManusSkeleton->TargetName, MAX_NUM_CHARS_IN_TARGET_ID);
				break;
			}

			strcpy_s(ConvertedPolygonSkeletonData.m_Target.m_TargetSkeletonTarget.m_String, TargetName);
			break;
	}
	

	for (int i = 0; i < NUM_BONES_IN_POLYGON_SKELETON; i++)
	{
		ConvertedPolygonSkeletonData.m_Bones[i].m_ContainsValidBoneData = false;
	}

	// Root bone
	ConvertedPolygonSkeletonData.m_Bones[0].m_ContainsValidBoneData = true;

	FTransform ConvertedTransform = ManusConvert::ConvertUnrealToUnityTransform(ComponentSpaceTransforms[0]);

	FVector Location = ConvertedTransform.GetLocation();
	ConvertedPolygonSkeletonData.m_Bones[0].m_Position.x = Location.X;
	ConvertedPolygonSkeletonData.m_Bones[0].m_Position.y = Location.Y;
	ConvertedPolygonSkeletonData.m_Bones[0].m_Position.z = Location.Z;

	FQuat Quaternion = ConvertedTransform.GetRotation();
	ConvertedPolygonSkeletonData.m_Bones[0].m_Rotation.x = Quaternion.X;
	ConvertedPolygonSkeletonData.m_Bones[0].m_Rotation.y = Quaternion.Y;
	ConvertedPolygonSkeletonData.m_Bones[0].m_Rotation.z = Quaternion.Z;
	ConvertedPolygonSkeletonData.m_Bones[0].m_Rotation.w = Quaternion.W;

	// Skeleton mapped bones
	for (int i = 0; i < NUM_BONES_IN_POLYGON_SKELETON; i++)
	{
		int ManusBoneIndex = ConvertSdkPolygonSkeletonBoneTypeToBp(i);
		if (ManusBoneIndex != INDEX_NONE)
		{
			FName MappedBoneName = GET_BONE_NAME(ManusSkeleton->BoneMap[ManusBoneIndex]);
			int SkeletonBoneIndex = ManusSkeleton->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(MappedBoneName);
			if (SkeletonBoneIndex != INDEX_NONE)
			{
				ConvertedPolygonSkeletonData.m_Bones[i].m_ContainsValidBoneData = true;

				ConvertedTransform = ManusConvert::ConvertUnrealToUnityTransform(ComponentSpaceTransforms[SkeletonBoneIndex]);

				// Send Manus internal orientation (if any)
				if (FQuat* Quat = ManusSkeleton->ManusInternalOrientations.Find(ManusBoneIndex))
				{
					ConvertedTransform.SetRotation(*Quat);
				}

				Location = ConvertedTransform.GetLocation();
				ConvertedPolygonSkeletonData.m_Bones[i].m_Position.x = Location.X;
				ConvertedPolygonSkeletonData.m_Bones[i].m_Position.y = Location.Y;
				ConvertedPolygonSkeletonData.m_Bones[i].m_Position.z = Location.Z;

				Quaternion = ConvertedTransform.GetRotation();
				ConvertedPolygonSkeletonData.m_Bones[i].m_Rotation.x = Quaternion.X;
				ConvertedPolygonSkeletonData.m_Bones[i].m_Rotation.y = Quaternion.Y;
				ConvertedPolygonSkeletonData.m_Bones[i].m_Rotation.z = Quaternion.Z;
				ConvertedPolygonSkeletonData.m_Bones[i].m_Rotation.w = Quaternion.W;
			}
			else
			{
				const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EManusBoneName"), true);
				FString ManusBoneName = EnumPtr->GetNameStringByIndex(ManusBoneIndex);
				UE_LOG(LogManus, Verbose, TEXT("CoreSdk: Initializing skeleton %s to be used with Manus Polygon. Bone \"%s\" mapped to \"%s\" not found. You might want to set a correct bone name for the bone \"%s\" in the Polygon Skeleton Bone Name Map in the corresponding Manus Live Link User."),
					*ManusSkeleton->GetSkeleton()->GetName(),
					*ManusBoneName,
					*GET_BONE_NAME(ManusSkeleton->BoneMap[ManusBoneIndex]).ToString(),
					*ManusBoneName
				);
			}
		}
	}

	EManusRet ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_AddOrUpdatePolygonSkeleton);
		ReturnCode = (EManusRet)FunctionPointers->AddOrUpdatePolygonSkeleton(ConvertedPolygonSkeletonData);
		//UE_LOG(LogManus, Warning, TEXT("Adding polygon state %i"), ReturnCode);
	}
	if (ReturnCode != EManusRet::Success)
	{
		return ReturnCode;
	}

	// Set Retargeting Parameters
	ReturnCode = CoreSdk::SetRetargetingSettings(
		SkeletonId,
		ManusSkeleton->RetargetingParameters);

	if (ReturnCode != EManusRet::Success)
	{
		UE_LOG(LogManus, Warning, TEXT("Manus Polygon Retargeting Settings failed to set"));
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::RemovePolygonSkeleton(int64 PolygonSkeletonId)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_RemovePolygonSkeleton);
	RETURN_IF_NOT_INITIALISED(TEXT("RemovePolygonSkeleton"), EManusRet::Error);

	RemoveSkeletonArgs Args;
	Args.m_SkeletonID = PolygonSkeletonId;

	EManusRet ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_RemovePolygonSkeleton);
		ReturnCode = (EManusRet)FunctionPointers->RemovePolygonSkeleton(Args);
	}
	if (ReturnCode != EManusRet::Success)
	{
		return ReturnCode;
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::SetPolygonSkeletonTarget(FManusPolygonSkeletonTarget SkeletonTarget)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_SetPolygonSkeletonTarget);
	RETURN_IF_NOT_INITIALISED(TEXT("SetPolygonSkeletonTarget"), EManusRet::Error);



	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_SetPolygonSkeletonTarget);
	return EManusRet::Success;
}

EManusRet CoreSdk::SetRetargetingSettings(int64 PolygonSkeletonId, FManusPolygonParameters PolygonSkeletonRetargetingSettings)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_SetRetargetingSettings);
	RETURN_IF_NOT_INITIALISED(TEXT("SetRetargetingSettings"), EManusRet::Error);

	RetargetingSettingsArgs RetargetingSettingsArgs;
	RetargetingSettingsArgs.m_ID = PolygonSkeletonId;

	RetargetingSettings RetargetingSettings;
	RetargetingSettings.m_MatchSameSpeed = PolygonSkeletonRetargetingSettings.MatchSameSpeed;
	RetargetingSettings.m_HipHeightMultiplier = PolygonSkeletonRetargetingSettings.HipHeightMultiplier;
	RetargetingSettings.m_LegWidth = PolygonSkeletonRetargetingSettings.LegWidth;
	RetargetingSettings.m_KneeRotation = PolygonSkeletonRetargetingSettings.KneeRotation;
	RetargetingSettings.m_ShoulderForwardOffset = PolygonSkeletonRetargetingSettings.ShoulderForwardOffset;
	RetargetingSettings.m_ShoulderHeightOffset = PolygonSkeletonRetargetingSettings.ShoulderHeightOffset;
	RetargetingSettings.m_ShoulderForwardRotationMultiplier = PolygonSkeletonRetargetingSettings.ShoulderForwardMultiplier;
	RetargetingSettings.m_ShoulderHeightRotationMultiplier = PolygonSkeletonRetargetingSettings.ShoulderHeightMultiplier;
	RetargetingSettings.m_ElbowRotation = PolygonSkeletonRetargetingSettings.ElbowRotation;
	RetargetingSettings.m_HandLocalPosition = (HandLocalTo)PolygonSkeletonRetargetingSettings.HandLocalPosition;
	RetargetingSettings.m_HandLocalRotation = (HandLocalTo)PolygonSkeletonRetargetingSettings.HandLocalRotation;
	RetargetingSettings.m_ArmSpacing = PolygonSkeletonRetargetingSettings.ArmSpacing;
	RetargetingSettings.m_ArmLengthMultiplier = PolygonSkeletonRetargetingSettings.ArmLengthMultiplier;
	RetargetingSettings.m_ArmIK = PolygonSkeletonRetargetingSettings.ArmIK;
	RetargetingSettings.m_HandRotationLocal = PolygonSkeletonRetargetingSettings.HandRotationLocal;
	RetargetingSettings.m_HandForwardOffset = PolygonSkeletonRetargetingSettings.HandForwardOffset;
	RetargetingSettings.m_HandWidthOffset = PolygonSkeletonRetargetingSettings.HandWidthOffset;
	RetargetingSettings.m_HandHeightOffset = PolygonSkeletonRetargetingSettings.HandHeightOffset;
	RetargetingSettings.m_HandForwardMultiplier = PolygonSkeletonRetargetingSettings.HandForwardMultiplier;
	RetargetingSettings.m_HandWidthMultiplier = PolygonSkeletonRetargetingSettings.HandWidthMultiplier;
	RetargetingSettings.m_HandHeightMultiplier = PolygonSkeletonRetargetingSettings.HandHeightMultiplier;
	RetargetingSettings.m_DefaultHipBend = PolygonSkeletonRetargetingSettings.DefaultHipBend;
	RetargetingSettings.m_DefaultSpineBend = PolygonSkeletonRetargetingSettings.DefaultSpineBend;
	RetargetingSettings.m_DefaultNeckBend = PolygonSkeletonRetargetingSettings.DefaultNeckBend;
	RetargetingSettings.m_SpineBendMultiplier = PolygonSkeletonRetargetingSettings.SpineBendMultiplier;
	RetargetingSettings.m_SpineAngleMultiplier = PolygonSkeletonRetargetingSettings.SpineAngleMultiplier;
	RetargetingSettings.m_SpineTwistMultiplier = PolygonSkeletonRetargetingSettings.SpineTwistMultiplier;
	
	RetargetingSettingsArgs.m_Settings = RetargetingSettings;

	EManusRet ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_SetRetargetingSettings);
		ReturnCode = (EManusRet)FunctionPointers->SetPolygonRetargetingSettings(RetargetingSettingsArgs);
	}
	if (ReturnCode != EManusRet::Success)
	{
		return ReturnCode;
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::IsPolygonSkeletonIdValid(int64 PolygonSkeletonId, bool& IsValid)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_IsPolygonSkeletonIdValid);
	RETURN_IF_NOT_INITIALISED(TEXT("IsPolygonSkeletonIdValid"), EManusRet::Error);

	EManusRet ReturnCode = EManusRet::Success;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_IsPolygonSkeletonIdValid);
		ReturnCode = (EManusRet)FunctionPointers->IsPolygonSkeletonIdValid(PolygonSkeletonId, IsValid);
	}
	return ReturnCode;
}

EManusRet CoreSdk::GetNumberOfAvailablePolygonSkeletons(int32& NumberOfAvailablePolygonSkeletons)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetNumberOfAvailablePolygonSkeletons);
	RETURN_IF_NOT_INITIALISED(TEXT("GetNumberOfAvailablePolygonSkeletons"), EManusRet::Error);

	EManusRet ReturnCode = EManusRet::Success;
	uint32_t NumPolygonSkeletons = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetNumberOfAvailablePolygonSkeletons);
		ReturnCode = (EManusRet)FunctionPointers->GetNumberOfAvailablePolygonSkeletons(NumPolygonSkeletons);
	}
	if (ReturnCode != EManusRet::Success)
	{
		return ReturnCode;
	}

	NumberOfAvailablePolygonSkeletons = static_cast<int32>(NumPolygonSkeletons);

	return EManusRet::Success;
}

EManusRet CoreSdk::GetIdsOfAvailablePolygonSkeletons(TArray<int64>& IdsOfAvailablePolygonSkeletons)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetIdsOfAvailablePolygonSkeletons);
	RETURN_IF_NOT_INITIALISED(TEXT("GetIdsOfAvailablePolygonSkeletons"), EManusRet::Error);

	if (IdsOfAvailablePolygonSkeletons.Num() != 0)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Attempted to get an array of Polygon skeleton IDs with a non-empty array."));

		return EManusRet::InvalidArgument;
	}

	EManusRet ReturnCode = EManusRet::Success;
	uint32_t NumPolygonSkeletons = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetNumberOfAvailablePolygonSkeletons);
		ReturnCode = (EManusRet)FunctionPointers->GetNumberOfAvailablePolygonSkeletons(NumPolygonSkeletons);
	}
	if (ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the number of available Polygon skeletons from the SDK."));

		return ReturnCode;
	}

	if (NumPolygonSkeletons == 0)
	{
		// Nothing left to do here, so just return.

		return EManusRet::Success;
	}

	uint32_t* IdsOfPolygonSkeletons = new uint32_t[NumPolygonSkeletons];
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetIdsOfAvailablePolygonSkeletons);
		ReturnCode = (EManusRet)FunctionPointers->GetIdsOfAvailablePolygonSkeletons(IdsOfPolygonSkeletons, NumPolygonSkeletons);
	}
	if (ReturnCode != EManusRet::Success)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed to get the IDs of available Polygon skeletons from the SDK."));

		return ReturnCode;
	}

	IdsOfAvailablePolygonSkeletons.Reset(static_cast<int32>(NumPolygonSkeletons));

	for (uint32_t Id = 0; Id < NumPolygonSkeletons; Id++)
	{
		IdsOfAvailablePolygonSkeletons.Add(IdsOfPolygonSkeletons[Id]);
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::GetIdOfFirstAvailablePolygonSkeleton(int64& PolygonSkeletonId)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetIdOfFirstAvailablePolygonSkeleton);
	RETURN_IF_NOT_INITIALISED(TEXT("GetIdOfFirstAvailablePolygonSkeleton"), EManusRet::Error);

	PolygonSkeletonId = 0;

	EManusRet ReturnCode = EManusRet::Success;
	uint32_t Id = 0;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetIdOfFirstAvailablePolygonSkeleton);
		ReturnCode = (EManusRet)FunctionPointers->GetIdOfFirstAvailablePolygonSkeleton(Id);
	}
	if (ReturnCode != EManusRet::Success)
	{
		// This can be intended behaviour, so dont log a message here.

		return ReturnCode;
	}

	PolygonSkeletonId = Id;

	return EManusRet::Success;
}

EManusRet CoreSdk::GetDataForPolygonSkeleton(int64 PolygonSkeletonId, FManusPolygonSkeleton& DataForPolygonSkeleton)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetDataForPolygonSkeleton);
	RETURN_IF_NOT_INITIALISED(TEXT("GetDataForPolygonSkeleton"), EManusRet::Error);

	if (PolygonSkeletonId == 0)
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Tried to get data for a Polygon skeleton with ID %u, but this Polygon skeleton ID is not valid.")
			, static_cast<unsigned int>(PolygonSkeletonId));

		return EManusRet::InvalidArgument;
	}

	EManusRet ReturnCode = EManusRet::Success;
	PolygonSkeletonData Data;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetDataForPolygonSkeleton);
		ReturnCode = (EManusRet)FunctionPointers->GetDataForPolygonSkeleton(PolygonSkeletonId, Data);
	}
	if (ReturnCode != EManusRet::Success)
	{
		return ReturnCode;
	}

	if (!ConvertSdkPolygonSkeletonDataToBp(Data, DataForPolygonSkeleton))
	{
		UE_LOG(
			LogManus
			, Warning
			, TEXT("Failed convert CoreSDK Polygon skeleton data to blueprint-compatible Polygon skeleton data for the Polygon skeleton with ID %u.")
			, static_cast<unsigned int>(PolygonSkeletonId));

		return EManusRet::Error;
	}

	return EManusRet::Success;
}

EManusRet CoreSdk::GetDataForTracker(int32 UserIndex, EManusHandType HandTypeOfTracker, FManusTracker& DataForTracker)
{
	SCOPE_CYCLE_COUNTER(STAT_CoreSDK_GetDataForTracker_UsingIndexAndType);
	RETURN_IF_NOT_INITIALISED(TEXT("GetDataForTracker_UsingIndexAndType"), EManusRet::Error);

	uint32_t SdkTrackerType;
	EManusRet ReturnCode = ManusConvert::BpHandTypeToSdkTrackerType(HandTypeOfTracker, SdkTrackerType);
	if (ReturnCode != EManusRet::Success)
	{
		return ReturnCode;
	}

	TrackerData Data;
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreSDK_DLL_GetDataForTracker_UsingIndexAndType);
		ReturnCode = (EManusRet)FunctionPointers->GetDataForTracker_UsingIndexAndType(UserIndex, SdkTrackerType, Data);
	}
	if (ReturnCode != EManusRet::Success)
	{
		return ReturnCode;
	}

	if (!ConvertSdkTrackerDataToBp(Data, DataForTracker))
	{
		UE_LOG(LogManus, Warning, TEXT("Failed convert CoreSDK Tracker data to blueprint-compatible Tracker data for the Tracker from the User Index %d and Hand Type %d."), UserIndex, (int)HandTypeOfTracker);

		return EManusRet::Error;
	}

	return EManusRet::Success;
}

///////////////////////////////////////////////////////////////////////////////
// Locally declared functions.

bool ConvertSdkGloveDataToBp(const GloveData &SdkInput, FManusGlove &BpOutput)
{
	BpOutput.LastUpdateTime = SdkInput.m_LastUpdateTime;

	BpOutput.Fingers.Reset(NUM_FINGERS_ON_HAND);

	for (int Finger = 0; Finger < NUM_FINGERS_ON_HAND; Finger++)
	{
		FManusFingerProcessed NewFinger;

		NewFinger.Joints.Reset(NUM_PHALANGES_IN_FINGER);

		for (int Phalange = 0; Phalange < NUM_PHALANGES_IN_FINGER; Phalange++)
		{
			FManusFingerJoint NewFingerJoint;

			NewFingerJoint.Rotation = FRotator(FQuat(
				SdkInput.m_Processed.m_Joints[Finger][Phalange].x
				, SdkInput.m_Processed.m_Joints[Finger][Phalange].y
				, SdkInput.m_Processed.m_Joints[Finger][Phalange].z
				, SdkInput.m_Processed.m_Joints[Finger][Phalange].w));

			NewFingerJoint.Stretch = SdkInput.m_Processed.m_Stretch[Finger][Phalange];

			NewFinger.Joints.Add(NewFingerJoint);

			UE_LOG(
				LogManus
				, Verbose
				, TEXT("----------> Finger=%d Phalange=%d Rot=%f %f %f from Joints=%f %f %f %f")
				, Finger
				, Phalange
				, NewFingerJoint.Rotation.Yaw
				, NewFingerJoint.Rotation.Roll
				, NewFingerJoint.Rotation.Pitch
				, SdkInput.m_Processed.m_Joints[Finger][Phalange].x
				, SdkInput.m_Processed.m_Joints[Finger][Phalange].y
				, SdkInput.m_Processed.m_Joints[Finger][Phalange].z
				, SdkInput.m_Processed.m_Joints[Finger][Phalange].w);
		}

		NewFinger.Spread = SdkInput.m_Processed.m_Spread[Finger];

		BpOutput.Fingers.Add(NewFinger);
	}

	FQuat ConvertedWristRotation = ManusConvert::ConvertUnityToUnrealQuat(FQuat(SdkInput.m_Processed.m_WristImu.x, SdkInput.m_Processed.m_WristImu.y, SdkInput.m_Processed.m_WristImu.z, SdkInput.m_Processed.m_WristImu.w));
	if (SdkInput.m_HandType == HandType::eHANDTYPE_RIGHT_HAND)
		ConvertedWristRotation = FQuat::MakeFromEuler(FVector(180, 0, 180)) * (ConvertedWristRotation * FQuat::MakeFromEuler(FVector(180, 0, -90)));
	else
		ConvertedWristRotation = FQuat::MakeFromEuler(FVector(0, 0, 0)) * (ConvertedWristRotation * FQuat::MakeFromEuler(FVector(0, 0, 90)));

	BpOutput.WristImuOrientation = ConvertedWristRotation;// 

	BpOutput.WristTrackerTransforms.Reset(MAX_NUM_USERS);
	for (int UserIndex = 0; UserIndex < MAX_NUM_USERS; UserIndex++)
	{
		BpOutput.WristTrackerTransforms.Add(ManusConvert::ConvertUnityToUnrealTransform(FTransform(
			FQuat(
				SdkInput.m_Processed.m_WristTransforms[UserIndex].m_Orientation.x, 
				SdkInput.m_Processed.m_WristTransforms[UserIndex].m_Orientation.y, 
				SdkInput.m_Processed.m_WristTransforms[UserIndex].m_Orientation.z, 
				SdkInput.m_Processed.m_WristTransforms[UserIndex].m_Orientation.w),
			FVector(
				SdkInput.m_Processed.m_WristTransforms[UserIndex].m_Position.x, 
				SdkInput.m_Processed.m_WristTransforms[UserIndex].m_Position.y, 
				SdkInput.m_Processed.m_WristTransforms[UserIndex].m_Position.z),
			SdkInput.m_Processed.m_WristTransforms[UserIndex].m_DataIsAvailable ? FVector::OneVector : FVector::ZeroVector
		)));
	}

	BpOutput.GloveInfo.GloveId = SdkInput.m_GloveId;
	BpOutput.GloveInfo.DongleId = SdkInput.m_DongleId;

	if (ManusConvert::SdkHandTypeToBp(SdkInput.m_HandType, BpOutput.GloveInfo.HandType) != EManusRet::Success)
	{
		// An error message will be logged in the convert function, so don't print anything here.

		return false;
	}

	if (SdkInput.m_LowFrequencyData.m_DataIsAvailable)
	{
		if (ManusConvert::SdkGloveTypeToBp(SdkInput.m_LowFrequencyData.m_GloveType, BpOutput.GloveInfo.GloveType) != EManusRet::Success)
		{
			// An error message will be logged in the convert function, so don't print anything here.

			return false;
		}
	}

	BpOutput.GloveInfo.BatteryPercentage = static_cast<int32>(SdkInput.m_LowFrequencyData.m_BatteryPercentage);
	BpOutput.GloveInfo.TransmissionStrengthInDb = static_cast<int32>(SdkInput.m_LowFrequencyData.m_TransmissionStrengthInDb);
	BpOutput.GloveInfo.ReceivedLowFrequencyData = SdkInput.m_LowFrequencyData.m_DataIsAvailable;

	BpOutput.RawData.Fingers.Reset(NUM_FINGERS_ON_HAND);

	for (int Finger = 0; Finger < NUM_FINGERS_ON_HAND; Finger++)
	{
		FManusFingerRaw NewFinger;
		int32 NewFingerIndex = BpOutput.RawData.Fingers.Add(NewFinger);

		BpOutput.RawData.Fingers[NewFingerIndex].McpFlexSensor = SdkInput.m_Raw.m_FlexSensor[Finger][FLEX_SENSOR_MCP];
		BpOutput.RawData.Fingers[NewFingerIndex].PipFlexSensor = SdkInput.m_Raw.m_FlexSensor[Finger][FLEX_SENSOR_PIP];
	}

	BpOutput.RawData.Imus.Reset(MAX_NUM_IMUS_ON_GLOVE);
	for (int i = 0; i < MAX_NUM_IMUS_ON_GLOVE; i++)
	{
		BpOutput.RawData.Imus.Add(FRotator(FQuat(
			SdkInput.m_Raw.m_Imus[i].x
			, SdkInput.m_Raw.m_Imus[i].y
			, SdkInput.m_Raw.m_Imus[i].z
			, SdkInput.m_Raw.m_Imus[i].w)));
	}

	return true;
}

bool ConvertSdkPolygonSkeletonDataToBp(const PolygonSkeletonData& SdkInput, FManusPolygonSkeleton& BpOutput)
{
	BpOutput.LastUpdateTime = SdkInput.m_LastUpdateTime;

	BpOutput.Bones.SetNum(BODY_LIVE_LINK_BONE_NUM);

	for (int BoneType = 0; BoneType < NUM_BONES_IN_POLYGON_SKELETON; BoneType++)
	{
		if (SdkInput.m_Bones[BoneType].m_ContainsValidBoneData)
		{
			// Convert Core bone type to Unreal bone index:
			// - See Skeleton.pb.h BoneType enum for the values of BoneType
			// - See ManusBlueprintTypes.h EManusBoneName enum for the values of BoneIndex
			int BoneIndex = ConvertSdkPolygonSkeletonBoneTypeToBp(BoneType);
			if (BoneIndex != INDEX_NONE)
			{
				FManusPolygonBone NewBone;

				NewBone.Validity = true;

				NewBone.Transform = ManusConvert::ConvertUnityToUnrealTransform(FTransform(
					FQuat(SdkInput.m_Bones[BoneType].m_Rotation.x, SdkInput.m_Bones[BoneType].m_Rotation.y, SdkInput.m_Bones[BoneType].m_Rotation.z, SdkInput.m_Bones[BoneType].m_Rotation.w),
					FVector(SdkInput.m_Bones[BoneType].m_Position.x, SdkInput.m_Bones[BoneType].m_Position.y, SdkInput.m_Bones[BoneType].m_Position.z),
					FVector(SdkInput.m_Bones[BoneType].m_Scale > 0.0f ? SdkInput.m_Bones[BoneType].m_Scale : 1.0f)
				));

				BpOutput.Bones[BoneIndex] = NewBone;
			}
		}
	}

	BpOutput.PolygonSkeletonId = SdkInput.m_SkeletonId;

	return true;
}

int ConvertSdkPolygonSkeletonBoneTypeToBp(int BoneType)
{
	int BoneIndex = INDEX_NONE;
	switch (BoneType)
	{
	case 0:		BoneIndex = (int)EManusBoneName::Root;				break;
	case 1:		BoneIndex = (int)EManusBoneName::Head;				break;
	case 2:		BoneIndex = (int)EManusBoneName::Neck;				break;
	case 3:		BoneIndex = (int)EManusBoneName::Hips;				break;
	case 4:		BoneIndex = (int)EManusBoneName::Spine;				break;
	case 5:		BoneIndex = (int)EManusBoneName::Chest;				break;
	case 6:		BoneIndex = (int)EManusBoneName::UpperChest;		break;
	case 7:		BoneIndex = (int)EManusBoneName::LeftUpperLeg;		break;
	case 8:		BoneIndex = (int)EManusBoneName::RightUpperLeg;		break;
	case 9:		BoneIndex = (int)EManusBoneName::LeftLowerLeg;		break;
	case 10:	BoneIndex = (int)EManusBoneName::RightLowerLeg;		break;
	case 11:	BoneIndex = (int)EManusBoneName::LeftFoot;			break;
	case 12:	BoneIndex = (int)EManusBoneName::RightFoot;			break;
	case 13:	BoneIndex = (int)EManusBoneName::LeftToes;			break;
	case 14:	BoneIndex = (int)EManusBoneName::RightToes;			break;
	case 15:	BoneIndex = (int)EManusBoneName::LeftToesEnd;		break;
	case 16:	BoneIndex = (int)EManusBoneName::RightToesEnd;		break;
	case 17:	BoneIndex = (int)EManusBoneName::LeftShoulder;		break;
	case 18:	BoneIndex = (int)EManusBoneName::RightShoulder;		break;
	case 19:	BoneIndex = (int)EManusBoneName::LeftUpperArm;		break;
	case 20:	BoneIndex = (int)EManusBoneName::RightUpperArm;		break;
	case 21:	BoneIndex = (int)EManusBoneName::LeftLowerArm;		break;
	case 22:	BoneIndex = (int)EManusBoneName::RightLowerArm;		break;
	case 23:	BoneIndex = (int)EManusBoneName::LeftHand;			break;
	case 24:	BoneIndex = (int)EManusBoneName::RightHand;			break;
	}
	return BoneIndex;
}

bool ConvertSdkTrackerDataToBp(const TrackerData& SdkInput, FManusTracker& BpOutput)
{
	BpOutput.LastUpdateTime = SdkInput.m_LastUpdateTime;

	BpOutput.TrackerId = FString(ANSI_TO_TCHAR(SdkInput.m_TrackerId.m_String));

	BpOutput.Transform = ManusConvert::ConvertUnityToUnrealTransform(FTransform(
		FQuat(SdkInput.m_Rotation.x, SdkInput.m_Rotation.y, SdkInput.m_Rotation.z, SdkInput.m_Rotation.w),
		FVector(SdkInput.m_Position.x, SdkInput.m_Position.y, SdkInput.m_Position.z)
	));

	//BpOutput.Transform.SetRotation(BpOutput.Transform.GetRotation() * FQuat::MakeFromEuler(FVector(0, -90, 0)));

	BpOutput.UserIndex = SdkInput.m_UserIndex;

	if (ManusConvert::SdkTrackerTypeToBpHandType(SdkInput.m_Type, BpOutput.HandType) != EManusRet::Success)
	{
		return false;
	}

	return true;
}

// Callbacks
void OnConnectedToCore(void) 
{
	UE_LOG(LogManus, Log, TEXT("Connected to manus core"));
	
	CoreSdk::CheckCompatibility();
}
