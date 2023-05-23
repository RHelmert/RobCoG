// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "RWCManager.h"
#include "ROSBridgeHandler.h"
#include "ROSBridgeSrvServer.h"
#include "ROSBridgeSrv.h"
/**
 * 
 */
class FROSGetCuttableInfoServer final : public FROSBridgeSrvServer
{

private:
    FROSGetCuttableInfoServer();

    UWorld* World;

//    URWCManager* Controller;
public:
    FROSGetCuttableInfoServer(FString Namespace, FString Name, UWorld* InWorld/*, URWCManager* InController*/) :
        FROSBridgeSrvServer(Namespace + TEXT("/") + Name, TEXT("world_control_msgs/GetCuttableInfo")),
        World(InWorld)/*, Controller(InController)*/
    {
    }

    TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override;

    TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> Request) override;
};
