// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "Classes/Actors/SLRestModuleAgentClass.h"

// Sets default values
ASLRestModuleAgentClass::ASLRestModuleAgentClass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASLRestModuleAgentClass::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASLRestModuleAgentClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASLRestModuleAgentClass::SetSLKRRestClient(FSLKRRestClient* InFSLKRRestClient)
{
	fSLKRRestClient = InFSLKRRestClient;
}

bool ASLRestModuleAgentClass::SendRestCall()
{
	//fSLKRRestClient = InFSLKRRestClient;
	// TODO: send create episode call to knowrob
	
	return true;
}