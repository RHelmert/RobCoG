// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen
// @Author: Robin Helmert (rhelmert2@gmail.com)


#include "CppFunctionLibrary.h"
#include "IPythonScriptPlugin.h"
#include "Runtime/Core/Public/Features/IModularFeatures.h"
#include "zmq.hpp"
#include <string>
#include <future>
#include "Serialization/JsonSerializer.h"
#include <JsonUtilities/Public/JsonObjectConverter.h>
#include <DefaultValueHelper.h>



FTaskCompletedEvent UCppFunctionLibrary::OnTaskCompletedEvent;
UPythonCallbackContainer* UCppFunctionLibrary::pythonCallback;
TMap<FString,UMessageReceivedCallbackContainer*> UCppFunctionLibrary::serverCallbacks;
TMap<FString, UMessageReceivedCallbackContainer*> UCppFunctionLibrary::clientCallbacks;
TMap<FString,ZmqServer*> UCppFunctionLibrary::servers;
TMap<FString,ZmqClient*> UCppFunctionLibrary::clients;




void UCppFunctionLibrary::StartPython(FString filename, FString funcName)
{

	// Run the Python function call (NOTE: It has to loaded prior via init using the same filename)

	//remove ".py" 
	if (filename.EndsWith(".py")) {
		filename.LeftChopInline(3);
	}

	FString file_func = filename + "." +funcName;
	
	//Task to run asynchronosly
	TUniqueFunction<int32()> RTask = [st = file_func]() -> int32 {
		UCppFunctionLibrary::PythonCall(st);
		pythonCallback->OnTaskCompletedEvent.Broadcast();
		OnTaskCompletedEvent.Broadcast();
		return 1; 
	};
	auto Result = Async(EAsyncExecution::Thread, MoveTemp(RTask));
	
	//Not used atm but can check whether the returnvalue of the task has been set and if so it can be read
	if (Result.IsReady()) {
		int32 funcresult = Result.Get();
		UE_LOG(LogTemp, Warning, TEXT("Return is Ready : %d"), funcresult);
	}


}

void UCppFunctionLibrary::InitPython(FString filename)
{

	//Init events
	if (!OnTaskCompletedEvent.IsBound()) {
		OnTaskCompletedEvent.AddStatic(&UCppFunctionLibrary::TaskCompletedCallback);
	}
	if (pythonCallback == nullptr) {
		pythonCallback = (NewObject<UPythonCallbackContainer>());
	}



	//filename is expected to be like abc.py but we do not need the ".py"
	if (filename.EndsWith(".py")) {
		filename.LeftChopInline(3);
	}

	//Project Directory
	FString ProjectDirectory = FPaths::ProjectDir();
	//Folder in the project
	ProjectDirectory += "PythonScripting/";


	//Form a correct python command
	FString myString = "import sys";
	myString += "\n";
	myString += "sys.path.append(\"";
	myString += ProjectDirectory;
	myString += "\")";
	myString += "\n";
	myString += "import ";
	myString += filename;
	myString += "\n";
	myString += "import importlib";
	myString += "\n";
	myString += "importlib.reload(hello)";
	myString += "\n";

	//Execute that command such that hello.py is loaded
	IPythonScriptPlugin::Get()->ExecPythonCommand(*myString);
	UE_LOG(LogTemp, Warning, TEXT("Line after Task \n%s"), *myString);
}

bool  UCppFunctionLibrary::StartZmQServer(FString Port)
{

	if (servers.Contains(Port)) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d ZmqServer is already running...."), *FString(__func__), __LINE__);
		return false;
	}

	// Creates and starts a new zmq server with the callback object on which the events are called
	UCppFunctionLibrary::CreateOrGetBlueprintZmqServerCallbackObject(Port);
	servers.Add(Port, new ZmqServer(serverCallbacks.FindChecked(Port)));
	return true;
	
}

void UCppFunctionLibrary::StopZmQServer(FString Port)
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d Currently Not Implemented since it was never neccessary"), *FString(__func__), __LINE__);
}

bool UCppFunctionLibrary::StartZmQClient(FString Ip_Port)
{

	if (clients.Contains(Ip_Port)) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d ZmqClient is already running...."), *FString(__func__), __LINE__);
		return false;
	}
	// Creates and starts a new zmq server with the callback object on which the events are called
	UCppFunctionLibrary::CreateOrGetBlueprintZmqClientCallbackObject(Ip_Port);
	clients.Add(Ip_Port, new ZmqClient(clientCallbacks.FindChecked(Ip_Port),Ip_Port));
	return true;
}

bool UCppFunctionLibrary::StopZmQClient(FString Ip_Port)
{
	if (clients.Contains(Ip_Port)) {
		clients.FindChecked(Ip_Port)->Stop();
		clients.Remove(Ip_Port);
		clientCallbacks.Remove(Ip_Port);
		return true;
	}
	return false;
}



void UCppFunctionLibrary::SendZmqMessageOverClient(FString Ip_Port,FString Message)
{
	if (!clients.Contains(Ip_Port)) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d ZmqClient is not running...."), *FString(__func__), __LINE__);
		return;
	}
	clients.FindChecked(Ip_Port)->EnqueueMessage(Message);

}

