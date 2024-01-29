// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLContactEventHandler.h"
#include "Monitors/SLContactMonitorInterface.h"
#include "Events/SLContactEvent.h"
#include "Events/SLSupportedByEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Utils/SLUuid.h"
#include "Individuals/Type/SLParticleIndividual.h"
#include "Individuals/Type/SLCutterIndividual.h"
#include "Individuals/Type/SLCuttieIndividual.h"
#include "Events/SLPouringEvent.h"
#include "Events/SLCuttingEvent.h"
#include "Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"




// Set parent
void FSLContactEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		Parent = Cast<ISLContactMonitorInterface>(InParent);
		if (Parent)
		{
			bIsInit = true;
		}

		
	}
}

// Bind to input delegates
void FSLContactEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if (Parent)
		{
			Parent->OnBeginSLContact.AddRaw(this, &FSLContactEventHandler::OnSLOverlapBegin);
			Parent->OnEndSLContact.AddRaw(this, &FSLContactEventHandler::OnSLOverlapEnd);

			Parent->OnBeginSLSupportedBy.AddRaw(this, &FSLContactEventHandler::OnSLSupportedByBegin);
			Parent->OnEndSLSupportedBy.AddRaw(this, &FSLContactEventHandler::OnSLSupportedByEnd);

			// TODO: Use pouring event start and end here. This is for some reason not working when the event is detected.
			Parent->OnPouringBegin.AddRaw(this, &FSLContactEventHandler::OnSLPouringBegin);
			Parent->OnPouringEnds.AddRaw(this, &FSLContactEventHandler::OnSLPouringEnds);
		}

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLContactEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Let parent first publish any pending (delayed) events
		if(!Parent->IsFinished())
		{
			Parent->Finish();
		}
		
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

// Start new contact event
void FSLContactEventHandler::AddNewContactEvent(const FSLContactResult& InResult)
{
	// TODO: this code is temporary solution to increase the fps for simulation case when 100s of particles collide with objects
		// Start a semantic contact event
	TSharedPtr<FSLContactEvent> Event = MakeShareable(new FSLContactEvent(
		FSLUuid::NewGuidInBase64Url(), InResult.Time,
		FSLUuid::PairEncodeCantor(InResult.Self->GetUniqueID(), InResult.Other->GetUniqueID()),
		InResult.Self, InResult.Other));
	Event->EpisodeId = EpisodeId;
	// Add event to the pending contacts array
	StartedContactEvents.Emplace(Event);

	// Start a semantic Pouring event, if particles are involved in contact
	// TODO: Replace this with the Parent->OnPouringBegin.AddRaw
	// TODO: add new spilling event as well when particles touch the table or other defined surfaces
	AddNewPouringEvent(InResult);


	
}

