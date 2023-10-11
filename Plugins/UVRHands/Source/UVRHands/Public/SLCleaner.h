// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SLCleaner.generated.h"




DECLARE_DYNAMIC_MULTICAST_DELEGATE(FReserve);



/**
 * 
 */
UCLASS()
class UVRHANDS_API ASLCleaner : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	ASLCleaner();

	//Let the Event be Assignable (connect Listeners), and be Callable by blueprints
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Test")
		FReserve CleaningStartedEvent;



	UFUNCTION(BlueprintCallable)
		void CleaningStarted();

	UFUNCTION(BlueprintCallable)
		void CleaningStopped();


};
