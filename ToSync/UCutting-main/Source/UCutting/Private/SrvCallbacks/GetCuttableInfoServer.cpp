// Fill out your copyright notice in the Description page of Project Settings.


#include"SrvCallbacks/GetCuttableInfoServer.h"
#include "Tags.h"
#include "world_control_msgs/srv/GetCuttableInfo.h"
#include "Cuttable.h"
#include "Conversions.h"

TSharedPtr<FROSBridgeSrv::SrvRequest> FROSGetCuttableInfoServer::FromJson(TSharedPtr<FJsonObject> JsonObject) const
{
    TSharedPtr<FROSGetCuttableInfoSrv::Request> Request_ =
        MakeShareable(new FROSGetCuttableInfoSrv::Request());
    Request_->FromJson(JsonObject);
    return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request_);
}

TSharedPtr<FROSBridgeSrv::SrvResponse> FROSGetCuttableInfoServer::Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> Request)
{
    TSharedPtr<FROSGetCuttableInfoSrv::Request> GetCuttableInfoRequest=StaticCastSharedPtr<FROSGetCuttableInfoSrv::Request>(Request);
    FString AnswerName=TEXT("-1");
    TArray<world_control_msgs::CuttablePartDescription> CuttableParts;
    bool bSuccess=false;

    if(GetCuttableInfoRequest)
    {

        //Cast success Request has correct format
        FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
        {
            TArray<AActor*> Actors = FTags::GetActorsWithKeyValuePair(World, TEXT("SemLog"), TEXT("Id"), GetCuttableInfoRequest->GetId());

    //        TArray <TSharedPtr<FJsonValue>> CuttableParts;
            CuttableParts.Empty();
            if (Actors.IsValidIndex(0))
            {
                AActor* PossibleCuttable = Actors.Pop();
                ACuttable* Cuttable=Cast<ACuttable>(PossibleCuttable);
                if(Cuttable)
                {
                    TArray<UProceduralMeshComponent*> ProMeshPartsArray;
                    Cuttable->GetComponents<UProceduralMeshComponent>(ProMeshPartsArray);
                    for(UProceduralMeshComponent* ProMeshComp : ProMeshPartsArray)
                    {
                        FString ProMeshPartName=ProMeshComp->GetName();
                        const FVector UELocation = ProMeshComp->GetComponentLocation();
                        FVector ROSLocation=FConversions::UToROS(UELocation);
                        const FQuat UERotation= ProMeshComp->GetComponentRotation().Quaternion();
                        FQuat ROSRotation=FConversions::UToROS(UERotation);
                        geometry_msgs::Point* GeoMsgPoint = new geometry_msgs::Point(ROSLocation);
                        geometry_msgs::Quaternion* GeoMshQuad = new geometry_msgs::Quaternion(ROSRotation);
                        geometry_msgs::Pose* ProMeshCompPose=new geometry_msgs::Pose(*GeoMsgPoint,*GeoMshQuad);
                        float radius=ProMeshComp->Bounds.SphereRadius;
                        // We assume that the Otherhalf is a cube inside of the Sphere --> This means the Height Width and Depths all have the same value --> r*sqrt(2)
                        float CubeSize=radius*FGenericPlatformMath::Sqrt(2);
                        world_control_msgs::CuttablePartDescription*  ProMeshCompMsg=new world_control_msgs::CuttablePartDescription(ProMeshPartName,*ProMeshCompPose,CubeSize,CubeSize,CubeSize); //Cube has same Height,width and Depth
                        CuttableParts.Add(*ProMeshCompMsg);
                        UE_LOG(LogTemp, Warning, TEXT("In For CuttableParts Length: %d"),CuttableParts.Num());
                    }
                    UE_LOG(LogTemp, Warning, TEXT("After For CuttableParts Length: %d"),CuttableParts.Num());
                    bSuccess=true;
                    AnswerName=Cuttable->GetName();

                }
            }
        }, TStatId(), nullptr, ENamedThreads::GameThread);
        UE_LOG(LogTemp, Warning, TEXT("Before Wait"));
        FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
        UE_LOG(LogTemp, Warning, TEXT("After Wait"));
    }
    UE_LOG(LogTemp, Warning, TEXT("Before Return CuttableParts Length: %d"),CuttableParts.Num());
    auto help= new FROSGetCuttableInfoSrv::Response(AnswerName,CuttableParts,bSuccess);
    UE_LOG(LogTemp, Warning, TEXT("Before Return Message as String: %s"),*help->ToString());
    for(auto valeera :help->GetCuttableParts())
    {
           UE_LOG(LogTemp, Warning, TEXT("CuttableParts: %s"),*valeera.ToString());
    }
    return MakeShareable<FROSBridgeSrv::SrvResponse>(new FROSGetCuttableInfoSrv::Response(AnswerName,CuttableParts,bSuccess));
}
