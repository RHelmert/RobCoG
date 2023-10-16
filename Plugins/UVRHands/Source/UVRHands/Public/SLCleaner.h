// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SLCleaner.generated.h"




DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCleaningFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCleaningStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCleaningStopped);



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
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "USemLog_C++Event")
		FCleaningStarted CleaningStartedEvent;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "USemLog_C++Event")
		FCleaningStopped CleaningStoppedEvent;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "USemLog_C++Event")
		FCleaningFinished CleaningFinishedEvent;



	UFUNCTION(BlueprintCallable)
		void CleaningStarted();

	UFUNCTION(BlueprintCallable)
		void CleaningStopped();

	UFUNCTION(BlueprintCallable)
		void CleaningFinished();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
