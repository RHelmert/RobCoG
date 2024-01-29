// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SLEventCreator.generated.h"

/**
 * 
 */
UCLASS()
class UVRHANDS_API ASLEventCreator : public AStaticMeshActor
{
	GENERATED_BODY()
	

public:

	UFUNCTION(BlueprintCallable)
		void TestMethod(int input);
};
