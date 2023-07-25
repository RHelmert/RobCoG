// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "SeperateActorManusComponent.h"

USeperateActorManusComponent::USeperateActorManusComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//
}


bool USeperateActorManusComponent::IsLocalPCControlled() const
{
	if (GEngine && GEngine->GetWorldContextFromWorld(GetWorld()))
	{
		APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
		//UE_LOG(LogManus, Warning, TEXT("SeperateActorManus component: \"Class works\" was set to TRUE to support the Manus Glove finger haptics."));
		//bool local = (LocalPlayerController && GetOwner() == LocalPlayerController->GetPawn());
		//bool bol = GetAttachmentRootActor() == LocalPlayerController->GetPawn();
		
		UE_LOG(LogTemp, Warning, TEXT("ComponentWorks!---"));
		return (LocalPlayerController && (GetOwner() == LocalPlayerController->GetPawn() || GetAttachmentRootActor() == LocalPlayerController->GetPawn()));
	}
	return true;
}