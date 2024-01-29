// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLPouringEventHandler.h"
#include "Events/SLPouringEvent.h"
#include "Events/SLSupportedByEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Utils/SLUuid.h"
#include "Actors/SLPouringParticleAgentClass.h"
#include "Monitors/SLContactMonitorBox.h"
#include "Engine/World.h"

// Set parent
void FSLPouringEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		/*Parent = Cast<USLContactMonitorBox>(InParent);
		if (Parent)
		{
			bIsInit = true;
		}*/
	}
}

// Bind to input delegates
void FSLPouringEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		/*if (Parent)
		{
			Parent->OnPouringBegin.AddRaw(this, &FSLPouringEventHandler::OnSLPouringBegin);
			Parent->OnPouringEnd.AddRaw(this, &FSLPouringEventHandler::OnSLPouringEnd);
		}*/

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLPouringEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Let parent first publish any pending (delayed) events
		/*if (!Parent->IsFinished())
		{
			Parent->Finish(EndTime);
		}*/
		
		// End and broadcast all started events
		FinishAllEvents(EndTime);

		// TODO use dynamic delegates to be able to unbind from them
		// https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Delegates/Dynamic
		// this would mean that the handler will need to inherit from UObject

		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Start new Pouring event
void FSLPouringEventHandler::AddNewPouringEvent(const FSLContactResult& InResult)
{
	// Start a semantic Pouring event
	if (InResult.Other->GetClass() == ASLPouringParticleAgentClass::StaticClass()) {
		// if there is already CurrPouredIndividual object then pouring is initiated
			TSharedPtr<FSLPouringEvent> Event = MakeShareable(new FSLPouringEvent(
				FSLUuid::NewGuidInBase64Url(), InResult.Time,
				FSLUuid::PairEncodeCantor(InResult.Self->GetUniqueID(), InResult.Other->GetUniqueID()),
				InResult.Self, InResult.Other, USLPouringEventTypes::PouredOut));
			Event->EpisodeId = EpisodeId;
			// Add event to the pending Pourings array
			StartedPouringEvents.Emplace(Event);
	}
	
}

// Publish finished event
bool FSLPouringEventHandler::FinishPouringEvent(USLBaseIndividual* InOther, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedPouringEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Individual2 == InOther)
		{
			// Set the event end time
			(*EventItr)->EndTime = EndTime;

			// Avoid publishing short events
			if (((*EventItr)->EndTime - (*EventItr)->StartTime) > PouringEventMin)
			{
				OnSemanticEvent.ExecuteIfBound(*EventItr);
			}
			
			// Remove event from the pending list
			EventItr.RemoveCurrent();

			return true;
		}
	}
	return false;
}
 

// Terminate and publish pending Pouring events (this usually is called at end play)
void FSLPouringEventHandler::FinishAllEvents(float EndTime)
{
	// Finish Pouring events
	for (auto& Ev : StartedPouringEvents)
	{
		// Ignore short events
		if (EndTime - Ev->StartTime > PouringEventMin)
		{
			// Set end time and publish event
			Ev->EndTime = EndTime;
			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}
	StartedPouringEvents.Empty();

}

// Event called when a semantic overlap event begins
void FSLPouringEventHandler::OnSLPouringBegin(const FSLContactResult& InResult)
{
	AddNewPouringEvent(InResult);
}

// Event called when a semantic overlap event ends
void FSLPouringEventHandler::OnSLPouringEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time)
{
	FinishPouringEvent(Other, Time);
}