// Publish finished event
bool FSLContactEventHandler::FinishContactEvent(USLBaseIndividual* InOther, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedContactEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Individual2 == InOther)
		{
			// Set the event end time
			(*EventItr)->EndTime = EndTime;

			// Avoid publishing short events
			if (((*EventItr)->EndTime - (*EventItr)->StartTime) > ContactEventMin)
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



// Start new supported by event
void FSLContactEventHandler::AddNewSupportedByEvent(USLBaseIndividual* Supported, USLBaseIndividual* Supporting, float StartTime, const uint64 EventPairId)
{
	// Start a supported by event
	TSharedPtr<FSLSupportedByEvent> Event = MakeShareable(new FSLSupportedByEvent(
		FSLUuid::NewGuidInBase64Url(), StartTime, EventPairId, Supported, Supporting));
	Event->EpisodeId = EpisodeId;
	// Add event to the pending array
	StartedSupportedByEvents.Emplace(Event);
}

// Finish then publish the event
bool FSLContactEventHandler::FinishSupportedByEvent(const uint64 InPairId, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedSupportedByEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->PairId == InPairId)
		{
			// Ignore short events
			if (EndTime - (*EventItr)->StartTime > SupportedByEventMin)
			{
				// Set end time and publish event
				(*EventItr)->EndTime = EndTime;
				OnSemanticEvent.ExecuteIfBound(*EventItr);
			}
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending contact events (this usually is called at end play)
void FSLContactEventHandler::FinishAllEvents(float EndTime)
{
	// Finish contact events
	for (auto& Ev : StartedContactEvents)
	{
		// Ignore short events
		if (EndTime - Ev->StartTime > ContactEventMin)
		{
			// Set end time and publish event
			Ev->EndTime = EndTime;
			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}
	StartedContactEvents.Empty();

	// Finish supported by events
	for (auto& Ev : StartedSupportedByEvents)
	{
		// Ignore short events
		if ((EndTime - Ev->StartTime) > SupportedByEventMin)
		{
			// Set end time and publish event
			Ev->EndTime = EndTime;
			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}
	StartedSupportedByEvents.Empty();

	// Finish pouring events
	for (auto& Ev : StartedPouringEvents)
	{
		// Ignore short events
		if (EndTime - Ev->StartTime > PouringEventMin)
		{
			// Set end time and publish event
			Ev->NumberOfParticles = particlesOverlapEnded;
			Ev->PouringPoseForSourceContainer = PouringPoseForSourceContainer;
			Ev->PouringPoseForDestinationContainer = PouringPoseForDestinationContainer;
			

			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}

	StartedPouringEvents.Empty();

	
}

// Start new Pouring event
void FSLContactEventHandler::AddNewPouringEvent(const FSLContactResult& InResult)
{
	// find out the angle of the containers and define which one is the source and which one is the destination container
	float ContainerEulerXAngle = InResult.Self->GetCachedPose().GetRotation().Euler().X;
	float ContainerEulerYAngle = InResult.Self->GetCachedPose().GetRotation().Euler().Y;

	// Start a semantic Pouring event, check if the source container has required angles around X and Y axis in oder to consider it as source container	
if (InResult.Other->GetClass() == USLParticleIndividual::StaticClass() &&
		(ContainerEulerXAngle > 45.00 || ContainerEulerXAngle < -45.00 
			|| ContainerEulerYAngle > 45.00 || ContainerEulerYAngle < -45.00)) {
		
		// check if the pouring event with given source conainer is already running, if yes then do not create new one
		if (!IsPouringEventCurrentlyRunning(SourceContainersList, InResult)) {
			//if (StartedPouringEvents.Num() == 0 || !HasPouringEventHappened(SourceContainers, InResult)) {
			CurrentPouringEvent = MakeShareable(new FSLPouringEvent(
				FSLUuid::NewGuidInBase64Url(), InResult.Time,
				FSLUuid::PairEncodeCantor(InResult.Self->GetUniqueID(), InResult.Other->GetUniqueID()),
				InResult.Self, InResult.Other, USLPouringEventTypes::PouredOut));
			CurrentPouringEvent->EpisodeId = EpisodeId;
			CurrentPouringEvent->SourceContainerName = InResult.Self->GetClassValue();
			// Add event to the pending Pourings array
			StartedPouringEvents.Emplace(CurrentPouringEvent);

		}

		// needs to keep updated due to potential role change in next event
		SourceContainersList.Add({ InResult.Self->GetClassValue(), InResult.Time });
		
		// Add current pose for source container
		PouringPoseForSourceContainer.Add(InResult.Self->GetCachedPose());

		// Not working
		CurrentPouringEvent->DestinationContainerName = DestinationContainerName;

		// Due to overlapping issues between multiple pouring events, we use last particle start time as endtime of current pouring event
		if (CurrentPouringEvent != NULL && CurrentPouringEvent->Individual1 == InResult.Self)
			CurrentPouringEvent->EndTime = InResult.Time;
		
		
	}
	else if (InResult.Other->GetClass() == USLParticleIndividual::StaticClass() &&
		(ContainerEulerXAngle < 45.00 || ContainerEulerXAngle > -45.00
			|| ContainerEulerYAngle < 45.00 || ContainerEulerYAngle > -45.00)) {
		
		// check if the pouring event with given destination conainer is already running, if yes then do not create new one
		if (!IsPouringEventCurrentlyRunning(DestinationContainersList, InResult)) {
			
			CurrentPouringEvent = MakeShareable(new FSLPouringEvent(
				FSLUuid::NewGuidInBase64Url(), InResult.Time,
				FSLUuid::PairEncodeCantor(InResult.Self->GetUniqueID(), InResult.Other->GetUniqueID()),
				InResult.Self, InResult.Other, USLPouringEventTypes::PouredInto));
			CurrentPouringEvent->EpisodeId = EpisodeId;
			CurrentPouringEvent->DestinationContainerName = InResult.Self->GetClassValue();
			// Add event to the pending Pourings array
			StartedPouringEvents.Emplace(CurrentPouringEvent);
		}

		// needs to keep updated due to potential role change in next event
		DestinationContainersList.Add({ InResult.Self->GetClassValue(), InResult.Time });
		// Add current pose for desti container
		PouringPoseForDestinationContainer.Add(InResult.Self->GetCachedPose());

		// Due to overlapping issues between multiple pouring events, we use last particle start time as endtime of current pouring event
		if (CurrentPouringEvent != NULL && CurrentPouringEvent->Individual1 == InResult.Self)
			CurrentPouringEvent->EndTime = InResult.Time;

	}

}

/*
*  Check if for the given list of Pouring events the container name is the same, if yes then the event has started
*/

bool FSLContactEventHandler::IsPouringEventCurrentlyRunning(TArray<std::tuple<FString, float>> Containers, const FSLContactResult& InResult) {
	for (std::tuple<FString, float> Container : Containers) {
		// check for the same container if the time is below limit or above?
		if (std::get<0>(Container) == InResult.Self->GetClassValue()) {
			// if the pouring event with same container lasts more than 5 seconds, a new event will be created
			if((InResult.Time - std::get<1>(Container)) < MaxPouringEventTime)
				return true;
		}

	}
	return false;
}

// NOT CALLED: Publish finished event
bool FSLContactEventHandler::FinishPouringEvent(USLBaseIndividual* InSelf, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedPouringEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Individual1 == InSelf)
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

// Event called when a semantic overlap event begins
void FSLContactEventHandler::OnSLOverlapBegin(const FSLContactResult& InResult)
{
	AddNewContactEvent(InResult);
}

// Event called when a semantic overlap event ends
void FSLContactEventHandler::OnSLOverlapEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time)
{
	FinishContactEvent(Other, Time);


	// for pouring contacts do some more calculation about what time it should end
	if (Other->GetClass() == USLParticleIndividual::StaticClass()) {
		// find out how many particles are ending overlap with source container
		particlesOverlapEnded = particlesOverlapEnded + 1;
	}
}

// Event called when a supported by event begins
void FSLContactEventHandler::OnSLSupportedByBegin(USLBaseIndividual* Supported, USLBaseIndividual* Supporting, float StartTime, const uint64 PairId)
{
	AddNewSupportedByEvent(Supported, Supporting, StartTime, PairId);
}

// Event called when a 'possible' supported by event ends
void FSLContactEventHandler::OnSLSupportedByEnd(const uint64 PairId1, const uint64 PairId2, float EndTime)
{
	if(!FinishSupportedByEvent(PairId1, EndTime))
	{
		FinishSupportedByEvent(PairId2, EndTime);
	}
}

void FSLContactEventHandler::OnSLPouringBegin(const FSLContactResult& InResult)
{
	AddNewPouringEvent(InResult);
}

void FSLContactEventHandler::OnSLPouringEnds(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time)
{
	FinishPouringEvent(Other, Time);
}


