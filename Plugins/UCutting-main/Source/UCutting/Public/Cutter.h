// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cutter.generated.h"

//Quick and dirty Cutter Implementation --> For a better Solution look into Collision Channels
UCLASS()
class UCUTTING_API ACutter : public AActor
{
 GENERATED_BODY()

    /** Sphere collision component */
//    UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
//    class USphereComponent* CollisionComp;

    UPROPERTY(EditAnywhere, Category=Projectile)
    class UBoxComponent* CollisionComp;

    UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
    class UStaticMeshComponent* StaticMesh;


public:
    ACutter();

//    // Called every frame
//    virtual void Tick(float DeltaTime) override;

    /** called when projectile hits something */
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

//    /** Returns CollisionComp subobject **/
//    FORCEINLINE class USphereComponent* GetCollisionComp() const { return CollisionComp; }

    /** Returns CollisionComp subobject **/
    FORCEINLINE class UBoxComponent* GetCollisionComp() const { return CollisionComp; }

    void SetbCanCut(bool InbCanCut);
    bool GetbCanCut();

private:
    class UProceduralMeshComponent* OtherHalf;
protected:
    bool bCanCut;
};
