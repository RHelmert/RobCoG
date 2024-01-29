// Copyright 2015-2022 Manus

#pragma once

// Set up a Doxygen group.
/** @addtogroup ManusConvert
 *  @{
 */

#include "ManusBlueprintTypes.h"
#include "ManusSDK.h"

struct MANUS_API ManusConvert
{
	/// @brief converts handtype from Manus SDK version to Unreal version
	/// @param Input 
	/// @param Output 
	/// @return succes when it succeeds
	static EManusRet SdkHandTypeToBp(Side Input, EManusHandType &Output);
	/// @brief converts handtype from Unreal version to Manus SDK version
	/// @param Input 
	/// @param Output 
    /// @return succes when it succeeds
	static EManusRet BpHandTypeToSdk(EManusHandType Input, Side &Output);

	/// @brief converts TrackerType from Manus SDK version to unreal version
	/// @param Input 
	/// @param Output 
    /// @return succes when it succeeds
	static EManusRet SdkTrackerTypeToBpHandType(uint32_t Input, EManusHandType& Output);
	/// @brief converts TrackerType from Unreal version to Manus SDK version
	/// @param Input 
	/// @param Output 
    /// @return succes when it succeeds
	static EManusRet BpHandTypeToSdkTrackerType(EManusHandType Input, uint32_t& Output);
	/// @brief converts DeviceType from Manus SDK version to unreal version
	/// @param p_Input 
	/// @param p_Output 
    /// @return succes when it succeeds
	static EManusRet SdkDeviceTypeToBp(DeviceFamilyType p_Input, EManusGloveType& p_Output);
       
	/// @brief Convert NodeSetup from unreal to %Manus Core version
	/// @param p_OutputNode 
	/// @param p_InputNode 
	static void BpManusNodeSetupToSDK(NodeSetup& p_OutputNode, const FManusNodeSetup& p_InputNode);
	/// @brief Convert Nodesetup from %Manus Core to unreal version
	/// @param p_OutputNode 
	/// @param p_InputNode 
	static void SDKManusNodeSetupToBP(FManusNodeSetup& p_OutputNode, const NodeSetup& p_InputNode);
	/// @brief Convert chainSetup from unreal to %Manus Core version
	/// @param p_OutputChain 
	/// @param p_InputChain 
	/// @param p_Overridetrackers this is a demo only function for quick overriding a chain setting to support trackers withotu permanently adjusting manus skeleton
	static void BpManusChainSetupToSDK(ChainSetup& p_OutputChain, const FManusChainSetup& p_InputChain, bool p_Overridetrackers = false);
	/// @brief Convert Chainsetup from %Manus Core to unreal version
	/// @param p_OutputChain 
	/// @param p_InputChain 
	static void SDKManusChainSetupToBp(FManusChainSetup& p_OutputChain, const ChainSetup& p_InputChain);
	/// @brief converts %Manus Core version of side to unreal version.
	/// @param p_Input 
	/// @return succes when it succeedsReturns %Manus Core version of side.
	static Side BpSideToSdk(EManusSide p_Input);
	/// @brief converts Unreal version of side to %Manus Core version.
	/// @param p_Input 
	/// @return Returns Unreal version of side.
	static EManusSide SDKSideToBp(Side p_Input);
	/// @brief Convert %Manus Core handversion to unreal version
	/// @param p_Input 
	/// @return returns unreal version of handversion
	static EManusHandMotion SDKHandMotionToBp(const HandMotion& p_Input);
	/// @brief Convert %Manus Core version of handmation to Unreal version
	/// @param p_Input 
	/// @return returns unreal version of handmotion 
	static HandMotion BpHandMotionToSDK(const EManusHandMotion& p_Input);
	/// @brief Converts unreal vector to %Manus Core version
	/// @param p_Input 
	/// @param p_Output 
	static void BpVectorToSDK(const FVector& p_Input, ManusVec3& p_Output);
	/// @brief converts %Manus Core version of vector to unreal version.
	/// @param p_Input 
	/// @param p_Output 
	static void SDKVectorToBp(const ManusVec3& p_Input, FVector& p_Output);
	/// @brief Convert unreal chain settings to %Manus Core version
	/// @param p_Input 
	/// @param p_Output 
	/// @param p_Overridetrackers this is a demo only function for quick overriding a chain setting to support trackers without permanently adjusting manus skeleton
	static void BpChainSettingsToSDK(const FManusChainSettings& p_Input, ChainSettings& p_Output, bool p_Overridetrackers = false);
    /// @brief Converts %Manus Core chain setting to unreal version.
    /// @param p_Input 
    /// @param p_Output 
    static void SDKChainSettingsToBp(const ChainSettings& p_Input, FManusChainSettings& p_Output);
	/// @brief Converts Unreal Chaintype to %Manus Core version
	/// @param p_Input 
	/// @return returns %Manus Core version of ChainType
	static ChainType ChainTypeToSDK(const EManusChainType p_Input);
	/// @brief Converts %Manus Core Chaintype to Unreal version
	/// @param p_Input 
	/// @return returns unreal version of ChainType
	static EManusChainType ChainTypeToBp(const ChainType p_Input);
};

// Close the Doxygen group.
/** @} */
