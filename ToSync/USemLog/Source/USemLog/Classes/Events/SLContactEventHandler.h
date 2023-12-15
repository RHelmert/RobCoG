// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once
#include <tuple> // for tuple
#include <string>
//#include "../../UVRHands/Source/UVRHands/Public/SLCutter.h"
#include "Actors/SLCutterAgentClass.h"
#include "Engine/StaticMeshActor.h"
#include "Events/ISLEventHandler.h"



// Forward declarations
class USLBaseIndividual;
class FSLContactEvent;
class FSLSupportedByEvent;
struct FSLContactResult;
class FSLPouringEvent;
class FSLCuttingEvent;
 

/**
 * Listens to contact events input, and outputs finished semantic contact events
 */
class FSLContactEventHandler : public ISLEventHandler
{
public:
	// Init parent
	void Init(UObject* InParent) override;
	
	// Start listening to input
	void Start() override;

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime, bool bForced = false) override;

private:
	// Start new contact event
	void AddNewContactEvent(const FSLContactResult& InResult);

	// Finish then publish the event
	bool FinishContactEvent(USLBaseIndividual* InOther, float EndTime);

	// Start new Pouring event
	void AddNewPouringEvent(const FSLContactResult& InResult);


	// Finish then publish the event
	bool FinishPouringEvent(USLBaseIndividual* InOther, float EndTime);

	// Start new supported by event
	void AddNewSupportedByEvent(USLBaseIndividual* Supported, USLBaseIndividual* Supporting, float StartTime, const uint64 EventPairId);

	// Finish then publish the event
	bool FinishSupportedByEvent(const uint64 InPairId, float EndTime);

	// Terminate and publish started events (this usually is called at end play)
	void FinishAllEvents(float EndTime);

	// Event called when a semantic overlap event begins
	void OnSLOverlapBegin(const FSLContactResult& InResult);
	
	// Event called when a semantic overlap event ends
	void OnSLOverlapEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time);

	// Event called when a supported by event begins
	void OnSLSupportedByBegin(USLBaseIndividual* Supported, USLBaseIndividual* Supporting, float StartTime, const uint64 EventPairId);
	
	// Event called when a supported by event ends
	void OnSLSupportedByEnd(const uint64 PairId1, const uint64 PairId2, float Time);

	// Event called when a semantic overlap begins
	void OnSLPouringBegin(const FSLContactResult& InResult);

	// Event called when a semantic overlap event ends
	void OnSLPouringEnds(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time);
	
private:
	// Parent semantic overlap area
	class ISLContactMonitorInterface* Parent = nullptr;

	// Array of started contact events
	TArray<TSharedPtr<FSLContactEvent>> StartedContactEvents;

	// Array of started supported by events
	TArray<TSharedPtr<FSLSupportedByEvent>> StartedSupportedByEvents;

	// Array of started Pouring events
	TArray<TSharedPtr<FSLPouringEvent>> StartedPouringEvents;

	bool IsPouringEventCurrentlyRunning(TArray<std::tuple<FString, float>> Containers, const FSLContactResult& InResult);
	
	/* Constant values */
	constexpr static float ContactEventMin = 0.01f;
	constexpr static float SupportedByEventMin = 0.4f;
	constexpr static float PouringEventMin = 0.03f;
	constexpr static float CuttingEventMin = 0.03f;
	int particlesOverlapEnded = 0;
	TArray<FTransform> PouringPoseForSourceContainer;
	TArray<FTransform> PouringPoseForDestinationContainer;
	TArray <FString> SourceContainers;
	TArray <FString> DestinationContainers;
	FString SourceContainerName;
	FString DestinationContainerName;
	
	TArray<std::tuple<FString, float>> SourceContainersList;
	
	TArray<std::tuple<FSLContactResult, FSLContactResult>> ListOfContainersCombinations;

	TArray<std::tuple<FString, float>> DestinationContainersList;
	float MaxPouringEventTime = 5;
	TSharedPtr<FSLPouringEvent> CurrentPouringEvent;

	
};
