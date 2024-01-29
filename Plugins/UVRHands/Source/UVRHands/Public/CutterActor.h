// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "CutterActor.generated.h"



//To be callable by blueprints it has to be DynamicMulticast
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTestDelegate, float, input);



/**
 * Minimal example of an Event having 1 parameter
 */
UCLASS()
class UVRHANDS_API ACutterActor : public AStaticMeshActor
{
	GENERATED_BODY()
public:
	ACutterActor();

	//Let the Event be Assignable (connect Listeners), and be Callable by blueprints
	UPROPERTY(BlueprintAssignable,BlueprintCallable, Category = "Test")
		FTestDelegate OnTestDelegate;
	
	//Create a basic function
	UFUNCTION(Category = "EventTest")
	void EventTriggered(float val);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
