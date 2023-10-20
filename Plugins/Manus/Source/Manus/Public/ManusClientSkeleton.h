#pragma once

// Set up a Doxygen group.
/** @addtogroup ClientSkeleton
 *  @{
 */

#include "ManusSdkTypes.h"
#include <vector>

/// @brief This support class helps with converting and storing the skeleton data from %Manus Core animations before they get converted to UManusSkeleton
class ClientSkeleton
{
public:
	/// @brief skeleton meta data 
	SkeletonInfo info;
	/// @brief an array of the nodes (transform and ID )
	SkeletonNode* nodes = nullptr;

	ClientSkeleton() {};

    /// @brief destroys clientskeleton and also cleans the array if it is still filled
	~ClientSkeleton()
	{
		if (nodes != nullptr)delete[] nodes;
	}
};

/// @brief An animation update will contain updates for multiple skeletons which need to be temporarily stored and moved between threads
class ClientSkeletonCollection
{
public:
	/// @brief a list of skeletons
	std::vector<ClientSkeleton> skeletons;
	/// @brief empty constructor
	ClientSkeletonCollection();
	/// @brief copy constructor
	/// @param p_Original 
	ClientSkeletonCollection(const ClientSkeletonCollection& p_Original);
	/// @brief copy skeleton collection over 'this' skeletons list. it will destroy old instances of client skeletons.
	/// @param p_Original 
	void CopyFrom(const ClientSkeletonCollection& p_Original);
	/// @brief copy individual skeleton indicated with p_SkeletonId from the list of skeletons into the p_Data instance of a skeleton.
	/// @param p_SkeletonId id of the skeleton we want to copy.
	/// @param p_Data pre-allocated instance of a ClientSkeleton to put the data into.
	/// @return true upon success.
	bool CopySkeleton(uint32_t p_SkeletonId, ClientSkeleton* p_Data);
	
};


// Close the Doxygen group.
/** @} */
