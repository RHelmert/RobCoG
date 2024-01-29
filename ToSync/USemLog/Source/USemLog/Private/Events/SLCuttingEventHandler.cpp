// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "Events/SLCuttingEventHandler.h"
#include <Monitors/SLMonitorStructs.h>
#include <Utils/SLUuid.h>

SLCuttingEventHandler::SLCuttingEventHandler()
{
}

SLCuttingEventHandler::~SLCuttingEventHandler()
{
}



void SLCuttingEventHandler::Init(UObject* InAgentToObserve)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		AgentToObserve = Cast<ASLCutterAgentClass>(InAgentToObserve);
		if (AgentToObserve)
		{
			bIsInit = true;
		}
			
	}

}

void SLCuttingEventHandler::Start()
{

	if (!bIsStarted && bIsInit)
	{

		AgentToObserve->CuttingStartedEvent.AddRaw(this, &SLCuttingEventHandler::OnCuttingBegin);
		AgentToObserve->CuttingEndedEvent.AddRaw(this, &SLCuttingEventHandler::OnCuttingEnd);

		bIsStarted = true;
		//UE_LOG(LogTemp, Log, TEXT("%s::%d Cutting event handler has been started"), *FString(__FUNCTION__), __LINE__);
	}
	else if (bIsInit) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d Cutting event handler could not be started correctly"), * FString(__FUNCTION__), __LINE__);
	
	}
}

void SLCuttingEventHandler::Finish(float EndTime, bool bForced)
{

	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		
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

void SLCuttingEventHandler::FinishAllEvents(float EndTime)
{
	// Finish cutting events
	// Ignore all events not finished
	StartedCuttingEvents.Empty();

}

void SLCuttingEventHandler::OnCuttingBegin(const FSLInteractionResult& cuttingInfo)
{
	AddNewSLCuttingEvent(cuttingInfo);
}

void SLCuttingEventHandler::OnCuttingEnd(USLBaseIndividual* Other, float Time, int32 result)
{

	FinishSLCuttingEvent(Other, Time, static_cast<USLCuttingInfo>(result));
}




//--------------EVENTS
// 
// Start new Cutting event
void SLCuttingEventHandler::AddNewSLCuttingEvent(const FSLInteractionResult& cuttingInfo)
{
	//AStaticMeshActor* ASLCutter = Cast<AStaticMeshActor>(InResult.Self);
	//UActorComponent* comp = GetComponentByClass(ASLCutter::StaticClass());
	//if (InResult.Other->GetClass() == USLParticleIndividual::StaticClass();

	/*if (cuttingInfo.Self->GetParentActor()->IsA(ASLCutterAgentClass::StaticClass())) {
		
		ASLCutterAgentClass* cutter = Cast<ASLCutterAgentClass>(cuttingInfo.Self->GetParentActor());
		UE_LOG(LogTemp, Display, TEXT("%s Cutting event detected ... and IsA"), cutter->isCutting ? TEXT("true") : TEXT("false"));
	}
	else {
		FString ClassName = cuttingInfo.Self->GetParentActor()->GetClass()->GetName();
		UE_LOG(LogTemp, Display, TEXT("%s Cuttin event detected ......."), *ClassName);
		UE_LOG(LogTemp, Display, TEXT("%s Cuttin event detected ......."), *ASLCutterAgentClass::StaticClass()->GetName());
	}*/

	TSharedPtr<FSLCuttingEvent> Event = MakeShareable(new FSLCuttingEvent(
		FSLUuid::NewGuidInBase64Url(), cuttingInfo.Time,
		FSLUuid::PairEncodeCantor(cuttingInfo.Self->GetUniqueID(), cuttingInfo.Other->GetUniqueID()),
		cuttingInfo.Self, cuttingInfo.Other));
	Event->EpisodeId = EpisodeId;

	// Add event to the pending Cutting array
	StartedCuttingEvents.Emplace(Event);
	//}
}

// Publish finished event
bool SLCuttingEventHandler::FinishSLCuttingEvent(USLBaseIndividual* InOther, float EndTime, USLCuttingInfo result)
{
	// Use iterator to be able to remove the entry from the array
	// TIndexedContainerIterator<TArray<TSharedPtr<FSLCuttingEvent>>, TSharedPtr<FSLCuttingEvent>, int32> 
	for (auto EventItr(StartedCuttingEvents.CreateIterator()); EventItr; ++EventItr)
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




