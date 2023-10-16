// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SLCuttable.generated.h"

/**
 * 
 */
UCLASS()
class UVRHANDS_API ASLCuttable : public AStaticMeshActor
{
	GENERATED_BODY()
public:
	ASLCuttable();

	UFUNCTION(BlueprintCallable)
		void ObjectCut();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
