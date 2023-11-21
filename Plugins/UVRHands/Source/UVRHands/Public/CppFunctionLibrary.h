// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PythonCallbackContainer.h"
#include "CppFunctionLibrary.generated.h"


DECLARE_MULTICAST_DELEGATE(FTaskCompletedEvent);



UCLASS()
class UVRHANDS_API UCppFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static void StartPython(FString filename, FString funcName);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static void InitPython(FString filename);

	static void TaskCompletedCallback();

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static UPythonCallbackContainer* GetBlueprintCallbackObject();

private:
	static void PythonCall(FString command);

	static TArray<UPythonCallbackContainer*> callbacklist;
	
public:
	static FTaskCompletedEvent OnTaskCompletedEvent;

	
};
