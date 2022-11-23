// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#include "ManusSkeletonAssetActions.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ManusSkeleton.h"


#define LOCTEXT_NAMESPACE "AssetTypeActions"


bool FManusSkeletonAssetActions::CanFilter()
{
	return true;
}

uint32 FManusSkeletonAssetActions::GetCategories()
{
	return EAssetTypeCategories::Animation;
}


FText FManusSkeletonAssetActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ManusSkeleton", "Manus Skeleton");
}


UClass* FManusSkeletonAssetActions::GetSupportedClass() const
{
	return UManusSkeleton::StaticClass();
}


FColor FManusSkeletonAssetActions::GetTypeColor() const
{
	return FColor::White;
}


#undef LOCTEXT_NAMESPACE