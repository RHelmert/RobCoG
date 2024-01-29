// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SeperateActorManusComponent.h"
#include "ManusGestureDetector.generated.h"




UENUM(BlueprintType)
enum class GestureType : uint8
{
	ThumbsUp
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UVRHANDS_API UManusGestureDetector : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere)
	USeperateActorManusComponent* manusHand;

	TArray<FManusBone> boneData;
	bool isInit;

	// Sets default values for this component's properties
	UManusGestureDetector();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UFUNCTION(BlueprintCallable, Category = Manus)
	bool CheckForGesture(GestureType gType);

	UFUNCTION(BlueprintCallable, Category = Manus)
	TArray<FTransform> GetBoneData();

	UFUNCTION(BlueprintCallable, Category = Manus)
	bool CheckForThumbsup();

};
