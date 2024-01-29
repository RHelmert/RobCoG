// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Events/ISLEventHandler.h"
#include "Actors/SLCleanerAgent.h"
#include "SLCleaningEvent.h"

/**
 * 
 */
class USEMLOG_API SLCleaningEventHandler : public ISLEventHandler
{

private:
	// Array of started Cutting events
	TArray<TSharedPtr<FSLCleaningEvent>> StartedCleaningEvents;

	class ASLCleanerAgent* AgentToObserve = nullptr;


public:
	SLCleaningEventHandler();
	~SLCleaningEventHandler();

	// Geerbt über ISLEventHandler
	void Init(UObject* InAgentToObserve) override;
	void Start() override;
	void Finish(float EndTime, bool bForced) override;
	void FinishAllEvents(float EndTime);

	void OnCleaningBegin(const FSLInteractionResult& cuttingInfo);
	void OnCleaningEnd(USLBaseIndividual* Other, float Time, int32 result);

	void AddNewSLCleaningEvent(const FSLInteractionResult& cuttingInfo);
	bool FinishSLCleaningEvent(USLBaseIndividual* InOther, float EndTime, USLCleaningStatus result);
};
