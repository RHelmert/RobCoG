// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "Actors/SLCleanerAgent.h"
#include "Individuals/SLIndividualComponent.h"
#include <Events/SLCleaningEvent.h>
#include <Individuals/SLIndividualUtils.h>

ASLCleanerAgent::ASLCleanerAgent()
{
}

void ASLCleanerAgent::Finish(bool bForced)
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

void ASLCleanerAgent::FinishActiveEvents()
{
	if (CurrCleanedIndividual)
	{

		// Fake a Cut abort call
		float EndTime = currentWorld->GetTimeSeconds();
		int32 enumval = static_cast<int32>(USLCleaningStatus::CleaningNotFullyFinished);
		CleaningEndedEvent.Broadcast(CurrCleanedIndividual->GetIndividualObject(), EndTime, enumval);

		//OnSLGraspEnd(OwnerIndividualObject, CurrGraspedIndividual, GetWorld()->GetTimeSeconds());
	}
}

bool ASLCleanerAgent::CheckIndividual(AActor* other, USLBaseIndividual*& result)
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

void ASLCleanerAgent::BeginPlay()
{
	Super::BeginPlay();
	currentWorld = GetWorld();
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s Init done with result: %s"), *FString(__FUNCTION__), __LINE__, *GetName(), Init() ? TEXT("True") : TEXT("False"));
	Start();
}

bool ASLCleanerAgent::Init()
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

		// Set the individual object
		// We are the owner individual object
		//OwnerIndividualObject = OwnerIndividualComponent->GetIndividualObject();
		bIsInit = true;
		return true;
	}
	return false;
}

void ASLCleanerAgent::Start()
{
	if (!bIsStarted && bIsInit)
	{
		//Nothing atm
	}
}

bool ASLCleanerAgent::CleaningStarted(UPARAM(ref) AActor* other)
{
	if (other == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d Cutting Startedcalled but no other object given to cut"), *FString(__FUNCTION__), __LINE__);
		return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Sucessfully started cutting Event"), *FString(__FUNCTION__), __LINE__);


	UActorComponent* AC = other->GetComponentByClass(USLIndividualComponent::StaticClass());
	CurrCleanedIndividual = CastChecked<USLIndividualComponent>(AC);

	USLBaseIndividual* IndividualResult = nullptr;
	//first, check whetter the other individual is valid or should be ignored
	if (CheckIndividual(other, IndividualResult)) {
		// Start new Cutting event
		float StartTime = currentWorld->GetTimeSeconds();
		FSLInteractionResult CuttingResult(OwnerIndividualComponent->GetIndividualObject(), IndividualResult, StartTime);
		CleaningStartedEvent.Broadcast(CuttingResult);
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Cut Object has no individual or should be ignored. No event will be created"), *FString(__FUNCTION__), __LINE__);
		return false;
	}
}

bool ASLCleanerAgent::CleaningStopped(UPARAM(ref) AActor* other)
{
	if (other == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d Cutting CuttingAborted called but no other object given to cut"), *FString(__FUNCTION__), __LINE__)
			return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Sucessfully aborted cutting Event"), *FString(__FUNCTION__), __LINE__);




	USLBaseIndividual* IndividualResult = nullptr;

	//first, check wheter the other individual is valid or should be ignored
	if (CheckIndividual(other, IndividualResult)) {

		float EndTime = currentWorld->GetTimeSeconds();
		int32 enumval = static_cast<int32>(USLCleaningStatus::CleaningNotFullyFinished);
		CleaningEndedEvent.Broadcast(IndividualResult, EndTime, enumval); //CuttingSucceed
		
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Cut Object has no individual or should be ignored. No event will be aborted"), *FString(__FUNCTION__), __LINE__);
		return false;
	}
}

bool ASLCleanerAgent::CleaningFinished(UPARAM(ref) AActor* other)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Sucessfully finished cutting Event"), *FString(__FUNCTION__), __LINE__);
	//Check if the other actor should be ignored and no Events should be logged

	if (other == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d Cutting CuttingSucceeded called but no other object given to cut"), *FString(__FUNCTION__), __LINE__)
			return false;
	}

	USLBaseIndividual* IndividualResult = nullptr;
	//first, check wheter the other individual is valid or should be ignored
	if (CheckIndividual(other, IndividualResult)) {

		float EndTime = currentWorld->GetTimeSeconds();
		int32 enumval = static_cast<int32>(USLCleaningStatus::CleaningFinished);
		CleaningEndedEvent.Broadcast(IndividualResult, EndTime, enumval); //CuttingSucceed
		return true;

	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Cut Object has no individual or should be ignored. No event will be finished"), *FString(__FUNCTION__), __LINE__);
		return false;
	}
}



