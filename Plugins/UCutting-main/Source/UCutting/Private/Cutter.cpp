// Fill out your copyright notice in the Description page of Project Settings.


#include "Cutter.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent.h"
#include "CustomMeshComponent.h"

// Sets default values
ACutter::ACutter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

//    USceneComponent* RootSceneComponent= CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
//    RootComponent=RootSceneComponent;
    // Use a sphere as a simple collision representation
//    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
//    CollisionComp->InitSphereRadius(8.0f);
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    RootComponent=StaticMesh;

    CollisionComp= CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    CollisionComp->SetGenerateOverlapEvents(true);

    CollisionComp->BodyInstance.SetCollisionProfileName("BlockAll");
    CollisionComp->OnComponentHit.AddDynamic(this, &ACutter::OnHit);		// set up a notification for when this component hits something blocking
    CollisionComp->AttachToComponent(RootComponent,FAttachmentTransformRules::SnapToTargetIncludingScale);
    bCanCut=true;
}



// Called every frame
//void ACutter::Tick(float DeltaTime)
//{
//    Super::Tick(DeltaTime);
//    if(OtherHalf)
//    {

//        UE_LOG(LogTemp, Warning, TEXT("[%s]: OtherHalf is at %s"), *FString(__FUNCTION__),*OtherHalf->GetComponentLocation().ToString());
//    }

//}


void ACutter::OnHit(UPrimitiveComponent *HitComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse, const FHitResult &Hit)
{
    if(bCanCut) //Ignore OnHit if we dont want to cut
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s]: hit %s"), *FString(__FUNCTION__),*OtherComp->GetName());
        TArray<UProceduralMeshComponent*> ProMeshPartsArray;


        Hit.Actor->GetComponents<UProceduralMeshComponent>(ProMeshPartsArray);

        UE_LOG(LogTemp, Log, TEXT("[%s]: The Hit Actor had %d ProMeshComponents"), *FString(__FUNCTION__),ProMeshPartsArray.Num());

        for(int i=0;i<ProMeshPartsArray.Num();i++)
        {
            UE_LOG(LogTemp, Log, TEXT("[%s]: %d ProMeshPartsArray Name: %s at Position: %s"), *FString(__FUNCTION__),i,*ProMeshPartsArray[i]->GetName(),*ProMeshPartsArray[i]->GetComponentLocation().ToString());
        }

        UProceduralMeshComponent* HitProMesh = Cast<UProceduralMeshComponent>(OtherComp);
        if(HitProMesh)
        {
            UMaterialInterface* InsideMaterial = nullptr;
            UProceduralMeshComponent* Otherhalf;
            UE_LOG(LogTemp, Warning, TEXT("[%s]: Hit a ProMesh with Name: %s"), *FString(__FUNCTION__),*HitProMesh->GetName());
            UKismetProceduralMeshLibrary::SliceProceduralMesh(HitProMesh,Hit.ImpactPoint,FVector(1,0,0),true,Otherhalf,EProcMeshSliceCapOption::CreateNewSectionForCap,InsideMaterial); //UMaterialInterface* InsideMaterial
            float radius = Otherhalf->Bounds.SphereRadius;
            // We assume that the Otherhalf is a cube inside of the Sphere --> This means the Height Width and Depths all have the same value --> r*sqrt(2)
            float CubeSize=radius*FGenericPlatformMath::Sqrt(2);
            UE_LOG(LogTemp, Warning, TEXT("[%s]: Created Otherhalf with Name: %s, Postition: %s, SphereRaduis: %f Heigt,Width,Depth of BoundingBox Cube: %f"), *FString(__FUNCTION__),*Otherhalf->GetName(),*Otherhalf->GetComponentLocation().ToString(),radius, CubeSize);
            Otherhalf->SetSimulatePhysics(true);
            Otherhalf->AddImpulse(FVector(1000,1000,1000),FName("None"),true);
        }
    }
}

void ACutter::SetbCanCut(bool InbCanCut)
{
    bCanCut=InbCanCut;
}
bool ACutter::GetbCanCut()
{
    return bCanCut;
}
