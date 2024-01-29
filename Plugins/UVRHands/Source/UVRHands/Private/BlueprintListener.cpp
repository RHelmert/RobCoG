// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "BlueprintListener.h"

// Sets default values for this component's properties
UBlueprintListener::UBlueprintListener()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
//	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UBlueprintListener::BeginPlay()
{
	Super::BeginPlay();
	
	// ...
	
}


// Called every frame
void UBlueprintListener::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UBlueprintListener::EventTriggered()
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d Sucessfully triggered Event"),
		*FString(__func__), __LINE__);

}

