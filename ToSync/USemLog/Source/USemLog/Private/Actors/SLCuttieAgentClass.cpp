// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "Actors/SLCuttieAgentClass.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

// Sets default values
ASLCuttieAgentClass::ASLCuttieAgentClass()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = mesh;

}

// Called when the game starts or when spawned
void ASLCuttieAgentClass::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASLCuttieAgentClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}