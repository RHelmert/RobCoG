// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/ActorComponent.h"
#include "StrenghAdjustment.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UStrenghAdjustment : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStrenghAdjustment();

	UFUNCTION(BlueprintCallable, Category = "My Nodes")
	void Init();

	UFUNCTION(BlueprintCallable, Category = "My Nodes")
	void AddConstraint(UPhysicsConstraintComponent* constraint, USceneComponent* parent, USceneComponent* child);

	UFUNCTION(BlueprintCallable, Category = "My Nodes")
	void SetFlattenDistance(float d);

	UFUNCTION(BlueprintCallable, Category = "My Nodes")
	void SetSpringForce(float d);

	UFUNCTION(BlueprintCallable, Category = "My Nodes")
	void SetDampeningTarget(float d);

	



	UFUNCTION(BlueprintCallable, Category = "My Nodes")
	virtual void TestChangeStrength(float Strength, float Dampening);



protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	

	TArray<UPhysicsConstraintComponent*> SphereConstrains;

	TArray<USceneComponent*> Parents;

	TArray<USceneComponent*> Childs;

	float triggerdistance;
	float springforce;
	float dampening;


	UPROPERTY(EditAnywhere)
		float MaxDistance;

	float DefaultForce;
};
