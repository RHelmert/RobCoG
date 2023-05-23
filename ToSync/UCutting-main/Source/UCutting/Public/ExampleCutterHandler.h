// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CuttingHandlerComponent.h"
#include "ExampleCutterHandler.generated.h"

UCLASS()
class UCUTTING_API AExampleCutterHandler : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExampleCutterHandler();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROS")
    UCuttingHandlerComponent* CuttingHandler;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
