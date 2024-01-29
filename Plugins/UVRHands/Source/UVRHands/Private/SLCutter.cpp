#include "SLCutter.h"
// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "SLCutter.h"



ASLCutter::ASLCutter()
{

}


void ASLCutter::BeginPlay()
{
	Super::BeginPlay();

	// Let the function listen to the Event
	CuttingSucceededEvent.AddDynamic(this, &ASLCutter::ObjectCut);

}

void ASLCutter::ObjectCut(float val)
{
	//Write in Console 
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Sucessfully triggered Event CutterActor: %d"),
		*FString(__func__), __LINE__, val);

}

void ASLCutter::CuttingStarted() {

}
void ASLCutter::CuttingAborted() {

}
void ASLCutter::CuttingSucceeded() {
	CuttingSucceededEvent.Broadcast(1);
}

void ASLCutter::CuttingIntented() {

}

