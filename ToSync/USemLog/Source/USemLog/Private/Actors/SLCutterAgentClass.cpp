// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "Actors/SLCutterAgentClass.h"
#include "Individuals/SLIndividualComponent.h"
#include <Individuals/SLIndividualUtils.h>
#include "Individuals/Type/SLBaseIndividual.h"
#include <Monitors/SLManipulatorMonitor.h>
#include <Monitors/SLMonitorStructs.h>
#include <Kismet/GameplayStatics.h>
#include <Events/SLCuttingEvent.h>


//Ctor
ASLCutterAgentClass::ASLCutterAgentClass()
{

}

// Dtor
ASLCutterAgentClass::~ASLCutterAgentClass()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}


void ASLCutterAgentClass::BeginPlay()
{
	Super::BeginPlay();
	currentWorld = GetWorld();

}


bool ASLCutterAgentClass::Init()
{
	if (!bIsInit)
	{
		// Make sure the owner is semantically annotated
		if (UActorComponent* AC = GetComponentByClass(USLIndividualComponent::StaticClass()))
		{
			OwnerIndividualComponent = CastChecked<USLIndividualComponent>(AC);
			if (!OwnerIndividualComponent->IsLoaded())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual component is not loaded.."), *FString(__FUNCTION__), __LINE__, *GetName());
				return false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no individual component.."), *FString(__FUNCTION__), __LINE__, *GetName());
			return false;
		}

		bIsInit = true;
		return true;
	}
	return false;
}

// Start listening to grasp events, update currently overlapping objects
void ASLCutterAgentClass::Start()
{
	if (!bIsStarted && bIsInit)
	{
		//Deprecated (you can hide the following by pressing the - to the left)
		/*
		// Not necessary, or maybe in the futuer
		// 
		// 
		// Get the manipulation monitor from the VR Pawn
		//if (AActor* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0)) {
		

			// Find the attached actor
			//TArray<AActor*> AttachedActors;
			//TArray<USLManipulatorMonitor*> ManipulationMonitors;
			//PlayerPawn->GetAttachedActors(AttachedActors);

			//FString PawnName = PlayerPawn->GetClass()->GetName();

			////AMyDerivedClass* DerivedClass = Cast<AMyDerivedClass>(SomeBaseClass);

			//UE_LOG(LogTemp, Error, TEXT("%s::%d %s Pawn is of class: %s"), *FString(__FUNCTION__), __LINE__, *GetName(), *PawnName);
			//UE_LOG(LogTemp, Error, TEXT("%s::%d %s Found %d attached actors to pawn."), *FString(__FUNCTION__), __LINE__, *GetName(), AttachedActors.Num());


			// We are too early, since the Hands are not yet attached to the pawn
			// If you ever want to change the way the pawn works you have to give here the references to the manipulation monitors
			// Or lets just look at cutting. We do not need a hand to cut.
			//if (AttachedActors.Num() == 0) {
			
			
			
			//}

			/*for (AActor* AttachedActor : AttachedActors)
			{
				// Access the components of the attached actor
				USLManipulatorMonitor* AttachedComponent = AttachedActor->FindComponentByClass<USLManipulatorMonitor>();
				if (AttachedComponent)
				{
					ManipulationMonitors.Add(AttachedComponent);

				}
				
			}

			//UE_LOG(LogTemp, Error, TEXT("%s::%d Found %d Manipulation Monitors on the Pawn"), *FString(__func__), __LINE__,ManipulationMonitors.Num());

			//Cannot work at the moment since the pawn has two hands and not the manipulation monitor
			//for (USLManipulatorMonitor* Sibling : ManipulationMonitors)
			//{
			//	//not yet used
			//	//use it t get the important events and data to work with and when it did not work throw the error
			//	Sibling->OnBeginManipulatorGrasp.AddUObject(this, &ASLCutterAgentClass::OnSLGraspBegin);
			//	Sibling->OnEndManipulatorGrasp.AddUObject(this, &ASLCutterAgentClass::OnSLGraspEnd);


			//	//when everything works, continue
			//	
			//}

			// Let the function listen to the Event
			//CuttingSucceededEvent.AddDynamic(this, &ASLCutterAgentClass::ObjectCut);
			

			//else
			//if(ManipulationMonitors.Num() != 2)
			//{
			//UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find enough USLManipulatorMonitor .."), *FString(__func__), __LINE__);
			//}
			//else {
			//	bIsStarted = true;
			//}

		//}
		//else {
		//	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the PlayerPawn .."), *FString(__func__), __LINE__);
		//}*/
		
	}
}







