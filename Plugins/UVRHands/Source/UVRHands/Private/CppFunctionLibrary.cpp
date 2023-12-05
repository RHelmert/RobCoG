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
TArray<UPythonCallbackContainer*> UCppFunctionLibrary::callbacklist;



void UCppFunctionLibrary::StartPython(FString filename, FString funcName)
{

	// Run the Python function call (NOTE: It has to loaded prior via init using the same filename)
	
	//PyImport_Import("python");
	//Old version without parameterized input
	//FString myString = "hello.waitFunc()";
	

	//remove ".py" 
	if (filename.EndsWith(".py")) {
		filename.LeftChopInline(3);
	}

	FString file_func = filename + "." +funcName;
	
	//Task to run asynchronosly
	TUniqueFunction<int32()> RTask = [st = file_func]() -> int32 {
		UCppFunctionLibrary::PythonCall(st);
		if (callbacklist.Num() != 0) {
			callbacklist[0]->OnTaskCompletedEvent.Broadcast();
		}
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

void UCppFunctionLibrary::StartPythonAlternate(FString filename, FString funcName)
{




}

void UCppFunctionLibrary::InitPython(FString filename)
{


	if (!OnTaskCompletedEvent.IsBound()) {
		OnTaskCompletedEvent.AddStatic(&UCppFunctionLibrary::TaskCompletedCallback);
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

void UCppFunctionLibrary::StartZmQServer()
{
	//lets try zeromq
	TUniqueFunction<int32()> ZMQTask = []() -> int32 {
		//UCppFunctionLibrary::PythonCall(st);
		int major, minor, patch;
		zmq::version(&major, &minor, &patch);
		UE_LOG(LogTemp, Log, TEXT("Server:ZeroMQ version: v%d.%d.%d"), major, minor, patch);
		UE_LOG(LogTemp, Log, TEXT("Server:ZeroMQ started"));

		// initialize the zmq context with a single IO thread
		zmq::context_t context{ 1 };
		zmq::socket_t socket{ context, zmq::socket_type::rep };
		
		try
		{
			socket.bind("tcp://*:5555");
		}
		catch (const zmq::error_t& e)
		{
			int errorCode = e.num();
			FString emessage = e.what();
			UE_LOG(LogTemp, Error, TEXT("Server: Socket binding error: %d : %s"), errorCode, *emessage);
			return 0;

		}

		int i = 0;
		while (i < 30) {
			UE_LOG(LogTemp, Log, TEXT("Server: iteration %d"), i);
			zmq::message_t request;

			// receive a request from client
			
			//std::cout << "Received " << request.to_string() << std::endl;

			try
			{
				socket.recv(&request, 0);
				// Message received successfully
				FString rMessage = ConvertMessageToString(request);
				//https://forums.unrealengine.com/t/convert-std-string-to-fstring/5065/9
				UE_LOG(LogTemp, Log, TEXT("Server: received message: %s"), *rMessage);



				FString response = "Message received";
				const char* stringValue = TCHAR_TO_ANSI(*response);;
				zmq::message_t yourMessage(stringValue, strlen(stringValue));
				UE_LOG(LogTemp, Log, TEXT("Server: sending response"));
				socket.send(yourMessage);

			}
			catch (const zmq::error_t& e)
			{
				int errorCode = e.num();
				FString emessage = e.what();
				UE_LOG(LogTemp, Error, TEXT("Server: ZeroMQ error code %d and type %s"), errorCode, *emessage);
				//send the message
			}

			i++;
		}

		return 1;
	};
	auto zmqResult = Async(EAsyncExecution::Thread, MoveTemp(ZMQTask));
}

void UCppFunctionLibrary::StartZmQClient()
{

	TUniqueFunction<int32()> ZMQCTask = []() -> int32 {
		//UCppFunctionLibrary::PythonCall(st);
		int major, minor, patch;
		zmq::version(&major, &minor, &patch);
		UE_LOG(LogTemp, Log, TEXT("Client: ZeroMQ version: v%d.%d.%d"), major, minor, patch);
		UE_LOG(LogTemp, Log, TEXT("Client: ZeroMQ started"));


		// initialize the zmq context with a single IO thread
		zmq::context_t context{ 1 };

		UE_LOG(LogTemp, Log, TEXT("Client:Connecting to socket"));
		// construct a REQ (request) socket and connect to interface
		zmq::socket_t socket{ context, zmq::socket_type::req };
		socket.connect("tcp://localhost:5555");

		UE_LOG(LogTemp, Log, TEXT("Client:Trying to send a message in 3"));
		FPlatformProcess::Sleep(3);
		UE_LOG(LogTemp, Log, TEXT("Client:Trying to send a message in 1"));
		FPlatformProcess::Sleep(1);
		//for (auto request_num = 0; request_num < 10; ++request_num)
		//{
			//Create a message and do some string magic
		FString message = "This is the message from the client "; //+ request_num;
		const char* stringValue = TCHAR_TO_UTF8(*message);;
		zmq::message_t yourMessage(stringValue, strlen(stringValue));

		
		try
		{
			UE_LOG(LogTemp, Log, TEXT("Client:Sending"));
			socket.send(yourMessage);
			// Message sent successfully

			//now wait for reply
			zmq::message_t reply{};
			socket.recv(&reply, 0);

			FString re = ConvertMessageToString(reply);
			UE_LOG(LogTemp, Log, TEXT("Client: received answer: %s"), *re);

			// also possible but maybe slower and between each word there is a 20 e.g the answer "Hi this is the server" becomes "Hi 20 this 20 is 20 the 20 server"
			//FString r = reply.str().c_str();
			//UE_LOG(LogTemp, Log, TEXT("Client received answer: %s"), *r);
		}
		catch (const zmq::error_t& e)
		{
			int errorCode = e.num();
			FString emessage = e.what();
			UE_LOG(LogTemp, Error, TEXT("Client: Sending message: ZeroMQ error code %d and description %s"), errorCode, *emessage);
			//send the message
		}

		
	//}
		return 1;
	};
	auto zmqClientResult = Async(EAsyncExecution::Thread, MoveTemp(ZMQCTask));
}


//Callback when task is completed
void UCppFunctionLibrary::TaskCompletedCallback()
{
	UE_LOG(LogTemp, Warning, TEXT("Python Completed"));
}

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


FString UCppFunctionLibrary::StructToJsonString(FMethodJson obj)
{
	TSharedPtr<FJsonObject> jsonOnj = UCppFunctionLibrary::StructToJsonObj(obj);
	bool success;
	FString message;
	return UCppFunctionLibrary::JsonObjectToString(jsonOnj, success, message);
}

//other structs also possible
TSharedPtr<FJsonObject> UCppFunctionLibrary::StructToJsonObj(FMethodJson obj) {
	FString json = "";
	TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(obj);
	return JsonObject;
}


//Converts the Json Object to struct defined in the header
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

//Creates a Json Object from a String
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
UPythonCallbackContainer* UCppFunctionLibrary::GetBlueprintCallbackObject()
{
	if (callbacklist.Num() == 0) {
		callbacklist.Add(NewObject<UPythonCallbackContainer>());
	}
	return callbacklist[0];
}


//a direct python call given an explicit correctly formatted command
void UCppFunctionLibrary::PythonCall(FString command)
{
	FPythonCommandEx PythonCommand;
	PythonCommand.Command = command;
	UE_LOG(LogTemp, Warning, TEXT("Starting PythongCommand:"));
	IPythonScriptPlugin::Get()->ExecPythonCommandEx(PythonCommand);


}


FString UCppFunctionLibrary::ConvertMessageToString(const zmq::message_t& zmqMessage)
{
	// Get the size of the message
	size_t size = zmqMessage.size();

	// If the message is empty, return an empty FString
	if (size == 0)
	{
		return FString();
	}

	// Get the data pointer
	const unsigned char* msgData = static_cast<const unsigned char*>(zmqMessage.data());

	// Convert the data to an FString
	FString result;
	for (size_t i = 0; i < size; ++i)
	{
		result.AppendChar((msgData[i]));
	}

	return result;
}

