// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Set up a Doxygen group.
/** @addtogroup UManusLiveLinkSourceFactory_Local
 *  @{
 */

#include "LiveLinkSourceFactory.h"
#include "ManusLiveLinkSource.h"
#include "ManusLiveLinkSourceFactory_Replicated.generated.h"

UCLASS()
class UManusLiveLinkSourceFactory_Replicated : public ULiveLinkSourceFactory
{
	GENERATED_BODY()

public:
	/// @brief returns "Manus Replicated Source"
	virtual FText GetSourceDisplayName() const override;
	/// @brief 
	/// @return "Manus Replicated Source"
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