int UCppFunctionLibrary::GetNumberClients()
{
	return clients.Num();
}

int UCppFunctionLibrary::GetNumberServers()
{
	return servers.Num();
}

void UCppFunctionLibrary::StartAsyncCalculations()
{
	//1.Create a function with content
	TUniqueFunction<int32()> AsyncTask = []() -> int32 {
		//Do Calculations
			return 1;
	};

	//2. Execute The function async // Result is a TFuture<int32>
	auto Result = Async(EAsyncExecution::Thread, MoveTemp(AsyncTask));
}




//Callback when task is completed
void UCppFunctionLibrary::TaskCompletedCallback()
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Python Task Completed"), *FString(__func__), __LINE__);
}



//Testfunction for making JSons and working with them
void UCppFunctionLibrary::Test()
{
	//create the struct
	FMethodJson st;
	st.method_name = "method";
	st.method_kwargs.Add("in1","value1");
	st.method_kwargs.Add("in2","value2");
	st.kwargs_type.Add("in1", "int");
	st.kwargs_type.Add("in2", "string");
	UE_LOG(LogTemp, Warning, TEXT("Struct made: %s"),*st.ToString());


	//												struct -->json obj --> struct
	//Struct to JsonObject
	TSharedPtr<FJsonObject> jsonObj = StructToJsonObj(st);
	//Json Object to Struct
	FMethodJson revJson = JsonObjToStruct(jsonObj);
	UE_LOG(LogTemp, Warning, TEXT("Struct made: %s"), *revJson.ToString());


	//------------Alternate  with String            struct ->json obj --> string --> json obj --> struct
	//Struct to JsonObject
	jsonObj = StructToJsonObj(st);

	//JsonObject to String
	bool succ;
	FString outInfo;
	FString jsonString = JsonObjectToString(jsonObj, succ, outInfo);
	UE_LOG(LogTemp, Warning, TEXT("Json made: %s"), *jsonString);

	//and back ...
	//Json String to JsonObject
	TSharedPtr<FJsonObject> newJsonObj = JsonStringToJsonObj(jsonString, succ, outInfo);

	//Json Object Back to Struct
	FMethodJson revJson2 = JsonObjToStruct(newJsonObj);
	UE_LOG(LogTemp, Warning, TEXT("Struct made: %s"), *revJson2.ToString());

}


//FMethodJson(struct) ---> JsonString. Necessary for BPs since they can not work wit TSharedPtr
FString UCppFunctionLibrary::StructToJsonString(FMethodJson obj)
{
	TSharedPtr<FJsonObject> jsonObj = UCppFunctionLibrary::StructToJsonObj(obj);
	bool success;
	FString message;
	return UCppFunctionLibrary::JsonObjectToString(jsonObj, success, message);
}


FString UCppFunctionLibrary::StructToTypedJsonString(FMethodJson obj)
{
	TSharedPtr<FJsonObject> jsonObj = UCppFunctionLibrary::StructToJsonObj(obj);
	FJsonObject* jobj = jsonObj.Get();
	/*const TArray<TSharedPtr<FJsonValue>> *kwargmap;
	if (jobj->TryGetArrayField("method_kwargs", kwargmap)) {*/
		//TArray<TSharedPtr<FJsonValue>> kwargmap = jobj->GetArrayField("method_kwargs");

	

	//Iterate over all parameters
	for (const auto& pair : obj.kwargs_type) {
		const FString& Key = pair.Key;
		const FString& Type = pair.Value;

		//Replace Strings that can be represented as a number with the number
		if (Type == "int" || Type == "float") {
			float NumericValue;
			FDefaultValueHelper::ParseFloat(*obj.method_kwargs.Find(Key), NumericValue);
			jobj->GetObjectField("method_kwargs")->SetNumberField(Key, NumericValue);
			continue;
		}

		//Replace bool Strings with the actual bool value

		if (Type.ToLower().Equals("bool")) {
			jobj->GetObjectField("method_kwargs")->SetBoolField(Key, obj.method_kwargs.Find(Key)->ToBool());
			continue;
		}


	}
	
	//Remove Kwarg types from the message 
	jobj->RemoveField("kwargs_type");
	bool success;
	FString message;
	return UCppFunctionLibrary::JsonObjectToString(jsonObj, success, message);

}
	

//JsonString to FMethodJson. Necessary for BPs since they can not work wit TSharedPtr
FMethodJson UCppFunctionLibrary::JsonStringToStruct(FString jsonString)
{
	bool success;
	FString message;
	TSharedPtr<FJsonObject> jsonObj = UCppFunctionLibrary::JsonStringToJsonObj(jsonString, success, message);
	return UCppFunctionLibrary::JsonObjToStruct(jsonObj);

}

FString UCppFunctionLibrary::FMethodJsonToFancyString(FMethodJson struc)
{
	return struc.ToString();
}

