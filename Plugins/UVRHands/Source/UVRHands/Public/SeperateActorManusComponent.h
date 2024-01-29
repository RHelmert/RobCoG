// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ManusComponent.h"
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

	bool IsLocalPCControlled() const;
};
