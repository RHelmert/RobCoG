// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ManusComponent.h"
#include "ManusBlueprintTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "SeperateActorManusComponent.generated.h"






/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class UVRHANDS_API USeperateActorManusComponent : public UManusComponent
{
	GENERATED_BODY()
	
public:
	USeperateActorManusComponent(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaSeconds, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool IsLocalPCControlled() const;

private:
	/** Called every frame to update the finger haptics. */
	virtual void TickFingerHaptics() override;

	/** Called every frame to update the gesture detection. */
	virtual void TickGestureDetection() override;

	/** Called when a bone got overlapped or hit to add vibration to its corresponding finger haptics. */
	virtual void AddFingerVibration(FName BoneName) override;

};
