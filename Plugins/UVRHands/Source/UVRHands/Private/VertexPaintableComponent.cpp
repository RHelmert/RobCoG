// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


//Adapted from:
//https://alfredbaudisch.com/experiment-logs/in-game-vertex-painting-with-unreal-engine-wash-car-effect/

#include "VertexPaintableComponent.h"
#include "StaticMeshResources.h"

// Sets default values for this component's properties
UVertexPaintableComponent::UVertexPaintableComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;
}




// Called when the game starts
void UVertexPaintableComponent::BeginPlay()
{
    Super::BeginPlay();
    StaticMeshComponent = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    if (StaticMeshComponent != nullptr && IsValid(StaticMeshComponent))
    {
        InitialiseLODInfoAndBuffers();
        // Notify the render thread about the buffer change
        FStaticMeshComponentLODInfo* InstanceMeshLODInfo = &StaticMeshComponent->LODData[0];
        BeginUpdateResourceRHI(InstanceMeshLODInfo->OverrideVertexColors);
        StaticMeshComponent->MarkRenderStateDirty();
        StaticMeshComponent->CachePaintedDataIfNecessary();
    }
   
    //count all paintable vertices:
    FStaticMeshLODResources& LODModel = StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[0];
    VertexCount = LODModel.GetNumVertices();
    for (auto i = 0; i < VertexCount; i++)
    {
        const FStaticMeshVertexBuffer& VertexBuffer = LODModel.VertexBuffers.StaticMeshVertexBuffer;
        if (i >= 0 && i < static_cast<signed int>(VertexBuffer.GetNumVertices()))
        {
            // Get the vertex normal
            FVector VertexNormal = VertexBuffer.VertexTangentZ(i);
            //Count all paintable vertices within an error margin
            FVector up(0.0f, 1.0f, 0.0f);
            if (VertexNormal.Equals(up, 0.1f)) {
                PaintableVertexCount++;
                
            }
        }
        
    }
    UE_LOG(LogTemp, Warning, TEXT("Full paintable Vertext count for cleanable: %d"), PaintableVertexCount);

}
void UVertexPaintableComponent::PaintVertexAtLocation(FVector HitLocation, float PaintLerpProgress)
{
    // Init the buffers and LOD data
    InitialiseLODInfoAndBuffers();
    FStaticMeshComponentLODInfo* InstanceMeshLODInfo = &StaticMeshComponent->LODData[0];

    FStaticMeshLODResources& LODModel = StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[0];
    auto LocalToWorld = StaticMeshComponent->GetComponentToWorld().ToMatrixWithScale();
    
    for (auto i = 0; i < VertexCount; i++)
    {
        auto LocalVertexPosition = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(i);
        auto WorldVertexPosition = LocalToWorld.TransformPosition(FVector4(LocalVertexPosition.X, LocalVertexPosition.Y, LocalVertexPosition.Z, 1.0f));
        auto Distance = FVector::DistSquared(WorldVertexPosition, HitLocation);
        
        if (Distance <= BrushSize)
        {
            FLinearColor from = FLinearColor(InstanceMeshLODInfo->OverrideVertexColors->VertexColor(i));
            FLinearColor to = FLinearColor(TargetBaseColor);

            //why does this work (shouldnt it be from != to?)
            if (from == to) {
                // Get the vertex buffer for the LOD model
                
                const FStaticMeshVertexBuffer& VertexBuffer = LODModel.VertexBuffers.StaticMeshVertexBuffer;
                if (i >= 0 && i < VertexCount)
                {
                    // Get the vertex normal
                    FVector VertexNormal = VertexBuffer.VertexTangentZ(i);

                    //Only consider normals similar to the up vector within an error margin
                    FVector up(0.0f, 1.0f, 0.0f);
                    if (VertexNormal.Equals(up,0.1f)){
                        VertexProgression++;
                        UE_LOG(LogTemp, Warning, TEXT("Progression count: %d"), VertexProgression);
                    }
                }
            }

            FColor& currentColor = InstanceMeshLODInfo->OverrideVertexColors->VertexColor(i);
            currentColor = FLinearColor::LerpUsingHSV(from, to, PaintLerpProgress).ToFColor(false);
        }
    }
    // Notify the render thread about the buffer change
    BeginUpdateResourceRHI(InstanceMeshLODInfo->OverrideVertexColors);
    StaticMeshComponent->MarkRenderStateDirty();
#if WITH_EDITORONLY_DATA
    StaticMeshComponent->CachePaintedDataIfNecessary();
#endif
}
int32 UVertexPaintableComponent::GetNearestVertIndex(FVector Position, FStaticMeshLODResources& LODModel)
{
    auto ShortestDistance = 0;
    auto NearestVertexIndex = -1;
    auto LocalToWorld = StaticMeshComponent->GetComponentToWorld().ToMatrixWithScale();
    for (auto i = 0; i < LODModel.GetNumVertices(); i++)
    {
        auto LocalVertexPosition = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(i);
        auto WorldVertexPosition = LocalToWorld.TransformPosition(FVector4(LocalVertexPosition.X, LocalVertexPosition.Y, LocalVertexPosition.Z, 1.0f));
        auto Distance = FVector::DistSquared(WorldVertexPosition, Position);
        if (Distance < ShortestDistance || ShortestDistance < 0)
        {
            ShortestDistance = Distance;
            NearestVertexIndex = i;
        }
    }
    return NearestVertexIndex;
}
uint8 UVertexPaintableComponent::GetIntensity(FColor Color)
{
    return uint8();
}
void UVertexPaintableComponent::SetIntensity(FColor* Color, uint8 Intensity)
{
}
void UVertexPaintableComponent::InitialiseLODInfoAndBuffers()
{
    if (StaticMeshComponent->LODData.Num() == 0)
    {
        StaticMeshComponent->SetLODDataCount(1, StaticMeshComponent->LODData.Num());
    }
    FStaticMeshLODResources& LODModel = StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[0];
    FStaticMeshComponentLODInfo* InstanceMeshLODInfo = &StaticMeshComponent->LODData[0];
    if (InstanceMeshLODInfo->OverrideVertexColors == nullptr)
    {
        InitialiseInstancedOverrideVertexColorBuffer(InstanceMeshLODInfo, LODModel);
    }
}
void UVertexPaintableComponent::InitialiseInstancedOverrideVertexColorBuffer(FStaticMeshComponentLODInfo* InstanceMeshLODInfo, FStaticMeshLODResources& LODModel)
{
    // Check that we don't already have an overridden vertex color buffer
    check(InstanceMeshLODInfo->OverrideVertexColors == nullptr);
    // Create a new buffer
    InstanceMeshLODInfo->OverrideVertexColors = new FColorVertexBuffer;

    if ((int32)LODModel.VertexBuffers.ColorVertexBuffer.GetNumVertices() >= LODModel.GetNumVertices())
    {
        // If the mesh already has vertex colours, initialise OverrideVertexColors from them
        InstanceMeshLODInfo->OverrideVertexColors->InitFromColorArray(&LODModel.VertexBuffers.ColorVertexBuffer.VertexColor(0), LODModel.GetNumVertices());
    }
    else
    {
        // If it doesn't, set all overridden vert colours to black
        InstanceMeshLODInfo->OverrideVertexColors->InitFromSingleColor(TargetBaseColor, LODModel.GetNumVertices());
    }
    for (auto i = 0; i < LODModel.GetNumVertices(); i++)
    {
        if (InstanceMeshLODInfo->OverrideVertexColors->VertexColor(i) != TargetBaseColor)
        {
            // TODO: add vertex idx to the dirt list
            // TODO: save amount of dirt vertexes, to keep track of % cleaned/dirt
        }
    }
    BeginInitResource(InstanceMeshLODInfo->OverrideVertexColors);
}

