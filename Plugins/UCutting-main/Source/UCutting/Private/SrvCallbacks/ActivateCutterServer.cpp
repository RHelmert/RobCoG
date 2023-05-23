// Fill out your copyright notice in the Description page of Project Settings.


#include "SrvCallbacks/ActivateCutterServer.h"
#include "Engine/World.h"
#include "world_control_msgs/srv/ActivateCutter.h"
#include "Tags.h"
#include "Cutter.h"


TSharedPtr<FROSBridgeSrv::SrvRequest> FROSActivateCutterServer::FromJson(TSharedPtr<FJsonObject> JsonObject) const
{
    TSharedPtr<FROSActivateCutterSrv::Request> Request =
        MakeShareable(new FROSActivateCutterSrv::Request());
    Request->FromJson(JsonObject);
    return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);

}


TSharedPtr<FROSBridgeSrv::SrvResponse> FROSActivateCutterServer::Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> Request)
{
    TSharedPtr<FROSActivateCutterSrv::Request> ActivateCutterRequest =
        StaticCastSharedPtr<FROSActivateCutterSrv::Request>(Request);
    //Work with request
    bool bSuccess=true;
    FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
    {
        TArray<AActor*> Actors = FTags::GetActorsWithKeyValuePair(World,TEXT("SemLog"),TEXT("Id"),ActivateCutterRequest->GetId());
        if(Actors.IsValidIndex(0))
        {
            AActor* PossibleCutter=Actors.Pop();
            ACutter* Cutter=Cast<ACutter>(PossibleCutter);
            if(Cutter)
            {
                UE_LOG(LogTemp, Log, TEXT("%s"), *ActivateCutterRequest->ToString());
                Cutter->SetbCanCut(ActivateCutterRequest->GetbCanCut());
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Could not find Actor with Semlog Id: %s does not exist."), *ActivateCutterRequest->GetId());
            bSuccess=false;

        }
    }, TStatId(), nullptr, ENamedThreads::GameThread);
    //wait code above to complete
    FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);

    return MakeShareable<FROSBridgeSrv::SrvResponse>(new FROSActivateCutterSrv::Response(bSuccess));

}