TArray<FMethodJson> UCppFunctionLibrary::JsonToArrayOfStructs(FString jsonString,bool &bOut)
{
	TArray<FMethodJson> methodArray;
	bOut = FJsonObjectConverter::JsonArrayStringToUStruct<FMethodJson>(jsonString, &methodArray);
	return methodArray;
}

bool UCppFunctionLibrary::JsonStringToInspectibleMap(FString jsonString, TMap<FString, FMethodJsonArray>& outMap, FString& outString)
{
	//UE_LOG(LogTemp, Warning, TEXT("JsonString: %s"), *jsonString);
	//TMap<FString, TArray<FMethodJson>> outMethodMap;
	bool bStringReturn;
	outMap.Empty();
	FString returnMessage;
	TSharedPtr<FJsonObject> jsonObj = JsonStringToJsonObj(jsonString, bStringReturn, returnMessage);
	//UE_LOG(LogTemp, Warning, TEXT("JsonObject valid?: %s"), jsonObj.IsValid() ? TEXT("TRUE") : TEXT("FALSE"));

	bStringReturn = false;
	for (auto pair : jsonObj.Get()->Values) {
			
		const TArray< TSharedPtr<FJsonValue>>* jsonvalues;
		FString outValue;
		
		if (pair.Value->TryGetString(outValue)){
			FString outKey = pair.Key;
			//UE_LOG(LogTemp, Warning, TEXT("GotString: %s"), *outValue);
			outString = outKey.Append(": ").Append(outValue);
			bStringReturn = true;

		}
		else if(pair.Value->TryGetArray(jsonvalues)) {
			TArray<FMethodJson> methods;
			//UE_LOG(LogTemp, Warning, TEXT("GotARRAY: %s"));
			FJsonObjectConverter::JsonArrayToUStruct<FMethodJson>(*jsonvalues, &methods);
			outMap.Add(pair.Key, methods);
				
		}
		else {
			const TSharedPtr<FJsonObject>* obj;
			if (pair.Value->TryGetObject(obj)) {
				for (auto keyval : obj->Get()->Values) {
					outString.Append(keyval.Key + " : "+ keyval.Value.Get()->AsString()+ ";\n ");
					bStringReturn = true;
				}
			}
		}
	}
	return bStringReturn;
}




//struct --->JsonObj
TSharedPtr<FJsonObject> UCppFunctionLibrary::StructToJsonObj(FMethodJson obj) {
	FString json = "";
	TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(obj);
	return JsonObject;
}


//JsonObj--->FMethodJson(struct)
FMethodJson UCppFunctionLibrary::JsonObjToStruct(TSharedPtr<FJsonObject> obj)
{
	FMethodJson methodStruct;
	if (!FJsonObjectConverter::JsonObjectToUStruct<FMethodJson>(obj.ToSharedRef(), &methodStruct)) {
		return FMethodJson();
	}
	return methodStruct;
}



//Converts a JsonObject to String
FString UCppFunctionLibrary::JsonObjectToString(TSharedPtr<FJsonObject> JsonObject, bool& success, FString& OutInfoMessage)
{
	FString JsonString;
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString, 0))) {
		success = false;
		OutInfoMessage = "Object could not be converted to string";
		return "";
	}
	OutInfoMessage = "Object was successfully converted to string";
	success = true;
	return JsonString;
}

//Creates string ---> JsonObject
TSharedPtr<FJsonObject> UCppFunctionLibrary::JsonStringToJsonObj(FString Json, bool& success, FString& OutInfoMessage)
{
	TSharedPtr<FJsonObject> JsonObject;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), JsonObject)) {
		success = false;
		OutInfoMessage = "String could not be converted to json";
		return nullptr;
	}
	success = true;
	OutInfoMessage = "String was successfully converted to json";
	return JsonObject;
}



//Gets an CallbackObjectContainer such that BP can listen to it
UPythonCallbackContainer* UCppFunctionLibrary::GetBlueprintPythonCallbackObject()
{
	if (pythonCallback == nullptr) {
		pythonCallback = (NewObject<UPythonCallbackContainer>());
	}
	return pythonCallback;
}

UMessageReceivedCallbackContainer* UCppFunctionLibrary::CreateOrGetBlueprintZmqServerCallbackObject(FString Port)
{
	if (!serverCallbacks.Contains(Port)) {
		serverCallbacks.Add(Port, NewObject<UMessageReceivedCallbackContainer>());
	}
	return serverCallbacks.FindChecked(Port);
}

UMessageReceivedCallbackContainer* UCppFunctionLibrary::CreateOrGetBlueprintZmqClientCallbackObject(FString Ip_Port)
{

	if (!clientCallbacks.Contains(Ip_Port)) {
		clientCallbacks.Add(Ip_Port, NewObject<UMessageReceivedCallbackContainer>());
	}

	return clientCallbacks.FindChecked(Ip_Port);
}


//a direct python call given an explicit correctly formatted command
void UCppFunctionLibrary::PythonCall(FString command)
{
	FPythonCommandEx PythonCommand;
	PythonCommand.Command = command;
	UE_LOG(LogTemp, Warning, TEXT("Starting PythongCommand:"));
	IPythonScriptPlugin::Get()->ExecPythonCommandEx(PythonCommand);

}



