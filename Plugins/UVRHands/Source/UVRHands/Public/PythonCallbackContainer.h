// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PythonCallbackContainer.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTaskCompletedEventBP);

UCLASS(Blueprintable, BlueprintType)
class UVRHANDS_API UPythonCallbackContainer : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FTaskCompletedEventBP OnTaskCompletedEvent;
	
};
