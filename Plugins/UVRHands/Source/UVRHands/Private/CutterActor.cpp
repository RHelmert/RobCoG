// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "CutterActor.h"



ACutterActor::ACutterActor()
{

}


void ACutterActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Let the function listen to the Event
	OnTestDelegate.AddDynamic(this, &ACutterActor::EventTriggered);
	

}

void ACutterActor::EventTriggered(float val)
{
	//Write in Console 
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Sucessfully triggered Event CutterActor: %d"),
		*FString(__func__), __LINE__, val);

}