bool ASLCutterAgentClass::CuttingStarted(UPARAM(ref) AActor* other) {
	
	//Write in Console 
	if (other == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d Cutting Started called but no other object given to cut"), *FString(__FUNCTION__), __LINE__);
			return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Sucessfully started cutting Event"), *FString(__FUNCTION__), __LINE__);

		
	UActorComponent* AC = other->GetComponentByClass(USLIndividualComponent::StaticClass());
	CurrCutIndividual = CastChecked<USLIndividualComponent>(AC);
	
	USLBaseIndividual* IndividualResult = nullptr;
	//first, check whetter the other individual is valid or should be ignored
	if (CheckIndividual(other, IndividualResult)) {
		// Start new Cutting event
		float StartTime = currentWorld->GetTimeSeconds();
		FSLInteractionResult CuttingResult(OwnerIndividualComponent->GetIndividualObject(), IndividualResult, StartTime);
		CuttingStartedEvent.Broadcast(CuttingResult);
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Cut Object has no individual or should be ignored. No event will be created"), *FString(__FUNCTION__), __LINE__);
		return false;
	}
}


bool ASLCutterAgentClass::CuttingAborted(UPARAM(ref) AActor* other) {
	//Write in Console 
	// Calling Cutting Aborted requres a must use input
	
	if (other == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d Cutting CuttingAborted called but no other object given to cut"), *FString(__FUNCTION__), __LINE__)
			return false;
	}

	UE_LOG(LogTemp, Display, TEXT("%s::%d Sucessfully aborted cutting Event"), *FString(__FUNCTION__), __LINE__);
	//We want to track the whole process of cutting. From the start when the object is hit the first time
	//till the end when the tip of the blade fully went through the body. Therefore it is also possible to abort cutting.
	//For example when the knife enters the body to be cut and then just leaves it the way it went in.

	USLBaseIndividual* IndividualResult = nullptr;

	//first, check wheter the other individual is valid or should be ignored
	if (CheckIndividual(other, IndividualResult)) {

		float EndTime = currentWorld->GetTimeSeconds();
		int32 enumval = static_cast<int32>(USLCuttingInfo::CuttingAborted);
		CuttingEndedEvent.Broadcast(IndividualResult, EndTime, enumval); //CuttingSucceed

		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Cut Object has no individual or should be ignored. No event will be aborted"), *FString(__FUNCTION__), __LINE__);
		return false;
	}
}


bool ASLCutterAgentClass::CuttingSucceeded(UPARAM(ref) AActor* other) {
	//Write in Console 
	UE_LOG(LogTemp, Display, TEXT("%s::%d Sucessfully finished cutting Event"), *FString(__FUNCTION__), __LINE__);
	//Check if the other actor should be ignored and no Events should be logged

	if (other == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d Cutting CuttingSucceeded called but no other object given to cut"), *FString(__FUNCTION__), __LINE__)
			return false;
	}

	USLBaseIndividual* IndividualResult = nullptr;
	//first, check wheter the other individual is valid or should be ignored
	if (CheckIndividual(other, IndividualResult)) {
		
		float EndTime = currentWorld->GetTimeSeconds();
		int32 cuttingResult = static_cast<int32>(USLCuttingInfo::CuttingSucceed);
		CuttingEndedEvent.Broadcast(IndividualResult, EndTime, cuttingResult); //CuttingSucceed
		return true;
	
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Cut Object has no individual or should be ignored. No event will be finished"), *FString(__FUNCTION__), __LINE__);
		return false;
	}
}


void ASLCutterAgentClass::ObjectCut(float val)
{
	//not used atm
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d Sucessfully triggered Event CutterActor: %d"),
	//	*FString(__func__), __LINE__, val);

}



void ASLCutterAgentClass::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Finish any active event
		FinishActiveEvents();

		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}


void ASLCutterAgentClass::FinishActiveEvents()
{
	if (CurrCutIndividual)
	{

		// Fake a Cut abort call
		float EndTime = currentWorld->GetTimeSeconds();
		int32 enumval = static_cast<int32>(USLCuttingInfo::CuttingAborted);
		CuttingEndedEvent.Broadcast(CurrCutIndividual->GetIndividualObject(), EndTime, enumval);
		
		//OnSLGraspEnd(OwnerIndividualObject, CurrGraspedIndividual, GetWorld()->GetTimeSeconds());
	}
}

bool ASLCutterAgentClass::CheckIndividual(AActor* other,USLBaseIndividual*& result)
{
	if (FSLIndividualUtils::CheckIgnoreActor(other)) {
		return false;
	}

	// Check if the component or its outer is semantically annotated
	result = FSLIndividualUtils::GetIndividualObject(other);
	if (result == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not annotated, this should not happen.."), *FString(__FUNCTION__), __LINE__, *other->GetName());
		return false;
	}

	if (!result->IsLoaded())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual component is not loaded.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	return true;
}

