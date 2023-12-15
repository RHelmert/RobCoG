// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "Actors/SLStaticMeshAgentClass.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

// Sets default values
ASLStaticMeshAgentClass::ASLStaticMeshAgentClass()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = mesh;

}

// Called when the game starts or when spawned
void ASLStaticMeshAgentClass::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASLStaticMeshAgentClass::StaticClass(), FoundActors);

	UE_LOG(LogTemp, Log, TEXT("This is total number of found actors of type ASLStaticMeshAgentClass: %d"), FoundActors.Num());
}

// Called every frame
void ASLStaticMeshAgentClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}