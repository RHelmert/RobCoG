// Fill out your copyright notice in the Description page of Project Settings.


#include "CuttingHandlerComponent.h"
#include "SrvCallbacks/ActivateCutterServer.h"
#include "SrvCallbacks/GetCuttableInfoServer.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "ROSBridgeGameInstance.h"
#include "ROSBridgeHandler.h"

// Sets default values for this component's properties
UCuttingHandlerComponent::UCuttingHandlerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
    Namespace="Cutting";
}


// Called when the game starts
void UCuttingHandlerComponent::BeginPlay()
{
	Super::BeginPlay();
    World=GetOwner()->GetWorld();
    UGameInstance* PossibleROSGameInstance= World->GetGameInstance();
    UROSBridgeGameInstance* ROSGameInstance= Cast<UROSBridgeGameInstance>(PossibleROSGameInstance);
    if(ROSGameInstance)
    {
        if(ROSGameInstance->ROSHandler.IsValid())
        {
            ConnectToHandler(ROSGameInstance->ROSHandler);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s]: There is a ROSGameInstance but there is no valid ROSHandler. Can not advertise serives."), *FString(__FUNCTION__));
        }

    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s]: No ROSGameInstance. Can not advertise serives."), *FString(__FUNCTION__));
    }

	
}


// Called every frame
void UCuttingHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}



//void UCuttingHandlerComponent::Register(FString DefaultNamespace, UWorld *InWorld)
//{


//    NameSpace=TEXT("Cutting");
//    if(!World)
//    {
//        UE_LOG(LogTemp, Warning, TEXT("[%s]: GetWorld returned, NULL."), *FString(__FUNCTION__));
//    }

//}

void UCuttingHandlerComponent::ConnectToHandler(const TSharedPtr<FROSBridgeHandler> &ROSHandler)
{
    //Connect all
    ROSHandler->AddServiceServer(MakeShareable<FROSActivateCutterServer>(new FROSActivateCutterServer(Namespace, TEXT("activate_cutter"), World)));
    ROSHandler->AddServiceServer(MakeShareable<FROSGetCuttableInfoServer>(new FROSGetCuttableInfoServer(Namespace, TEXT("get_cuttable_info"), World)));
}

//void UCuttingHandlerComponent::SetupServiceServers()
//{
//    ServicesToPublish.Add(MakeShareable<FROSActivateCutterServer>(new FROSActivateCutterServer(Namespace, TEXT("activate_cutter"), World, this)));
//    ServicesToPublish.Add(MakeShareable<FROSGetCuttableInfoServer>(new FROSGetCuttableInfoServer(Namespace, TEXT("get_cuttable_info"), World, this)));
//}
