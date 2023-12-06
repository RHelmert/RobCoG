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



FTaskCompletedEvent UCppFunctionLibrary::OnTaskCompletedEvent;
UPythonCallbackContainer* UCppFunctionLibrary::pythonCallback;
UMessageReceivedCallbackContainer* UCppFunctionLibrary::serverCallback;
UMessageReceivedCallbackContainer* UCppFunctionLibrary::clientCallback;
ZmqServer* UCppFunctionLibrary::server;
ZmqClient* UCppFunctionLibrary::client;




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

bool  UCppFunctionLibrary::StartZmQServer()
{
	if (server != nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d ZmqServer is already running...."), *FString(__func__), __LINE__);
		return false;
	}
	// Creates and starts a new zmq server with the callback object on which the events are called
	UCppFunctionLibrary::GetBlueprintZmqServerCallbackObject();
	server = new ZmqServer(serverCallback);
	return true;
	
}

bool UCppFunctionLibrary::StartZmQClient()
{
	if (client != nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d ZmqClient is already running...."), *FString(__func__), __LINE__);
		return false;
	}
	// Creates and starts a new zmq server with the callback object on which the events are called
	UCppFunctionLibrary::GetBlueprintZmqClientCallbackObject();
	client = new ZmqClient(clientCallback);
	return true;
}

void UCppFunctionLibrary::SendZmqMessageOverClient(FString Message)
{
	if (client == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s::%d ZmqClient is not running...."), *FString(__func__), __LINE__);
		return;
	}
	client->EnqueueMessage(Message);

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
	st.method_args.Add("hello");
	st.method_args.Add("hi");
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
	TSharedPtr<FJsonObject> jsonOnj = UCppFunctionLibrary::StructToJsonObj(obj);
	bool success;
	FString message;
	return UCppFunctionLibrary::JsonObjectToString(jsonOnj, success, message);
}

//JsonString to FMethodJson. Necessary for BPs since they can not work wit TSharedPtr
FMethodJson UCppFunctionLibrary::JsonStringToStruct(FString jsonString)
{
	bool success;
	FString message;
	TSharedPtr<FJsonObject> jsonObj = UCppFunctionLibrary::JsonStringToJsonObj(jsonString, success, message);
	return UCppFunctionLibrary::JsonObjToStruct(jsonObj);

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

UMessageReceivedCallbackContainer* UCppFunctionLibrary::GetBlueprintZmqServerCallbackObject()
{
	if (serverCallback == nullptr) {
		serverCallback = (NewObject<UMessageReceivedCallbackContainer>());
	}
	return serverCallback;
}

UMessageReceivedCallbackContainer* UCppFunctionLibrary::GetBlueprintZmqClientCallbackObject()
{
	if (clientCallback == nullptr) {
		clientCallback = (NewObject<UMessageReceivedCallbackContainer>());
	}
	return clientCallback;
}


//a direct python call given an explicit correctly formatted command
void UCppFunctionLibrary::PythonCall(FString command)
{
	FPythonCommandEx PythonCommand;
	PythonCommand.Command = command;
	UE_LOG(LogTemp, Warning, TEXT("Starting PythongCommand:"));
	IPythonScriptPlugin::Get()->ExecPythonCommandEx(PythonCommand);


}



