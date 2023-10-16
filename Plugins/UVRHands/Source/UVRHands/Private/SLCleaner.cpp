// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "SLCleaner.h"

ASLCleaner::ASLCleaner()
{
}

void ASLCleaner::BeginPlay()
{
	Super::BeginPlay();
}

void ASLCleaner::CleaningStarted()
{
	CleaningStartedEvent.Broadcast();
}

void ASLCleaner::CleaningStopped()
{
	CleaningStoppedEvent.Broadcast();
}

void ASLCleaner::CleaningFinished() 
{
	CleaningFinishedEvent.Broadcast();
}




