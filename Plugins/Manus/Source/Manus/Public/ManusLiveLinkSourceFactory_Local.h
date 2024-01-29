// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Set up a Doxygen group.
/** @addtogroup UManusLiveLinkSourceFactory_Local
 *  @{
 */

#include "LiveLinkSourceFactory.h"
#include "ManusLiveLinkSource.h"
#include "ManusLiveLinkSourceFactory_Local.generated.h"

UCLASS()
class UManusLiveLinkSourceFactory_Local : public ULiveLinkSourceFactory
{
	GENERATED_BODY()

public:
    /// @brief 
    /// @return "Manus Source"
	virtual FText GetSourceDisplayName() const override;
	/// @brief  
	/// @return "Manus Source"
	virtual FText GetSourceTooltip() const override;
    /// @brief get menu type
    /// @return 
	virtual EMenuType GetMenuType() const override;
    /// @brief 
    /// @param ConnectionString 
    /// @return replicated livelinksource
	virtual TSharedPtr<ILiveLinkSource> CreateSource(const FString& ConnectionString) const override;

};


// Close the Doxygen group.
/** @} */
