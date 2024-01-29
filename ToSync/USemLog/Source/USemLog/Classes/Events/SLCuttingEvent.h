// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Abijit Vyas

#pragma once

#include "ISLEvent.h"
#include <tuple> // for tuple

// Forward declarations
class USLBaseIndividual;



enum class USLCuttingInfo : int32
{
	CuttingSucceed = 1,
	CuttingAborted = 2,
	NoInfo = 0
};


/**
* Cutting event class
*/
class FSLCuttingEvent : public ISLEvent
{
public:
	// Default constructor
	FSLCuttingEvent() = default;

	// Constructor with initialization
	FSLCuttingEvent(const FString& InId, float InStart, float InEnd, uint64 InPairId,
		USLBaseIndividual* InIndividual1, USLBaseIndividual* InIndividual2);

	// Constructor initialization without end time
	FSLCuttingEvent(const FString& InId, const float InStart, const uint64 InPairId,
		USLBaseIndividual* InIndividual1, USLBaseIndividual* InIndividual2);

	

	// Pair id of the event (combination of two unique runtime ids)
	uint64 PairId;

	// Individual1 in Cutting
	USLBaseIndividual* Individual1;

	// Individual2 in Cutting
	USLBaseIndividual* Individual2;

	USLCuttingInfo CuttingInformation = USLCuttingInfo::NoInfo;

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
	virtual FString TypeName() const override { return FString(TEXT("Cutting")); };
	/* End IEvent interface */

	// Create REST call to KnowRob
	virtual FString RESTCallToKnowRob(FSLKRRestClient* InFSLKRRestClient) const override;
};