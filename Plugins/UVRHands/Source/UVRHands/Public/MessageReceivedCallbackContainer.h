// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MessageReceivedCallbackContainer.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageReceivedBP,FString,message);
DECLARE_MULTICAST_DELEGATE_OneParam(FMessageReceived, FString);

UCLASS()
class UVRHANDS_API UMessageReceivedCallbackContainer : public UObject
{
	GENERATED_BODY()
	
public:
	UMessageReceivedCallbackContainer() {};

	UPROPERTY(BlueprintAssignable)
		FMessageReceivedBP OnMessageReceivedEventBP;

		FMessageReceived OnMessageReceivedEvent;
};
