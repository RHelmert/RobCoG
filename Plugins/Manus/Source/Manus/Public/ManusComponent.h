// Copyright 2015-2022 Manus

#pragma once

// Set up a Doxygen group.
/** @addtogroup ManusComponent
 *  @{
 */


#include "ManusBlueprintTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "ManusComponent.generated.h"

/// @brief A Skeletal Mesh component animated by a Manus Glove.
UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class MANUS_API UManusComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
    /// @brief
	UManusComponent(const FObjectInitializer& ObjectInitializer);
    /// @brief
	virtual void PostLoad() override;
    /// @brief
	virtual void BeginDestroy() override;
    /// @brief
	virtual void BeginPlay() override;
    /// @brief
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    /// @brief
	virtual void TickComponent(float DeltaSeconds, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    /// @brief
	virtual void RefreshManusSkeleton();

#if WITH_EDITOR
    /// @brief
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
    /// @brief
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR

protected:
    /// @brief Override this method to add the missing Bone names to the overlap results 
	virtual bool ComponentOverlapMultiImpl(TArray<struct FOverlapResult>& OutOverlaps, const class UWorld* InWorld, const FVector& Pos, const FQuat& Rot, ECollisionChannel TestChannel, const struct FComponentQueryParams& Params, const struct FCollisionObjectQueryParams& ObjectQueryParams = FCollisionObjectQueryParams::DefaultObjectQueryParam) const;

public:
	/// @brief Called when the component is hit.
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    /// @brief Initialize the Manus replicator ID to be used with this Manus component.
	void InitManusReplicatorID();

public:
    /// @brief get the skeleton id
    UFUNCTION(BlueprintCallable, Category = "Manus")
    int GetManusSkeletonId() const;

    /// @brief Returns the name of the Live Link subject of this Manus component.
	UFUNCTION(BlueprintCallable, Category = "Manus")
	FString GetLiveLinkSubjectName() const;

    /// @brief Tell the Manus glove of the given hand used by this Manus component to vibrate its fingers. Only works with Haptics Gloves.
	/// @param  HandType		The hand type of the glove to look for.
	/// @param  ThumbPower   The strength of the vibration for the thumb, between 0.0 and 1.0.
	/// @param  IndexPower   The strength of the vibration for the index, between 0.0 and 1.0.
	/// @param  MiddlePower  The strength of the vibration for the middle finger, between 0.0 and 1.0.
	/// @param  RingPower    The strength of the vibration for the ring finger, between 0.0 and 1.0.
	/// @param  PinkyPower   The strength of the vibration for the pinky finger, between 0.0 and 1.0.
	/// @return If the glove fingers were succesfully told to vibrate.
	UFUNCTION(BlueprintCallable, Category = "Manus")
	EManusRet VibrateManusGloveFingers(EManusHandType HandType, float ThumbPower = 1.0f, float IndexPower = 1.0f, float MiddlePower = 1.0f, float RingPower = 1.0f, float PinkyPower = 1.0f);

    /// @brief Tell the Manus glove of the given hand used by this Manus component to vibrate its fingers. Only works with Haptics Gloves.
    /// @param  SkeletonId	The skeleton id of the glove to look for.
	/// @param  HandType		The hand type of the glove to look for.
	/// @param  ThumbPower   The strength of the vibration for the thumb, between 0.0 and 1.0.
	/// @param  IndexPower   The strength of the vibration for the index, between 0.0 and 1.0.
	/// @param  MiddlePower  The strength of the vibration for the middle finger, between 0.0 and 1.0.
	/// @param  RingPower    The strength of the vibration for the ring finger, between 0.0 and 1.0.
	/// @param  PinkyPower   The strength of the vibration for the pinky finger, between 0.0 and 1.0.
	/// @return If the glove fingers were succesfully told to vibrate.
	UFUNCTION(BlueprintCallable, Category = "Manus")
	EManusRet VibrateManusGloveFingersForSkeleton(int64 SkeletonId, EManusHandType HandType, float ThumbPower = 1.0f, float IndexPower = 1.0f, float MiddlePower = 1.0f, float RingPower = 1.0f, float PinkyPower = 1.0f);


    /// @brief Returns whether this component is in a locally controlled Pawn.
	UFUNCTION(BlueprintCallable, Category = "Manus")
	bool IsLocallyControlled() const;

    /// @brief Returns whether this component is in a locally owned Pawn.
	UFUNCTION(BlueprintCallable, Category = "Manus")
	bool IsLocallyOwned() const;	

public:
    /// @brief Delegate called every frame for each gesture that just started. 
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FManusComponentGestureStartedSignature, EManusHandType, HandType, FName, GestureName);
	UPROPERTY(BlueprintAssignable, Category = "Manus|Gestures")
	FManusComponentGestureStartedSignature OnGestureStarted;

    /// @brief Delegate called every frame for each on-going gesture. 
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FManusComponentGestureOnGoingSignature, EManusHandType, HandType, FName, GestureName, float, Duration);
	UPROPERTY(BlueprintAssignable, Category = "Manus|Gestures")
	FManusComponentGestureOnGoingSignature OnGestureOnGoing;

    /// @brief Delegate called every frame for each gesture that just finished. 
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FManusComponentGestureFinishedSignature, EManusHandType, HandType, FName, GestureName);
	UPROPERTY(BlueprintAssignable, Category = "Manus|Gestures")
	FManusComponentGestureFinishedSignature OnGestureFinished;

public:

    /// @brief The Manus skeleton to use. 
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Manus")
	class UManusSkeleton* ManusSkeleton;

    /// @brief Whether to apply finger haptics on the Manus Gloves (only works with Haptic Gloves). 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	bool bFingerHaptics;

    /// @brief Vibrate powers coming from collision detection for each finger of the left hand. 
	UPROPERTY(Transient)
	float LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Max];

    /// @brief Vibrate powers coming from collision detection for each finger of the right hand. 
	UPROPERTY(Transient)
	float RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Max];

    /// @brief ID of the Manus replicator to be used with this Manus component. 
	UPROPERTY(Transient, Replicated)
	int32 ManusReplicatorId;

};

// Close the Doxygen group.
/** @} */
