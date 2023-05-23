// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ROSBridgeHandler.h"
#include "ROSBridgeSrvServer.h"
#include "ROSBridgeSrv.h"
//#include "RWCManager.h"

/**
 * 
 */
class FROSActivateCutterServer final: public FROSBridgeSrvServer
{
public:
    FROSActivateCutterServer(){}

    class UWorld* World;

    FThreadSafeBool ServiceSuccess;

public:
//    class URWCManager* Controller;

    FROSActivateCutterServer(FString Namespace, FString Name, UWorld* InWorld/*, class URWCManager* InController*/) :
        FROSBridgeSrvServer(Namespace + TEXT("/") + Name, TEXT("world_control_msgs/ActivateCutter"))
    {
//        Controller = InController;
        World = InWorld;
    }


    TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override;

    TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> Request) override;
};
