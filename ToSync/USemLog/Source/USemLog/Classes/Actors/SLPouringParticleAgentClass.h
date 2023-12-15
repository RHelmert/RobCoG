// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen
// In order to recognize custom blueprint class with Actor type, 
// we need to create custom Agent class which acts as parent class for blueprint objects. See Pourig particles blueprint example.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLPouringParticleAgentClass.generated.h"

UCLASS(ClassGroup = (SL), DisplayName = "SL Pouring Particle Actor")
class USEMLOG_API ASLPouringParticleAgentClass : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASLPouringParticleAgentClass();

	UPROPERTY(EditAnyWhere)
		UStaticMeshComponent* mesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};