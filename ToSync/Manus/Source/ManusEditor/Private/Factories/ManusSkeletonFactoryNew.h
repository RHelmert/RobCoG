// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2020 Manus

#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "ManusSkeletonFactoryNew.generated.h"



UCLASS(hidecategories = Object)
class UManusSkeletonFactoryNew : public UFactory
{
	GENERATED_UCLASS_BODY()

public:
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
};
