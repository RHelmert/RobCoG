// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#pragma once

#include "AssetTypeActions_Base.h"
#include "Templates/SharedPointer.h"

class ISlateStyle;


/**
 * Implements an action for UTextAsset assets.
 */
class FManusSkeletonAssetActions : public FAssetTypeActions_Base
{
public:
	FManusSkeletonAssetActions() {}

public:
	virtual bool CanFilter() override;
	virtual uint32 GetCategories() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
};