// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SLCleanable.generated.h"

/**
 * 
 */
UCLASS()
class UVRHANDS_API ASLCleanable : public AStaticMeshActor
{
	GENERATED_BODY()
public:
	ASLCleanable();

	UFUNCTION(BlueprintCallable)
		void CleaningStarted();

	UFUNCTION(BlueprintCallable)
		void CleaningStopped();



protected:
	// Called when the game starts
	virtual void BeginPlay() override;

};
