// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SLCutter.generated.h"




//To be callable by blueprints it has to be DynamicMulticast
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCuttingSucceeded, float, input);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCuttingStarted, float, input);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCuttingAborted, float, input);

//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FReserve);


/**
 * 
 */
UCLASS()
class UVRHANDS_API ASLCutter : public AStaticMeshActor
{
	GENERATED_BODY()

private: 
	bool isCutting;

public:
	ASLCutter();

	//Let the Event be Assignable (connect Listeners), and be Callable by blueprints
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Test")
		FCuttingSucceeded CuttingSucceededEvent;

	//Create a basic function
	UFUNCTION(Category = "EventTest")
		void ObjectCut(float val);


	//ToDo fill with parameters needed
	UFUNCTION(BlueprintCallable)
		void CuttingSucceeded();

	UFUNCTION(BlueprintCallable)
		void CuttingIntented();

	UFUNCTION(BlueprintCallable)
		void CuttingStarted();

	UFUNCTION(BlueprintCallable)
		void CuttingAborted();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
