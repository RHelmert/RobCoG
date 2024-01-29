// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLPouringEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Owl/SLOwlExperimentStatics.h"
#include <iostream>
#include <string>  

// Constructor with initialization
FSLPouringEvent::FSLPouringEvent(const FString& InId, float InStart, float InEnd, uint64 InPairId,
	USLBaseIndividual* InIndividual1,
	USLBaseIndividual* InIndividual2,
	USLPouringEventTypes PouringEventType):
	ISLEvent(InId, InStart, InEnd),
	PairId(InPairId),
	Individual1(InIndividual1), 
	Individual2(InIndividual2),
	PouringEventTypes(PouringEventType)
{
}

// Constructor initialization without end time
FSLPouringEvent::FSLPouringEvent(const FString& InId, float InStart, uint64 InPairId,
	USLBaseIndividual* InIndividual1,
	USLBaseIndividual* InIndividual2,
	USLPouringEventTypes PouringEventType):
	ISLEvent(InId, InStart),
	PairId(InPairId),
	Individual1(InIndividual1),
	Individual2(InIndividual2),
	PouringEventTypes(PouringEventType)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLPouringEvent::ToOwlNode() const
{
	// Create the Pouring event node
	FString EventName = "PouringSituation: " + FString(PouringEventTypes == USLPouringEventTypes::PouredInto ? "Poured in to" : "Poured out");
	
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, EventName);
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", StartTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", EndTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInEpisodeProperty("log", EpisodeId));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInContactProperty("log", Individual1->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInContactProperty("log", Individual2->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreatePouredParticlesProperty("log", NumberOfParticles));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLPouringEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
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
FString FSLPouringEvent::Context() const
{
	FString EventType = FString(PouringEventTypes == USLPouringEventTypes::PouredInto ? "Poured in to" : "Poured out");
	return FString::Printf(TEXT("%s - %lld"), *EventType , PairId);
}

// Get the tooltip data
FString FSLPouringEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'O1\',\'%s\',\'Id\',\'%s\',\'O2\',\'%s\',\'Id\',\'%s\', \'Id\',\'%s\'"),
		*Individual1->GetClassValue(), *Individual1->GetIdValue(), *Individual2->GetClassValue(), *Individual2->GetIdValue(), *Id);
}

// Get the data as string
FString FSLPouringEvent::ToString() const
{
	return FString::Printf(TEXT("Individual1:[%s] Individual2:[%s] PairId:%lld"),
		*Individual1->GetInfo(), *Individual2->GetInfo(), PairId);
}

FString FSLPouringEvent::RESTCallToKnowRob(FSLKRRestClient* InFSLKRRestClient) const
{
	// Call REST method to create sub actions on KnowRob side...
	// somaClassName:somaIndividualName is sent at the moment for objects participated
	FString ObjectsPartcipated = TEXT("[") 
		+ Individual1->GetClassValue() + TEXT(":") + Individual1->GetParentActor()->GetActorLabel() 
		+ TEXT(",") 
		+ Individual2->GetClassValue() + TEXT(":") + Individual2->GetParentActor()->GetActorLabel() 
		+ TEXT("]");

	FString SubActionType = TEXT("soma:'") + FString(PouringEventTypes == USLPouringEventTypes::PouredInto ? "PouredInTo" : "PouredOut") 
		+ TEXT("'");
	FString TaskType = TEXT("soma:'Pouring'");
	
	
	// get additional info for pouring event as JSonFString
	FString AdditionalInfoJsonFString = CreateAdditionalInfoForPouringEvent();
	//FString AdditionalInfoJsonFString = TEXT("test");
	UE_LOG(LogTemp, Warning, TEXT("Additional info for pouring event: %s"), *AdditionalInfoJsonFString);

	// create a pouring event
	InFSLKRRestClient->SendCreateSubActionRequest(SubActionType, TaskType,
		ObjectsPartcipated, AdditionalInfoJsonFString, double(StartTime), double(EndTime));

	

	return TEXT("Succeed!");
}

