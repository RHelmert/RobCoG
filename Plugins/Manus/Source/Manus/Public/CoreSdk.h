// Copyright 2015-2022 Manus

#pragma once

// Set up a Doxygen group.
/** @addtogroup CoreSdk
 *  @{
 */
#include <vector>
#include "ManusBlueprintTypes.h"
#include "ManusSdkTypes.h"


struct ClientGestures
{
    GestureProbabilities info;
    std::vector<GestureProbability> probabilities;
};

class ClientSkeletonCollection;
class ClientSkeleton;
/// @brief Manages all function pointers for the (wrapped) CoreSDK.
/// Used by the blueprint library.
/// Use the blueprint library instead of this whenever possible.
/// or expand blueprintlibrary to call this if needed.
class MANUS_API CoreSdk
{
public:
	/// @brief check if already initialized
	/// @return returncode for success state
	static EManusRet IsInitialized();
    /// @brief initialize SDK DLL
    /// @return returncode for success state
	static EManusRet Initialize();
    /// @brief shutdown SDK DLL
    /// @return returncode for success state
	static EManusRet ShutDown();
    /// @brief connect to local %Manus Core host
    /// @return returncode for success state
	static EManusRet ConnectLocally();
    /// @brief connect to remote host
    /// @param p_Host address to connect to
    /// @return returncode for success state
	static EManusRet ConnectRemotely(FString p_Host);
    /// @brief find all remote hosts available
	/// @param p_Hosts list that will be filled with found hosts
    /// @return returncode for success state
	static EManusRet FindRemoteHosts(TArray<FString>& p_Hosts);
    /// @brief check if connection is made
    /// @return returncode for success state
	static EManusRet CheckConnection();
    /// @brief get current session id
	/// @param p_SessionId 
    /// @return returncode for success state
	static EManusRet GetSessionId(uint32_t& p_SessionId);
    /// @brief check if this version is compatible with %Manus Core.
    /// @return returncode for success state
	static EManusRet CheckCompatibility();
    /// @brief vibrate fingers for dongle based on dongle id and hand type.
	/// @param p_DongleId 
	/// @param p_HandTypeOfGlove 
	/// @param p_Powers set the strenght of haptics per finger. (range from 0 (off) to 1(max) )
    /// @return returncode for success state
	static EManusRet VibrateFingers(int64 p_DongleId, EManusHandType p_HandTypeOfGlove, TArray<float> p_Powers);
    /// @brief vibrate fingers for specific skeleton
	/// @param p_SkeletonId 
	/// @param p_HandTypeOfGlove 
    /// @param p_Powers set the strenght of haptics per finger. (range from 0 (off) to 1(max) )
    /// @return returncode for success state
	static EManusRet VibrateFingersForSkeleton(int64 p_SkeletonId, EManusHandType p_HandTypeOfGlove, TArray<float> p_Powers);
    /// @brief get glove id of user using user index and hand type.
	/// @param UserIndex 
	/// @param p_HandTypeOfGlove 
	/// @param p_GloveId 
    /// @return returncode for success state
	static EManusRet GetGloveIdOfUser_UsingUserIndex(int32 UserIndex, EManusHandType p_HandTypeOfGlove, int64& p_GloveId);
    /// @brief get glove id of user using user id and handtype
	/// @param UserId 
	/// @param p_HandTypeOfGlove 
	/// @param p_GloveId 
    /// @return returncode for success state
	static EManusRet GetGloveIdOfUser_UsingUserId(int32 UserId, EManusHandType p_HandTypeOfGlove, int64& p_GloveId);
    /// @brief get number of available users
	/// @param NumberOfUsers 
    /// @return returncode for success state
	static EManusRet GetNumberOfAvailableUsers(int32& NumberOfUsers);
    /// @brief get ids of all current users.
	/// @param IdsOfAvailableUsers 
    /// @return returncode for success state
	static EManusRet GetIdsOfUsers(TArray<int64>& IdsOfAvailableUsers);
    /// @brief get number of available gloves
	/// @param NumberOfAvailableGloves 
    /// @return returncode for success state
	static EManusRet GetNumberOfAvailableGloves(int32 &NumberOfAvailableGloves);
    /// @brief get ids of all available gloves
	/// @param IdsOfAvailableGloves 
    /// @return returncode for success state
	static EManusRet GetIdsOfAvailableGloves(TArray<int64> &IdsOfAvailableGloves);
    /// @brief get all the meta data of a glove using glove id
	/// @param p_GloveId 
	/// @param p_GloveData 
    /// @return returncode for success state
	static EManusRet GetDataForGlove_UsingGloveId(uint32_t p_GloveId, GloveLandscapeData& p_GloveData);
    /// @brief get count of all the haptic dongles
	/// @param NumberOfHapticDongles
    /// @return returncode for success state
	static EManusRet GetNumberOfHapticDongles(int32& NumberOfHapticDongles);
    /// @brief get all the haptic dongle id's (p2 haptic dongles have the same id as their non haptic side)
	/// @param HapticDongleIds
    /// @return returncode for success state
	static EManusRet GetHapticDongleIds(TArray<int64>& HapticDongleIds);
    /// @brief get the data for a skeleton (every transform and node id you need to animate it)
    /// @param SkeletonId the id of the skeleton data we want
    /// @param DataForMayoSkeleton the pre-allocated data structure into which the skeleton data will be copied
    /// @return returncode for success state
    static EManusRet GetDataForSkeleton(int64 SkeletonId, FManusMetaSkeleton& DataForMayoSkeleton);
    /// @brief get all the tracker id's
	/// @param p_TrackerIds
    /// @return returncode for success state
	static EManusRet GetTrackerIds(TArray<FString>& p_TrackerIds);
    /// @brief assign a tracker to a user
	/// @param p_TrackerId
	/// @param p_UserId
    /// @return returncode for success state
	static EManusRet AssignTrackerToUser(char* p_TrackerId, int64 p_UserId);
    /// @brief get data for a specific tracker
	/// @param UserId
	/// @param HandTypeOfTracker
	/// @param DataForTracker
    /// @return returncode for success state
	static EManusRet GetDataForTracker(uint32 UserId, EManusHandType HandTypeOfTracker, FManusTracker& DataForTracker); // TODO old
    /// @brief get ergonomics data for a glove.
	/// @param p_Data
	/// @param p_GloveId
    /// @return returncode for success state
	static EManusRet GetErgonomicsDataForGlove(FManusErgonomicsData& p_Data, uint32_t p_GloveId);

