// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen
// In order to recognize custom blueprint class with Actor type, 
// we need to create custom Agent class which acts as parent class for blueprint objects. See Pourig particles blueprint example.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include <Monitors/SLMonitorStructs.h>
#include "SLCutterAgentClass.generated.h"



//To be callable by blueprints it has to be DynamicMulticast
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCuttingSucceeded, FSLInteractionResult*, result);

DECLARE_MULTICAST_DELEGATE_OneParam(FCuttingSucceeded, const FSLInteractionResult&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FCuttingEnded, USLBaseIndividual*,float, int32);




// This class is used to inherit from when building knives and other cutter. Call these methods from BP or anywhere else at the right time
// and the rest ist managed automatically

UCLASS(ClassGroup = (SL), DisplayName = "SL Cutter Actor")
class USEMLOG_API ASLCutterAgentClass : public AStaticMeshActor
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
	class USLIndividualComponent* CurrCutIndividual;



public:

	//Let the Event be Assignable (connect Listeners), and be Callable by blueprints
	//UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Test")
		FCuttingSucceeded CuttingStartedEvent;
		FCuttingEnded CuttingEndedEvent;



	bool isCutting;



	//---------------------------Methods----------------------------
public:

	ASLCutterAgentClass();
	~ASLCutterAgentClass();


	//Create a basic function
	UFUNCTION(Category = "EventTest")
		void ObjectCut(float val);


	//ToDo fill with parameters needed
	UFUNCTION(BlueprintCallable)
		bool CuttingSucceeded(UPARAM(ref) AActor* other);

	UFUNCTION(BlueprintCallable)
		bool CuttingStarted(UPARAM(ref) AActor* other);

	UFUNCTION(BlueprintCallable)
		bool CuttingAborted(UPARAM(ref) AActor* other);

	bool Init();

	void Start();

	// Get init state
	bool IsInit() const { return bIsInit; };

	void Finish(bool bForced);

	void FinishActiveEvents();

	bool CheckIndividual(AActor* other, USLBaseIndividual*& result);

//private:
	//void OnSLGraspBegin(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time, const FString& GraspType);

	//void OnSLGraspEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

};