// Copyright 2015-2020 Manus

#pragma once

#include "ManusBlueprintTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "ManusComponent.generated.h"


/**
 * All the live data of one hand of a Manus component.
 */
USTRUCT()
struct MANUS_API FManusComponentHandLiveData
{
	GENERATED_BODY()

	/** Whether the gestures are on-going. */
	UPROPERTY(Transient)
	TArray<bool> IsGestureOnGoing;
};


/**
 * A Skeletal Mesh component animated by a Manus Glove.
 */
UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class MANUS_API UManusComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	UManusComponent(const FObjectInitializer& ObjectInitializer);

	virtual void PostLoad() override;
	virtual void BeginDestroy() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaSeconds, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void RefreshManusSkeleton();
#if WITH_EDITOR
#if ENGINE_MAJOR_VERSION == 5 ||  ENGINE_MINOR_VERSION >= 25
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
#else
	virtual void PreEditChange(UProperty* PropertyAboutToChange) override;
#endif
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR

protected:
	/** Override this method to add the missing Bone names to the overlap results */
	virtual bool ComponentOverlapMultiImpl(TArray<struct FOverlapResult>& OutOverlaps, const class UWorld* InWorld, const FVector& Pos, const FQuat& Rot, ECollisionChannel TestChannel, const struct FComponentQueryParams& Params, const struct FCollisionObjectQueryParams& ObjectQueryParams = FCollisionObjectQueryParams::DefaultObjectQueryParam) const;

public:
	/**
	 * Called when the component is hit.
	 */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/**
	 * Initialize the Manus replicator ID to be used with this Manus component.
	 */
	void InitManusReplicatorID();


private:
	/** Called every frame to update the finger haptics. */
	virtual void TickFingerHaptics();

	/** Called every frame to update the gesture detection. */
	virtual void TickGestureDetection();

	/** Called when a bone got overlapped or hit to add vibration to its corresponding finger haptics. */
	virtual void AddFingerVibration(FName BoneName);


public:
	/** 
	 * Returns the name of the Live Link subject of this Manus component.
	 * @return The name of the Live Link subject of this Manus component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Manus")
	FString GetLiveLinkSubjectName() const;

	/**
	 * Returns the ID of the Manus glove of the given hand used by this Manus component.
	 * @param  HandType     The type of the hand to get the ID from.
	 * @return The ID of the Manus glove of the given hand used by this Manus component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Manus")
	int64 GetManusGloveID(EManusHandType HandType) const;

	/**
	 * Tell the Manus glove of the given hand used by this Manus component to vibrate.
	 * @param HandType 
	 * @param  HandType     The type of the hand to vibrate.
	 * @param  Power        The strength of the vibration, between 0.0 and 1.0.
	 * @param  Milliseconds The number of milliseconds the glove should rumble for.
	 * @return If the glove was succesfully told to vibrate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Manus")
	EManusRet VibrateManusGloveHand(EManusHandType HandType, float Power = 0.6f, int32 Milliseconds = 300);

	/**
	 * Tell the Manus glove of the given hand used by this Manus component to vibrate its fingers. Only works with Haptics Gloves.
	 * @param  HandType		The hand type of the glove to look for.
	 * @param  ThumbPower   The strength of the vibration for the thumb, between 0.0 and 1.0.
	 * @param  IndexPower   The strength of the vibration for the index, between 0.0 and 1.0.
	 * @param  MiddlePower  The strength of the vibration for the middle finger, between 0.0 and 1.0.
	 * @param  RingPower    The strength of the vibration for the ring finger, between 0.0 and 1.0.
	 * @param  PinkyPower   The strength of the vibration for the pinky finger, between 0.0 and 1.0.
	 * @return If the glove fingers were succesfully told to vibrate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Manus")
	EManusRet VibrateManusGloveFingers(EManusHandType HandType, float ThumbPower = 1.0f, float IndexPower = 1.0f, float MiddlePower = 1.0f, float RingPower = 1.0f, float PinkyPower = 1.0f);

	/**
	 * Returns whether this component is in a locally controlled Pawn.
	 * @return whether this component is in a locally controlled Pawn.
	 */
	UFUNCTION(BlueprintCallable, Category = "Manus")
	bool IsLocallyControlled() const;

	/**
	 * Returns whether this component is in a locally owned Pawn.
	 * @return whether this component is in a locally owned Pawn.
	 */
	UFUNCTION(BlueprintCallable, Category = "Manus")
	bool IsLocallyOwned() const;
	

public:
	/** Delegate called every frame for each gesture that just started. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FManusComponentGestureStartedSignature, EManusHandType, HandType, FName, GestureName);
	UPROPERTY(BlueprintAssignable, Category = "Manus|Gestures")
	FManusComponentGestureStartedSignature OnGestureStarted;

	/** Delegate called every frame for each on-going gesture. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FManusComponentGestureOnGoingSignature, EManusHandType, HandType, FName, GestureName, float, Duration);
	UPROPERTY(BlueprintAssignable, Category = "Manus|Gestures")
	FManusComponentGestureOnGoingSignature OnGestureOnGoing;

	/** Delegate called every frame for each gesture that just finished. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FManusComponentGestureFinishedSignature, EManusHandType, HandType, FName, GestureName);
	UPROPERTY(BlueprintAssignable, Category = "Manus|Gestures")
	FManusComponentGestureFinishedSignature OnGestureFinished;

public:
	/** The index of the User as displayed in the Manus Dashboard (first User is index 0).	*/
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Manus", meta = (UIMin = 0))
	int ManusDashboardUserIndex;

	/** The Manus skeleton to use. */
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Manus")
	class UManusSkeleton* ManusSkeleton;

	/** The type of Manus motion capture to use on this component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusMotionCaptureType MotionCaptureType;

	/** Mirror hand bone (useful when you use the same Skeletal Mesh for both hands). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	bool bMirrorHandBone;

	/** Whether to apply finger haptics on the Manus Gloves (only works with Haptic Gloves). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	bool bFingerHaptics;

	/** Vibrate powers coming from collision detection for each finger of the left hand. */
	UPROPERTY(Transient)
	float LeftHandFingersCollisionVibratePowers[(int)EManusFingerName::Max];

	/** Vibrate powers coming from collision detection for each finger of the right hand. */
	UPROPERTY(Transient)
	float RightHandFingersCollisionVibratePowers[(int)EManusFingerName::Max];

	/** ID of the Manus replicator to be used with this Manus component. */
	UPROPERTY(Transient, Replicated)
	int32 ManusReplicatorId;

	/** All the live data of each hand of this Manus component. */
	UPROPERTY(Transient)
	FManusComponentHandLiveData ManusComponentHandLiveData[(int)EManusHandType::Max];
};
