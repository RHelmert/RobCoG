// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Knowrob/SLKRRestClient.h"
#include "SLRestModuleAgentClass.generated.h"

UCLASS(ClassGroup = (SL), DisplayName = "SL Rest call module")
class USEMLOG_API ASLRestModuleAgentClass : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLRestModuleAgentClass();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FSLKRRestClient* fSLKRRestClient;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Save"))
	static bool SendRestCall();

	void SetSLKRRestClient(FSLKRRestClient* InFSLKRRestClient);

};