/**
* This method will create a json string with all the necessary information for pouring event that needs to be logged in NEEM
* The formate will be {	'SCName':'XYZ',
						'DCName':'XYZ',
						'MaxPouringAngle':{'X':,'Y':,'Z':},
						'MinPouringAngle':{'X':,'Y':,'Z':},
						'SCPoses':[
							{'X':,'Y':,'Z':},
							{'X':,'Y':,'Z':}
						],
						'DCPoses':[
							{'X':,'Y':,'Z':},
							{'X':,'Y':,'Z':}
						]
* 
*/
FString FSLPouringEvent::CreateAdditionalInfoForPouringEvent() const {

	// Do calculation for poses along with max and min angles
	FVector MaxPouringAngle, MinPouringAngle;
	MaxPouringAngle.X = -180;
	MinPouringAngle.X = 180;
	TArray<FVector> SourceContainerPoses;
	TArray<FVector> DestinationContainerPoses;

	// logic for selection of Source or Destination Container poses Array object..
	if (PouringEventTypes == USLPouringEventTypes::PouredOut) {
		// pouring out event shows that the source container locations are stores in PouringPoseForSourceContainer

		for (FTransform PP : PouringPoseForSourceContainer) {
			// if given container name is same as the one stored in tuple
			FVector PPRotationEuler = PP.GetRotation().Euler();

			// find min and max pouring angles
			if (MaxPouringAngle.X < PPRotationEuler.X) {
				MaxPouringAngle = PPRotationEuler;
			}
			if (MinPouringAngle.X > PPRotationEuler.X) {
				MinPouringAngle = PPRotationEuler;
			}

			// get all poses for source container
			SourceContainerPoses.Add(PP.GetTranslation());
		}

	}
	else {
		// pouring out event will have destination locations stored in PouringPoseForSourceContainer
		for (FTransform PP : PouringPoseForDestinationContainer) {
			DestinationContainerPoses.Add(PP.GetTranslation());
		}
	}

	FString AdditionalInfoJsonString = TEXT("{");

	// add source and destination container names
	AdditionalInfoJsonString += FString::Printf(TEXT("\'SCName\':\'%s\'"),
		*SourceContainerName);
	// comma seperator
	AdditionalInfoJsonString += TEXT(",");

	AdditionalInfoJsonString += FString::Printf(TEXT("\'DCName\':\'%s\'"),
		*DestinationContainerName);

	// comma seperator
	AdditionalInfoJsonString += TEXT(",");

	// add max pour angle
	AdditionalInfoJsonString += FString::Printf(TEXT("\'MaxPouringAngle\':{\'X\':\'%lf\',\'Y\':\'%lf\',\'Z\':\'%lf\'}"),
		MaxPouringAngle.X, MaxPouringAngle.Y, MaxPouringAngle.Z);

	// comma seperator
	AdditionalInfoJsonString += TEXT(",");

	// add min pour angle
	AdditionalInfoJsonString += FString::Printf(TEXT("\'MinPouringAngle\':{\'X\':\'%lf\',\'Y\':\'%lf\',\'Z\':\'%lf\'}"),
		MinPouringAngle.X, MinPouringAngle.Y, MinPouringAngle.Z);

	// comma seperator
	AdditionalInfoJsonString += TEXT(",");

	// add source container poses
	AdditionalInfoJsonString += FString::Printf(TEXT("\'SCPoses\':["));

	// for each SCPose
	int NumOfSCPoses = SourceContainerPoses.Num();
	int Count = 0;
	for (auto& SCPose : SourceContainerPoses) {
		if (Count == (NumOfSCPoses - 1)) {
			AdditionalInfoJsonString += FString::Printf(TEXT("{\'X\':\'%lf\',\'Y\':\'%lf\',\'Z\':\'%lf\'}"),
				SCPose.X, SCPose.Y, SCPose.Z);
		}
		else {
			AdditionalInfoJsonString += FString::Printf(TEXT("{\'X\':\'%lf\',\'Y\':\'%lf\',\'Z\':\'%lf\'},"),
				SCPose.X, SCPose.Y, SCPose.Z);
		}
		Count++;
	}
	AdditionalInfoJsonString += FString::Printf(TEXT("]"));

	// comma seperator
	AdditionalInfoJsonString += TEXT(",");

	// add destination container poses
	AdditionalInfoJsonString += FString::Printf(TEXT("\'DCPoses\':["));

	// for each SCPose
	for (auto& DCPose : DestinationContainerPoses) {
		AdditionalInfoJsonString += FString::Printf(TEXT("{\'X\':\'%lf\',\'Y\':\'%lf\',\'Z\':\'%lf\'},"),
			DCPose.X, DCPose.Y, DCPose.Z);
	}
	AdditionalInfoJsonString += FString::Printf(TEXT("]"));


	AdditionalInfoJsonString += TEXT("}");

	return AdditionalInfoJsonString;
}

