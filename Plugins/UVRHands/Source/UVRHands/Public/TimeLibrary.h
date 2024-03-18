// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TimeLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UVRHANDS_API UTimeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	static int64 ROSStartTime;
	
};
