// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen
// In order to recognize custom blueprint class with Actor type, 
// we need to create custom Agent class which acts as parent class for blueprint objects. 
// See Pourig particles blueprint example.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLStaticMeshAgentClass.generated.h"

UCLASS(ClassGroup = (SL), DisplayName = "SL Static Mesh Actor")
class USEMLOG_API ASLStaticMeshAgentClass : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASLStaticMeshAgentClass();

	UPROPERTY(EditAnyWhere)
		UStaticMeshComponent* mesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};