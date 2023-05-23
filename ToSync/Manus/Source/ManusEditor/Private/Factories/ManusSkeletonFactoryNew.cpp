// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#include "ManusSkeletonFactoryNew.h"
#include "ManusSkeleton.h"


UManusSkeletonFactoryNew::UManusSkeletonFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UManusSkeleton::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UManusSkeletonFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UManusSkeleton>(InParent, InClass, InName, Flags);
}


bool UManusSkeletonFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
