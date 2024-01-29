// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Events/ISLEventHandler.h"
#include "Actors/SLCutterAgentClass.h"
#include "SLCuttingEvent.h"

/**
 * 
 */
class SLCuttingEventHandler : public ISLEventHandler
{

private:
	// Array of started Cutting events
	TArray<TSharedPtr<FSLCuttingEvent>> StartedCuttingEvents;

	class ASLCutterAgentClass* AgentToObserve = nullptr;

public:
	SLCuttingEventHandler();
	~SLCuttingEventHandler();

	// Geerbt über ISLEventHandler
	 void Init(UObject* InAgentToObserve) override;
	 void Start() override;
	 void Finish(float EndTime, bool bForced) override;
	 void FinishAllEvents(float EndTime);

	 void OnCuttingBegin(const FSLInteractionResult& cuttingInfo);
	 void OnCuttingEnd(USLBaseIndividual* Other, float Time, int32 result);


	 void AddNewSLCuttingEvent(const FSLInteractionResult& cuttingInfo);
	 bool FinishSLCuttingEvent(USLBaseIndividual* InOther, float EndTime, USLCuttingInfo result);
};
