// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "StrenghAdjustment.h"
#include "EngineUtils.h"


// Sets default values for this component's properties
UStrenghAdjustment::UStrenghAdjustment()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// ...
}


// Called when the game starts
void UStrenghAdjustment::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle UnusedHandle;
	GetOwner()->GetWorldTimerManager().SetTimer(
		UnusedHandle, this, &UStrenghAdjustment::Init, 0.1, false);
	// ...
	
}


void UStrenghAdjustment::AddConstraint(UPhysicsConstraintComponent* constraint, USceneComponent* parent, USceneComponent* child) {
	SphereConstrains.Add(constraint);
	Parents.Add(parent);
	Childs.Add(child);
}

void UStrenghAdjustment::SetFlattenDistance(float d)
{
	triggerdistance = d;
}

void UStrenghAdjustment::SetSpringForce(float d)
{
	springforce = d;
}

void UStrenghAdjustment::SetDampeningTarget(float d)
{
	dampening = d;
}

void UStrenghAdjustment::Init() {
	
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

// Called every frame
void UStrenghAdjustment::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (SphereConstrains.Num() != 0) {
		UE_LOG(LogTemp, Warning, TEXT("Count: %d %d Positions: Child %s , Parent: %s"), Parents.Num(), Childs.Num(), *(*Childs[0]).GetComponentLocation().ToString(), *(*Parents[0]).GetComponentLocation().ToString());
	}
	


	for (int i = 0; i < SphereConstrains.Num(); i++) {
		float dist = FVector::Dist((*Childs[i]).GetComponentLocation(), (*Parents[i]).GetComponentLocation());
		
		//FTransform children = Childs[i];
		//FVector location = children.GetLocation();
		UE_LOG(LogTemp, Warning, TEXT("Dist: %f"), Childs.Num(), dist);

		//UE_LOG(LogTemp, Error, TEXT("Distance is = %d, Strength set to %d"), dist, 1000 / (dist / 5));
		if (dist > triggerdistance) {
			SphereConstrains[i]->SetLinearDriveParams(springforce / (dist / triggerdistance), dampening, 10000);
		}
	}
}

void UStrenghAdjustment::TestChangeStrength(float Strength, float Dampening)
{
	for (int i = 0; i < SphereConstrains.Num(); i++) {

		SphereConstrains[i]->SetLinearDriveParams(Strength, Dampening,10000);

		//float dist = FVector::Dist(Childs[i].GetLocation(), Parents[i].GetLocation());
		//if (dist > 5) {
		//	SphereConstrains[i]->SetLinearDriveParams(1000 / (dist / 5), 1000, 10000);
		//}
	}

}

