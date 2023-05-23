// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent.h"
#include "CustomMeshComponent.h"
#include "Cuttable.generated.h"

UCLASS()
class UCUTTING_API ACuttable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACuttable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform & Transform) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


    //Mesh that will become slicable
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* StaticMesh;
    //Mesh that will be cut
    UPROPERTY(VisibleAnywhere)
    UProceduralMeshComponent* ProMesh;
	
};
