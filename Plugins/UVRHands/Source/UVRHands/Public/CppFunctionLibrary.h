// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PythonCallbackContainer.h"
#include "zmq.hpp"
#include "CppFunctionLibrary.generated.h"



USTRUCT(BlueprintType)
struct FMethodJson {
	GENERATED_USTRUCT_BODY()

public:

	//"method_name" , "method_args" und "method_kwargs"
	UPROPERTY(BlueprintReadWrite)
		FString method_name;
	UPROPERTY(BlueprintReadWrite)
		TArray<FString> method_args;
	
		//TArray<FString> method_kwargs;
	UPROPERTY(BlueprintReadWrite)
		TMap<FString, FString> method_kwargs;

	FMethodJson() {};

	FMethodJson(FString name, TArray<FString> args, TMap<FString, FString> kwargs) {
		method_name = name;
		method_args = args;
		method_kwargs = kwargs;
	}

public:
	FString ToString() {
		FString str;
		str.Append("Name: ");
		str.Append(*method_name);
		str.Append("\n");

		str.Append(" args: ");
		for (FString s : method_args){
			str.Append(*s);
			str.Append(", ");
		}
		str.Append("\n");

		str.Append(" kwargs: ");
		for (TPair<FString,FString> s : method_kwargs) {
			str.Append(*s.Key);
			str.Append(": ");
			str.Append(*s.Value);
			str.Append(", ");
		}
		return str;
	}
	
};

DECLARE_MULTICAST_DELEGATE(FTaskCompletedEvent);

UCLASS()
class UVRHANDS_API UCppFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static void StartPython(FString filename, FString funcName);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static void StartPythonAlternate(FString filename, FString funcName);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static void InitPython(FString filename);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static void StartZmQServer();

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static void StartZmQClient();

	static void TaskCompletedCallback();

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static void Test();


	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static FString StructToJsonString(FMethodJson obj);

	static TSharedPtr<FJsonObject> StructToJsonObj(FMethodJson obj);
	static FMethodJson JsonObjToStruct(TSharedPtr<FJsonObject> obj);
	static TSharedPtr<FJsonObject> JsonStringToJsonObj(FString Json, bool& success, FString& OutInfoMessage);
	static FString JsonObjectToString(TSharedPtr<FJsonObject> JsonObject, bool& success, FString& OutInfoMessage);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static UPythonCallbackContainer* GetBlueprintCallbackObject();

private:
	static void PythonCall(FString command);

	static FString ConvertMessageToString(const zmq::message_t& zmqMessage);

	static TArray<UPythonCallbackContainer*> callbacklist;
	
public:
	static FTaskCompletedEvent OnTaskCompletedEvent;

	
};