    /// @brief create skeleton at SDK lvl and get the index associated to it.
	/// @param p_Skeleton
	/// @param p_SkeletonSetupIndex
    /// @return returncode for success state
	static EManusRet CreateSkeletonSetup(const SkeletonSetupInfo& p_Skeleton, uint32_t& p_SkeletonSetupIndex);
    /// @brief clear all temporary skeletons associated to the current session both in the sdk and core
    /// @return returncode for success state
    static EManusRet ClearAllTemporarySkeletons();
    /// @brief clear temporary skeleton
	/// @param p_SkeletonSetupIndex
	/// @param p_SessionId
    /// @return returncode for success state
	static EManusRet ClearTemporarySkeleton(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId);
    /// @brief add node to temporary skeleton
	/// @param p_SkeletonSetupIndex
	/// @param p_Node
    /// @return returncode for success state
	static EManusRet AddNodeToSkeletonSetup(uint32_t p_SkeletonSetupIndex, const NodeSetup& p_Node);
    /// @brief add chain to temporary skeleton
	/// @param p_SkeletonSetupIndex
	/// @param p_Chain
    /// @return returncode for success state
	static EManusRet AddChainToSkeletonSetup(uint32_t p_SkeletonSetupIndex, const ChainSetup& p_Chain);

    static EManusRet AddMeshSetupToSkeletonSetup(uint32_t p_SkeletonSetupIndex, uint32_t p_NodeID, uint32_t* p_MeshSetupIndex);
    static EManusRet AddVertexToMeshSetup(uint32_t p_SkeletonSetupIndex, uint32_t p_MeshSetupIndex, Vertex p_Vertex);
    static EManusRet AddTriangleToMeshSetup(uint32_t p_SkeletonSetupIndex, uint32_t p_MeshSetupIndex, Triangle p_Triangle);


