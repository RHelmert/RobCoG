// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Events/ISLEventHandler.h"

// Forward declarations
class USLBaseIndividual;
class FSLContactEvent;
class FSLSupportedByEvent;
struct FSLContactResult;

/**
 * Listens to Pouring events input, and outputs finished semantic Pouring events
 */
class FSLPouringEventHandler : public ISLEventHandler
{
public:
	// Init parent
	void Init(UObject* InParent) override;
	
	// Start listening to input
	void Start() override;

	// Terminate listener, finish and publish remaining events
	void Finish(float EndTime, bool bForced = false) override;

private:
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
	void OnSLPouringBegin(const FSLContactResult& InResult);
	
	// Event called when a semantic overlap event ends
	void OnSLPouringEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time);

private:

	// Array of started Pouring events
	TArray<TSharedPtr<FSLPouringEvent>> StartedPouringEvents;

	// Array of started supported by events
	TArray<TSharedPtr<FSLSupportedByEvent>> StartedSupportedByEvents;
	
	/* Constant values */
	constexpr static float PouringEventMin = 0.3f;
	constexpr static float SupportedByEventMin = 0.4f;
};
