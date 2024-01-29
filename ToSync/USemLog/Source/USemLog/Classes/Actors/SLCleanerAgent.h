// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include <Monitors/SLMonitorStructs.h>
#include "SLCleanerAgent.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FCleaningStarted, const FSLInteractionResult&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FCleaningEnded, USLBaseIndividual*, float, int32);


UCLASS(ClassGroup = (SL), DisplayName = "SL Cleaner Actor")
class USEMLOG_API ASLCleanerAgent : public AStaticMeshActor
{
	GENERATED_BODY()

		//---------------------------Variables----------------------------
private:

	UWorld* currentWorld;

	// True if initialized
	bool bIsInit = false;

	// True if started
	bool bIsStarted = false;

	// True if finished
	bool bIsFinished = false;

	// Semantic data component of the owner
	class USLIndividualComponent* OwnerIndividualComponent;

	// Object currently grasped
	class USLIndividualComponent* CurrCleanedIndividual;

public:

	//Let the Event be Assignable (connect Listeners), and be Callable by blueprints
	//UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Test")
	FCleaningStarted CleaningStartedEvent;
	FCleaningEnded CleaningEndedEvent;
	
public:
	ASLCleanerAgent();

	//ToDo fill with parameters needed
	UFUNCTION(BlueprintCallable)
		bool CleaningFinished(UPARAM(ref) AActor* other);

	UFUNCTION(BlueprintCallable)
		bool CleaningStarted(UPARAM(ref) AActor* other);

	UFUNCTION(BlueprintCallable)
		bool CleaningStopped(UPARAM(ref) AActor* other);
	
	
	
	
	
	bool Init();

	void Start();

	// Get init state
	bool IsInit() const { return bIsInit; };

	void Finish(bool bForced);

	void FinishActiveEvents();

	bool CheckIndividual(AActor* other, USLBaseIndividual*& result);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