    /// @brief overwrite an existing SkeletonSetup at a given index.
    /// @return returncode for success state
    static EManusRet OverwriteSkeletonSetup(uint32_t p_SkeletonIndex, const SkeletonSetupInfo& p_Skeleton);
    /// @brief overwrite existing node in temporary skeleton
    /// @return returncode for success state
    static EManusRet OverwriteNodeToSkeletonSetup(uint32_t p_SkeletonIndex, const NodeSetup& p_Node);
    /// @brief overwrite existing chain in temporary skeleton
	/// @param p_SkeletonIndex
	/// @param p_Chain
    /// @return returncode for success state
	static EManusRet OverwriteChainToSkeletonSetup(uint32_t p_SkeletonIndex, const ChainSetup& p_Chain);
    /// @brief get current temporary skeleton nodes and chain sizes.
	/// @param p_SkeletonSetupIndex
	/// @param p_SkeletonInfo
    /// @return returncode for success state
	static EManusRet GetSkeletonSetupArraySizes(uint32_t p_SkeletonSetupIndex, SkeletonSetupArraySizes& p_SkeletonInfo);
    /// @brief automatically let %Manus Core assign chains to the skeleton. This can fail if the skeleton is unrecognizable
	/// @param p_SkeletonIndex
    /// @return returncode for success state
	static EManusRet AllocateChainsForSkeletonSetup(uint32_t p_SkeletonIndex);
    /// @brief gets the skeleton setup info based on the index
    /// @param p_SkeletonSetupIndex 
    /// @param p_SDK locally allocated instance of SkeletonSetupInfo of which the pointer is passed along.
    /// @return 
    static EManusRet GetSkeletonSetupInfo(uint32_t p_SkeletonSetupIndex, SkeletonSetupInfo* p_SDK);
    /// @brief get the chains of the skeleton 
	/// @param p_SkeletonIndex
	/// @param p_SDK
    /// @return returncode for success state
	static EManusRet GetSkeletonSetupChains(uint32_t p_SkeletonIndex, ChainSetup* p_SDK);
    /// @brief get the nodes of the skeleton
	/// @param p_SkeletonIndex
	/// @param p_SDK
    /// @return returncode for success state
	static EManusRet GetSkeletonSetupNodes(uint32_t p_SkeletonIndex, NodeSetup* p_SDK);
    /// @brief load the skeleton into %Manus Core
	/// @param p_SkeletonIndex
	/// @param p_SkeletonId
    /// @return returncode for success state
	static EManusRet LoadSkeleton(uint32_t p_SkeletonIndex, uint32_t& p_SkeletonId);
    /// @brief remove skeleton from %Manus Core
	/// @param p_SkeletonId
    /// @return returncode for success state
	static EManusRet UnloadSkeleton(uint32_t p_SkeletonId);
    /// @brief send temporary skeleton to %Manus Core
	/// @param p_SkeletonSetupIndex
	/// @param p_SessionId
	/// @param p_IsSkeletonModified
    /// @return returncode for success state
	static EManusRet SaveTemporarySkeleton(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId, bool p_IsSkeletonModified);
    /// @brief get temporary skeleton from core
	/// @param p_SkeletonSetupIndex
	/// @param p_SessionId
    /// @return returncode for success state
	static EManusRet GetTemporarySkeleton(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId);
    /// @brief signal %Manus Core to compress temporary skeleton
	/// @param p_SkeletonSetupIndex
	/// @param p_SessionId
	/// @param p_TemporarySkeletonLengthInBytes
    /// @return returncode for success state
	static EManusRet CompressTemporarySkeletonAndGetSize(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId, uint32_t& p_TemporarySkeletonLengthInBytes);
    /// @brief get the compressed temporary skeleton data
	/// @param p_TemporarySkeletonData
	/// @param p_TemporarySkeletonLengthInBytes
    /// @return returncode for success state
	static EManusRet GetCompressedTemporarySkeletonData(unsigned char* p_TemporarySkeletonData, uint32_t p_TemporarySkeletonLengthInBytes);
    /// @brief get the temporary skeleton from a compressed file
	/// @param p_SkeletonSetupIndex
	/// @param p_SessionId
	/// @param p_TemporarySkeletonData
	/// @param p_TemporarySkeletonLengthInBytes
    /// @return returncode for success state
	static EManusRet GetTemporarySkeletonFromCompressedData(uint32_t p_SkeletonSetupIndex, uint32_t p_SessionId, unsigned char* p_TemporarySkeletonData, uint32_t p_TemporarySkeletonLengthInBytes);
    /// @brief get the last modified skeleton index for this session.
	/// @param p_Index
    /// @return returncode for success state
	static void GetLastModifiedSkeletonIndex(uint32_t& p_Index);

