// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "Actors/SLPouringParticleAgentClass.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

// Sets default values
ASLPouringParticleAgentClass::ASLPouringParticleAgentClass()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = mesh;

}

// Called when the game starts or when spawned
void ASLPouringParticleAgentClass::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASLPouringParticleAgentClass::StaticClass(), FoundActors);

	UE_LOG(LogTemp, Log, TEXT("This is total number of found actors of type ASLPouringParticleAgentClass: %d"), FoundActors.Num());

	// TODO0: Modify the SLParticleIndividual such that it can log its info to the semanticmap.owl file: DONE
	// TODO0: Work on toggle masks option as well so that each individual particle can have unique color
	// TODO0:create custom collision channel for particles such that they do not collide with each other
	// TODO0: Work on creating new maps with different source and dest container and then put all particles inside source container(make pouring scenario ready)
	// TODO0: Work on different shapes collisions in Blender. https://www.youtube.com/watch?v=q1iF08P8hC0&ab_channel=C%3A%5CInsertNameHere
	// TODO1: get all particles with given class ASLPouringParticleAgentClass
	// TODO2: for each particle, check if it collides with the invisible surface of the container, ignore its own collision with other particles
	// TODO3: if it leaves the container that means it is now in pouring mode
	// TODO4: log its location every second(10 times) into tf file
	// TODO5: 

}

// Called every frame
void ASLPouringParticleAgentClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}