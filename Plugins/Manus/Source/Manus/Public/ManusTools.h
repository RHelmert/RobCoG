// Copyright 2015-2022 Manus

#pragma once

// Set up a Doxygen group.
/** @addtogroup ManusTools
 *  @{
 */

#include "ManusBlueprintTypes.h"
#include "Animation/AnimTypes.h"
#include "BoneContainer.h"
#include "ManusSkeleton.h"
#include "ManusSdkTypes.h"

/// @brief some support tools
struct MANUS_API ManusTools
{
	/// @brief Create a NodeSetup
	/// @param p_Id 
	/// @param p_ParentId 
	/// @param p_Transform 
	/// @param p_Name 
	/// @return 
	static NodeSetup CreateNodeSetup(uint32_t p_Id, uint32_t p_ParentId, FTransform p_Transform, FName p_Name);

    /// @brief converts unreal fstring to a UTF8 byte array in a char array.
    /// @param p_OutputArray 
    /// @param p_InputString 
    /// @param p_MaxSize 
    static void ConvertFStringToUTF8Array(char* p_OutputArray, FString p_InputString, int p_MaxSize);
};

// Close the Doxygen group.
/** @} */