// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "ManusGestureDetector.h"
//#include "CoreSdk.h"


// Sets default values for this component's properties
UManusGestureDetector::UManusGestureDetector()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UManusGestureDetector::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UManusGestureDetector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


bool UManusGestureDetector::CheckForGesture(GestureType gType)
{
	if (isInit) {
		switch (gType)
		{
		case GestureType::ThumbsUp:
			return CheckForThumbsup();
			break;
		default:
			return false;

		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CheckForGestureCalled while the class is not initialized yet"));
		return false;
	}

	
}

//Debug function to see the relevant data
TArray<FTransform> UManusGestureDetector::GetBoneData()
{
	FManusMetaSkeleton p_Skeleton;
	
	
	//Does not work since including CoreSDK causes errors in other includes

	//CoreSdk::GetDataForSkeleton(manusHand.GetManusSkeletonId(), p_Skeleton);
	//TArray<FManusBone> bones = p_Skeleton.Bones;
	
	
	
	//middle index finger
	//boneTransforms.Add(boneData[2].Transform);

	//middle middle finger
	//boneTransforms.Add(boneData[5].Transform);

	//middle pinky
	//boneTransforms.Add(boneData[8].Transform);

	//middle ring
	//boneTransforms.Add(boneData[11].Transform);

	//all three thumb bones
	//boneTransforms.Add(boneData[13].Transform);
	//boneTransforms.Add(boneData[14].Transform);
	//boneTransforms.Add(boneData[15].Transform);


	//directly return the relevant boneData
	//return TArray<FTransform>{bones[2].Transform, bones[5].Transform, bones[8].Transform, bones[11].Transform, bones[13].Transform, bones[14].Transform, bones[15].Transform};
	return TArray<FTransform>{};
}

bool UManusGestureDetector::CheckForThumbsup()
{
	//Check for the middle bones of each finger, exept from the thumb

	//if (boneData[])


	return false;
}