    /// @brief get specific gesture probabilities found after the callback
    /// @param p_GestureStreamDataIndex 
    /// @param p_StartDataIndex 
    /// @param p_GestureProbabilitiesCollection 
    /// @return 
    static EManusRet GetGestureStreamData(uint32_t p_GestureStreamDataIndex, uint32_t p_StartDataIndex, GestureProbabilities* p_GestureProbabilitiesCollection);
    
    /// @brief unreal client can use this to get gesture probabilities. may contain nothing, requires an empty vector as input.
    /// @param p_ClientGestures 
    static void GetGestureProbabilities(std::vector<ClientGestures>& p_ClientGestures);

    /// @brief based on the manus landscape, a number of gestures have been defined. with this function you can retrieve the data about those defined gestures.
    /// @param p_LandscapeDataArray             array must be prescaled to the correct size before making this call/
    /// @param p_ArraySize                      size must be known before hand and is gotten via the landscape callback.
    /// @return 
    static EManusRet GetGestureLandscapeData(GestureLandscapeData* p_LandscapeDataArray, uint32_t p_ArraySize);

    /// @brief gets all current defined gestures. can be empty when just connecting. but once filled is not expected to change even rarely.
    static void GetGestures(std::vector<GestureLandscapeData>& p_ClientGestures);

    /// @brief given a skeleton id get the matching glove id (in case of bodies use p_left to determine which side)
    /// @param p_SkeletonId 
    /// @param p_Left 
    /// @return 
    static uint32_t GetGloveIdForSkeleton(uint32_t p_SkeletonId, bool p_Left);

    /// @brief given a skeleton id, check if the matching glove has haptics 
    /// @param p_SkeletonId 
    /// @param p_Left 
    /// @return 
    static bool DoesSkeletonHaveHaptics(int64 p_SkeletonId, bool p_Left);

    /// @brief check if a gesture is past a treshold for detection
    /// @param p_GestureId 
    /// @param p_Treshold range from 0.0 - 1.0
    /// @param p_GloveId
    static bool IsGesturePastTreshold(int64 p_GestureId, float p_Treshold, int64 p_GloveId);

    /// @brief get the rotation of a node from a glove for a given skeleton
    /// @param p_SkeletonId 
    /// @param p_NodeId
    /// @param p_Rotation if node found this will contain the rotation
    /// @return true if node was found
    static bool GetGloveRotationForSkeletonNode(int64 p_SkeletonId, int p_NodeId, ManusQuaternion& p_Rotation);

private:
	/// @brief Unreal method of handling DLL's. In this case the Manus SDK dll.
	struct CoreFunctionPointers;
	/// @brief Unreal method of handling DLL's. In this case the Manus SDK dll.
	static CoreFunctionPointers* s_FunctionPointers;
    /// @brief Unreal method of handling DLL's. In this case the Manus SDK dll.
	static void* s_DllHandle;

