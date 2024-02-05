// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PythonCallbackContainer.h"
#include "MessageReceivedCallbackContainer.h"
#include "zmq.hpp"
#include <ZmqClient.h>
#include <ZmqServer.h>
#include "CppFunctionLibrary.generated.h"



USTRUCT(BlueprintType)
struct FMethodJson {
	GENERATED_USTRUCT_BODY()

public:

	//"method_name" , "method_args" und "method_kwargs"
	UPROPERTY(BlueprintReadWrite)
	FString method_name;
	UPROPERTY(BlueprintReadWrite)
	FString method_docstring;
	
	//TArray<FString> method_kwargs;
	UPROPERTY(BlueprintReadWrite)
	TMap<FString, FString> method_kwargs;

	UPROPERTY(BlueprintReadWrite)
	TMap<FString, FString> kwargs_type;

	FMethodJson() {};

	FMethodJson(FString name,FString docstring, TMap<FString, FString> kwargs, TMap<FString, FString> types) {
		method_name = name;
		method_docstring = docstring;
		method_kwargs = kwargs;
		kwargs_type = types;

	}

	FMethodJson(FString name, TMap<FString, FString> kwargs) {
		method_name = name;
		method_kwargs = kwargs;
	}


public:
	FString ToString() {
		FString str;
		str.Append("Name: ");
		str.Append(*method_name);
		str.Append("\n");
		str.Append("Doc: ");
		str.Append(*method_docstring);
		str.Append("\n");

		str.Append("kwargs: ");
		for (TPair<FString,FString> s : method_kwargs) {
			str.Append(*s.Key);
			str.Append(": ");
			str.Append(*s.Value);
			str.Append(", ");
		}
		return str;
	}
	
};

USTRUCT(BlueprintType)
struct FMethodJsonArray {
	GENERATED_USTRUCT_BODY()
public:
	//"method_name" , "method_args" und "method_kwargs"
	UPROPERTY(BlueprintReadWrite)
	TArray<FMethodJson> methods;

	FMethodJsonArray() {}
	FMethodJsonArray(TArray<FMethodJson> mArray) {
		methods = mArray;
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
	static void InitPython(FString filename);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static bool StartZmQServer(FString Port);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static void StopZmQServer(FString Port);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static bool StartZmQClient(FString Ip_Port);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static bool StopZmQClient(FString Ip_Port);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static void SendZmqMessageOverClient(FString Ip_Port,FString Message);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static int GetNumberClients();

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static int GetNumberServers();



	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static void StartAsyncCalculations();


	static void TaskCompletedCallback();

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static void Test();


	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static FString StructToJsonString(FMethodJson obj);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static FString StructToTypedJsonString(FMethodJson obj);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static FMethodJson JsonStringToStruct(FString jsonString);
	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static FString FMethodJsonToFancyString(FMethodJson struc);
	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static TArray<FMethodJson> JsonToArrayOfStructs(FString jsonString, bool& bOut);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static bool JsonStringToInspectibleMap(FString jsonString, TMap<FString,FMethodJsonArray> &outMap, FString& outString);

	static TSharedPtr<FJsonObject> StructToJsonObj(FMethodJson obj);
	static FMethodJson JsonObjToStruct(TSharedPtr<FJsonObject> obj);
	static TSharedPtr<FJsonObject> JsonStringToJsonObj(FString Json, bool& success, FString& OutInfoMessage);
	static FString JsonObjectToString(TSharedPtr<FJsonObject> JsonObject, bool& success, FString& OutInfoMessage);

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static UPythonCallbackContainer* GetBlueprintPythonCallbackObject();

	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static UMessageReceivedCallbackContainer* CreateOrGetBlueprintZmqServerCallbackObject(FString Port);
	UFUNCTION(BlueprintCallable, Category = "BPLibrary")
	static UMessageReceivedCallbackContainer* CreateOrGetBlueprintZmqClientCallbackObject(FString Ip_Port);

private:
	static void PythonCall(FString command);
	static UPythonCallbackContainer* pythonCallback;
	static TMap<FString, UMessageReceivedCallbackContainer*> clientCallbacks;
	static TMap<FString, UMessageReceivedCallbackContainer*> serverCallbacks;
	
public:
	static TMap<FString, ZmqClient*> clients;
	static TMap<FString, ZmqServer*> servers;
	static FTaskCompletedEvent OnTaskCompletedEvent;
	

	
};
