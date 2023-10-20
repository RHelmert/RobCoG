// Copyright 2015-2022 Manus

#pragma once

// Set up a Doxygen group.
/** @addtogroup FManusLiveLinkUser
 *  @{
 */

/// @brief %Manus Live Link Users are the associations of a %Manus Dashboard User and a %Manus Skeleton, and their data.
struct MANUS_API FManusLiveLinkUser
{
    /// @brief The index of the User as displayed in the %Manus Dashboard (first User is index 0).	
	int ManusDashboardUserIndex = 0; 

    /// @brief The %Manus skeleton to use. 
	class UManusSkeleton* ManusSkeleton = NULL;

    /// @brief FLiveLinkFrameDataStruct pointer.
	FLiveLinkFrameDataStruct* LiveLinkFrame = NULL;

    /// @brief The objects currently using this %Manus Live Link User data. 
	TArray<TWeakObjectPtr<UObject>> ObjectsUsingUser;
    /// @brief Whether we should update the Live Link data or not. 
	bool bShouldUpdateLiveLinkData = false;

    /// @brief %Manus data last update time. 
	uint64 ManusDataLastUpdateTime = 0;

    /// @brief Skeletons initialization retry frames countdown. 
	int SkeletonsInitializationRetryCountdown = 0; 

public:
    
};

// Close the Doxygen group.
/** @} */
