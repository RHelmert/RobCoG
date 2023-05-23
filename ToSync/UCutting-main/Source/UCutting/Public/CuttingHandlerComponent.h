// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CuttingHandlerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UCUTTING_API UCuttingHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCuttingHandlerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    UWorld* World;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "ROS")
    FString Namespace;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void ConnectToHandler(const TSharedPtr<class FROSBridgeHandler>& ROSHandler);
};
