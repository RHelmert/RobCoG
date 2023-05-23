// Fill out your copyright notice in the Description page of Project Settings.


#include "ExampleCutterHandler.h"


// Sets default values
AExampleCutterHandler::AExampleCutterHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    CuttingHandler= CreateDefaultSubobject<UCuttingHandlerComponent>(TEXT("CuttingHandler"));

}

// Called when the game starts or when spawned
void AExampleCutterHandler::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AExampleCutterHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

