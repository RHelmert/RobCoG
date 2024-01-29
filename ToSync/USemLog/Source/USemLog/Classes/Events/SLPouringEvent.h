// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Abijit Vyas

#pragma once

#include "ISLEvent.h"
#include <tuple> // for tuple

// Forward declarations
class USLBaseIndividual;

enum class USLPouringEventTypes
{PouredInto,
PouredOut
};
/**
* Pouring event class
*/
class FSLPouringEvent : public ISLEvent
{
public:
	// Default constructor
	FSLPouringEvent() = default;

	// Constructor with initialization
	FSLPouringEvent(const FString& InId, float InStart, float InEnd, uint64 InPairId,
		USLBaseIndividual* InIndividual1, USLBaseIndividual* InIndividual2, USLPouringEventTypes PouringEventType);

	// Constructor initialization without end time
	FSLPouringEvent(const FString& InId, const float InStart, const uint64 InPairId,
		USLBaseIndividual* InIndividual1, USLBaseIndividual* InIndividual2, USLPouringEventTypes PouringEventType);
	
	FString CreateAdditionalInfoForPouringEvent() const;

	// Pair id of the event (combination of two unique runtime ids)
	uint64 PairId;

	// Individual1 in Pouring
	USLBaseIndividual* Individual1;

	// Individual2 in Pouring
	USLBaseIndividual* Individual2;

	USLPouringEventTypes PouringEventTypes;

	int NumberOfParticles = 0;

	FString SourceContainerName;
	FString DestinationContainerName;
	

	TArray<FTransform> PouringPoseForSourceContainer;
	TArray<FTransform> PouringPoseForDestinationContainer;

	/* Begin IEvent interface */
	// Create an owl representation of the event
	virtual FSLOwlNode ToOwlNode() const override;

	// Add the owl representation of the event to the owl document
	virtual void AddToOwlDoc(FSLOwlDoc* OutDoc) override;
	
	// Send through ROSBridge
	virtual FString ToROSQuery() const override { return ""; };

	// Get event context data as string
	virtual FString Context() const override;

	// Get the tooltip data
	virtual FString Tooltip() const override;

	// Get the data as string
	virtual FString ToString() const override;

	// Get the event type name
	virtual FString TypeName() const override { return FString(TEXT("Pouring")); };
	/* End IEvent interface */

	// Create REST call to KnowRob
	virtual FString RESTCallToKnowRob(FSLKRRestClient* InFSLKRRestClient) const override;
};