// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#include "Events/SLCleaningEventHandler.h"
#include <Utils/SLUuid.h>

SLCleaningEventHandler::SLCleaningEventHandler()
{
}

SLCleaningEventHandler::~SLCleaningEventHandler()
{
}



void SLCleaningEventHandler::Init(UObject* InAgentToObserve)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		AgentToObserve = Cast<ASLCleanerAgent>(InAgentToObserve);
		if (AgentToObserve)
		{
			bIsInit = true;
		}

	}
}

void SLCleaningEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		
		AgentToObserve->CleaningStartedEvent.AddRaw(this, &SLCleaningEventHandler::OnCleaningBegin);
		AgentToObserve->CleaningEndedEvent.AddRaw(this, &SLCleaningEventHandler::OnCleaningEnd);

		bIsStarted = true;
		//UE_LOG(LogTemp, Log, TEXT("%s::%d Cleaning event handler has been started"), *FString(__FUNCTION__), __LINE__);
	}
	else if (bIsInit) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d Cleaning event handler could not be started correctly"), *FString(__FUNCTION__), __LINE__);
	}
}

void SLCleaningEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{

		FinishAllEvents(EndTime);


		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

void SLCleaningEventHandler::FinishAllEvents(float EndTime)
{
	// Finish cutting events
// Ignore all events not finished
	StartedCleaningEvents.Empty();
}

void SLCleaningEventHandler::OnCleaningBegin(const FSLInteractionResult& cuttingInfo)
{
	AddNewSLCleaningEvent(cuttingInfo);
}

void SLCleaningEventHandler::OnCleaningEnd(USLBaseIndividual* Other, float Time, int32 result)
{
	FinishSLCleaningEvent(Other, Time, static_cast<USLCleaningStatus>(result));
}

void SLCleaningEventHandler::AddNewSLCleaningEvent(const FSLInteractionResult& cuttingInfo)
{

	TSharedPtr<FSLCleaningEvent> Event = MakeShareable(new FSLCleaningEvent(
		FSLUuid::NewGuidInBase64Url(), cuttingInfo.Time,
		FSLUuid::PairEncodeCantor(cuttingInfo.Self->GetUniqueID(), cuttingInfo.Other->GetUniqueID()),
		cuttingInfo.Self, cuttingInfo.Other));
	Event->EpisodeId = EpisodeId;

	// Add event to the pending Cutting array
	StartedCleaningEvents.Emplace(Event);
}

bool SLCleaningEventHandler::FinishSLCleaningEvent(USLBaseIndividual* InOther, float EndTime, USLCleaningStatus result)
{
	// Use iterator to be able to remove the entry from the array
		// TIndexedContainerIterator<TArray<TSharedPtr<FSLCuttingEvent>>, TSharedPtr<FSLCuttingEvent>, int32> 
	for (auto EventItr(StartedCleaningEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching 
		if ((*EventItr)->Individual2 == InOther)
		{
			// Set the event end time
			(*EventItr)->EndTime = EndTime;

			// Set the result of the cutting. Is ist successfull or was it aborted? 
			// TODO: Where is it saved? 
			(*EventItr)->CuttingInformation = result;

			OnSemanticEvent.ExecuteIfBound(*EventItr);

			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}
