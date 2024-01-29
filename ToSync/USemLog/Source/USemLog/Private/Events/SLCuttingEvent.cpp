// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLCuttingEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Owl/SLOwlExperimentStatics.h"
#include <iostream>
#include <string>  

// Constructor with initialization
FSLCuttingEvent::FSLCuttingEvent(const FString& InId, float InStart, float InEnd, uint64 InPairId,
	USLBaseIndividual* InIndividual1,
	USLBaseIndividual* InIndividual2):
	ISLEvent(InId, InStart, InEnd),
	PairId(InPairId),
	Individual1(InIndividual1), 
	Individual2(InIndividual2)
{
}

// Constructor initialization without end time
FSLCuttingEvent::FSLCuttingEvent(const FString& InId, float InStart, uint64 InPairId,
	USLBaseIndividual* InIndividual1,
	USLBaseIndividual* InIndividual2):
	ISLEvent(InId, InStart),
	PairId(InPairId),
	Individual1(InIndividual1),
	Individual2(InIndividual2)
{
}


/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLCuttingEvent::ToOwlNode() const
{
	// Create the Cutting event node
	FString EventName = "CuttingSituation:Cutting";
	
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, EventName);
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", StartTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", EndTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInEpisodeProperty("log", EpisodeId));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInContactProperty("log", Individual1->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInContactProperty("log", Individual2->GetIdValue()));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLCuttingEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
{
	// Add timepoint individuals
	// We know that the document is of type FOwlExperiment,
	// we cannot use the safer dynamic_cast because RTTI is not enabled by default
	// if (FOwlEvents* EventsDoc = dynamic_cast<FOwlEvents*>(OutDoc))
	FSLOwlExperiment* EventsDoc = static_cast<FSLOwlExperiment*>(OutDoc);
	EventsDoc->RegisterTimepoint(StartTime);
	EventsDoc->RegisterTimepoint(EndTime);
	EventsDoc->RegisterObject(Individual1);
	EventsDoc->RegisterObject(Individual2);
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLCuttingEvent::Context() const
{
	FString EventType = FString("Cutting");
	return FString::Printf(TEXT("%s - %lld"), *EventType , PairId);
}

// Get the tooltip data
FString FSLCuttingEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'O1\',\'%s\',\'Id\',\'%s\',\'O2\',\'%s\',\'Id\',\'%s\', \'Id\',\'%s\'"),
		*Individual1->GetClassValue(), *Individual1->GetIdValue(), *Individual2->GetClassValue(), *Individual2->GetIdValue(), *Id);
}

// Get the data as string
FString FSLCuttingEvent::ToString() const
{
	return FString::Printf(TEXT("Individual1:[%s] Individual2:[%s] PairId:%lld"),
		*Individual1->GetInfo(), *Individual2->GetInfo(), PairId);
}

FString FSLCuttingEvent::RESTCallToKnowRob(FSLKRRestClient* InFSLKRRestClient) const
{
	// Call REST method to create sub actions on KnowRob side...
	// somaClassName:somaIndividualName is sent at the moment for objects participated
	FString ObjectsPartcipated = TEXT("[") 
		+ Individual1->GetClassValue() + TEXT(":") + Individual1->GetParentActor()->GetActorLabel() 
		+ TEXT(",") 
		+ Individual2->GetClassValue() + TEXT(":") + Individual2->GetParentActor()->GetActorLabel() 
		+ TEXT("]");

	FString SubActionType = TEXT("soma:'Cut'");
	FString TaskType = TEXT("soma:'Cutting'");
	
	FString AdditionalEventInfo = TEXT("");

	// create a Cutting event
	InFSLKRRestClient->SendCreateSubActionRequest(SubActionType, TaskType,
		ObjectsPartcipated, AdditionalEventInfo, double(StartTime), double(EndTime));

	

	return TEXT("Succeed!");
}

/* End ISLEvent interface */