// sending JsonObject as FString does not work for some reason, hence we need to create our own Json String representaion
//FString FSLPouringEvent::CreateAdditionalInfoForPouringEvent() const{
// 
//	// Do calculation for poses along with max and min angles
//	FVector MaxPouringAngle, MinPouringAngle;
//	MaxPouringAngle.X = -180;
//	MinPouringAngle.X = 180;
//	TArray<FVector> SourceContainerPoses;
//	TArray<FVector> DestinationContainerPoses;
//
//	// logic for selection of Source or Destination Container poses Array object..
//	if (PouringEventTypes == USLPouringEventTypes::PouredOut) {
//		// pouring out event shows that the source container locations are stores in PouringPoseForSourceContainer
//
//		for (FTransform PP : PouringPoseForSourceContainer) {
//			// if given container name is same as the one stored in tuple
//			FVector PPRotationEuler = PP.GetRotation().Euler();
//
//			// find min and max pouring angles
//			if (MaxPouringAngle.X < PPRotationEuler.X) {
//				MaxPouringAngle = PPRotationEuler;
//			}
//			if (MinPouringAngle.X > PPRotationEuler.X) {
//				MinPouringAngle = PPRotationEuler;
//			}
//
//			// get all poses for source container
//			SourceContainerPoses.Add(PP.GetTranslation());
//		}
//
//	}
//	else {
//		// pouring out event will have destination locations stored in PouringPoseForSourceContainer
//		for (FTransform PP : PouringPoseForDestinationContainer) {
//			DestinationContainerPoses.Add(PP.GetTranslation());
//		}
//	}
//
//	TSharedPtr<FJsonObject> AdditionalInfoJsonObject = MakeShareable(new FJsonObject());
//	
//	AdditionalInfoJsonObject->SetStringField("SCName", SourceContainerName);
//	AdditionalInfoJsonObject->SetStringField("DCName", DestinationContainerName);
//
//	
//	TSharedPtr<FJsonObject> MaxPouringAngleJson = MakeShareable(new FJsonObject());
//	MaxPouringAngleJson->SetNumberField("X", MaxPouringAngle.X);
//	MaxPouringAngleJson->SetNumberField("Y", MaxPouringAngle.Y);
//	MaxPouringAngleJson->SetNumberField("Z", MaxPouringAngle.Z);
//	AdditionalInfoJsonObject->SetObjectField("MaxPouringAngle", MaxPouringAngleJson);
//
//	TSharedPtr<FJsonObject> MinPouringAngleJson = MakeShareable(new FJsonObject());
//	MinPouringAngleJson->SetNumberField("X", MinPouringAngle.X);
//	MinPouringAngleJson->SetNumberField("Y", MinPouringAngle.Y);
//	MinPouringAngleJson->SetNumberField("Z", MinPouringAngle.Z);
//	AdditionalInfoJsonObject->SetObjectField("MinPouringAngle", MinPouringAngleJson);
//
//	TArray<TSharedPtr<FJsonValue>> SCPoseJsonArray;
//	for (auto& SCPose : SourceContainerPoses) {
//		SCPoseJsonArray.Add((MakeShareable(new FJsonValueString(SCPose.ToString()))));
//	}
//	
//	AdditionalInfoJsonObject->SetArrayField("SCPoses", SCPoseJsonArray);
//
//	TArray<TSharedPtr<FJsonValue>> DCPoseJsonArray;
//	for (auto& DCPose : DestinationContainerPoses) {
//		DCPoseJsonArray.Add((MakeShareable(new FJsonValueString(DCPose.ToString()))));
//	}
//
//	AdditionalInfoJsonObject->SetArrayField("DCPoses", DCPoseJsonArray);
//
//	FString AdditionalInfoJsonFString;
//
//	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&AdditionalInfoJsonFString);
//
//	FJsonSerializer::Serialize(AdditionalInfoJsonObject.ToSharedRef(), Writer);
//
//	return AdditionalInfoJsonFString;
//}

/* End ISLEvent interface */