	/// @brief To handle different threads accessing data between each other, we need an unreal method of safeguarding this.
	static FCriticalSection* s_CriticalSectionSkeletons;
	/// @brief in the skeleton critical section we transfer data via an instance of the ClientSkeletonCollection
	static ClientSkeletonCollection* s_NextSkeletons; 

    /// @brief To handle different threads accessing data between each other, we need an unreal method of safeguarding this.
	static FCriticalSection* s_CriticalSectionErgonomics;
    /// @brief in the ergonomics critical section we transfer data via an instance of the ClientSkeletonCollection
	static ErgonomicsStream* s_ErgonomicsData;

    /// @brief To handle different threads accessing data between each other, we need an unreal method of safeguarding this.
	static FCriticalSection* s_CriticalSectionSystem; 

    /// @brief in the gesture critical section we transfer data 
    static std::vector<ClientGestures> s_GestureData;

    static std::vector<GestureLandscapeData> s_GestureLandscapeData;

    /// @brief To handle different threads accessing data between each other, we need an unreal method of safeguarding this.
    static FCriticalSection* s_CriticalSectionGesture;


    /// @brief in the landscape critical section we transfer data 
    static Landscape s_LandscapeData;

    /// @brief To handle different threads accessing data between each other, we need an unreal method of safeguarding this.
    static FCriticalSection* s_CriticalSectionLandscape;




	// no extra data just yet.. TBD
//	static FCriticalSection* s_CriticalSectionExtraData;
//	static ExtraDataStream* s_ExtraData;

	/// @brief when the Manus DLL makes an out of main thread call to report skeleton data updates, 
    /// it will transfer the data with a critical section to a local instance of the data
    /// this data can then be accessed via GetDataForSkeleton
	/// @param p_Skeletons 
	static void OnSkeletonsCallback(const SkeletonStreamInfo* const p_Skeletons);
    /// @brief when the Manus DLL makes an out of main thread call to report ergonomics updates, 
    /// it will transfer the data with a critical section to a local instance of the data
    /// this data can then be accessed via GetErgonomicsDataForGlove
	/// @param p_Ergonomics 
	static void OnErgonomicsCallback(const ErgonomicsStream* const p_Ergonomics);
    /// @brief when the Manus DLL signals that the connection was stopped it will call this function.
    /// nothing has been implemented here currently 
	/// @param p_Host 
	static void OnDisconnectedFromCore(const ManusHost* const p_Host);
    /// @brief when the Manus DLL passes a system message it can be caught here.
    /// it will transfer the data with a critical section to a local instance of the data
    /// this data can then be accessed via GetLastModifiedSkeletonIndex. Other messages are currently ignored
	/// @param p_SystemMessage 
	static void OnSystemCallback(const SystemMessage* const p_SystemMessage);

    /// @brief when Manus DLL passes a gesture probability, it can be caught here.
    /// it will transfer the data with a critical section to a local instance of the data
    /// this data can then be accessed via GetGestureProbabilities.
    /// @param p_GestureStream 
    static void OnGestureCallback(const GestureStreamInfo* const p_GestureStream);

    /// @brief when Manus DLL passes a landscape, it can be caught here.
    /// it will transfer part of the data with a critical section to a local instance of the data
    /// this data can then be accessed via GetGestureLandscapeData.
    /// @param p_Landscape 
    static void OnLandscapeCallback(const Landscape* const p_Landscape);

	//static void OnExtraDataCallback(const ExtraDataStream& p_ExtraData);
    
	/// @brief static value of how much time the SDK can spend to search for hosts
	static uint32_t s_SecondsToFindHosts;

	/// @brief system message can indicate which skeleton was last modified. this is used for automatically updating chains from a skeleton after it is reworked with manus_devtool
	static uint32_t s_LastModifiedSkeletonIndex;
};

// Close the Doxygen group.
/** @} */
