// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "CppFunctionLibrary.h"
#include "../Plugins/Experimental/PythonScriptPlugin/Source/PythonScriptPlugin/Public/IPythonScriptPlugin.h"
#include "Runtime/Core/Public/Features/IModularFeatures.h"



FTaskCompletedEvent UCppFunctionLibrary::OnTaskCompletedEvent;
TArray<UPythonCallbackContainer*> UCppFunctionLibrary::callbacklist;



void UCppFunctionLibrary::StartPython(FString filename, FString funcName)
{

	// Run the Python function call (NOTE: It has to loaded prior via init using the same filename)
	

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


//Callback when task is completed
void UCppFunctionLibrary::TaskCompletedCallback()
{
	UE_LOG(LogTemp, Warning, TEXT("Python Completed"));
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
	IPythonScriptPlugin::Get()->ExecPythonCommand(*command);


}


