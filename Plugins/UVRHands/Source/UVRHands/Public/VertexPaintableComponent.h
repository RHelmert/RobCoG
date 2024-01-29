// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


//Adapted from:
//https://alfredbaudisch.com/experiment-logs/in-game-vertex-painting-with-unreal-engine-wash-car-effect/


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VertexPaintableComponent.generated.h"


/**
Channel used for spreading vertex colors.
*/
UENUM()
namespace AVertexColorSpreadChannel
{
	enum Channel
	{
		Red,
		Green,
		Blue,
		Alpha,
	};
}


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UVRHANDS_API UVertexPaintableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrushSize = 150.0f;

	UPROPERTY(BlueprintReadOnly)
	int VertexProgression;

	UPROPERTY(BlueprintReadOnly)
	int PaintableVertexCount;

	int VertexCount;

	TArray<FLinearColor> BaseColors;
	TMap<int, FLinearColor> ChangedVertices;

	// Sets default values for this component's properties
	UVertexPaintableComponent();

	UFUNCTION(BlueprintCallable, Category = VertexPainting)
		void PaintVertexAtLocation(FVector HitLocation, float PaintLerpProgress = 1.0f);

	UFUNCTION(BlueprintCallable, Category = VertexPainting)
		void ResetAll();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = VertexPainting)
		FColor TargetBaseColor = FColor::White;



protected:

	

	// Called when the game starts
	virtual void BeginPlay() override;
	/**
	Create LOD info entries, and ensure the override vertex color buffer exists.
	*/
	void InitialiseLODInfoAndBuffers();
	/**
	Create a new override vertex color buffer.
	*/
	void InitialiseInstancedOverrideVertexColorBuffer(FStaticMeshComponentLODInfo* InstanceMeshLODInfo,
		FStaticMeshLODResources& LODModel);

	/**
	Get the intensity of the selected channel (see Channel)
	*/
	int32 GetNearestVertIndex(FVector Position, FStaticMeshLODResources& LODModel);
	/**
	Get the intensity of the selected channel (see Channel)
	*/
	uint8 GetIntensity(FColor Color);
	/**
	Set the intensity of the selected channel to the given value (see Channel)
	*/
	void SetIntensity(FColor* Color, uint8 Intensity);
	UPROPERTY(BlueprintReadOnly)
		UStaticMeshComponent* StaticMeshComponent;

		
};
