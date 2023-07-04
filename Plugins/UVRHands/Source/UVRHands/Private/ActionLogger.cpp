// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "ActionLogger.h"
#include "CoreMisc.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"


//Ctor
UActionLogger::UActionLogger()
{

}

UActionLogger::~UActionLogger()
{
	if (FileName != "") {
		if (username != "") {
			FString newFilename = username +"_"+ FileName.Mid(9);
			WriteLogToFile(newFilename);
		}
		else
		{
			WriteLogToFile(FileName);
		}
	}
}



void UActionLogger::LogAction(float date, FString message)
{
	
	times.Add(date);
	messages.Emplace(message);
}

void UActionLogger::ClearLog()
{
	times.Empty();
	messages.Empty();

}

void UActionLogger::Init()
{
	AActor* owner = GetOwner();
	UWorld* World = owner->GetWorld();
	FString LevelName = World->GetMapName();
	FString CurrentDate = FDateTime::Now().ToString(TEXT("%Y-%m-%d"));
	FString CurrentTime = FDateTime::Now().ToString(TEXT("%H-%M-%S"));

	FileName = FString::Printf(TEXT("%s_%s_%s"), *LevelName, *CurrentDate, *CurrentTime);
}

void UActionLogger::SetUsername(FString name)
{
	username = name;
}



void UActionLogger::RemoveEntry(int index)
{
	times.RemoveAt(index);
	messages.RemoveAt(index);
}

void UActionLogger::ReplaceArray(TArray<float> newTimes, TArray<FString> newMessages)
{
	times = newTimes;
	messages = newMessages;
}

bool UActionLogger::WriteLogToFile(
	FString FileName
)
{
	FString ProjectDirectory = FPaths::ProjectDir();
	//get complete file path
	UE_LOG(LogTemp, Warning, TEXT("FilePaths: ProjectDirectory: %s"), *ProjectDirectory);
	ProjectDirectory += "TimeLogging/";
	ProjectDirectory += FileName;
	ProjectDirectory += ".log";

	FString LogText;
	for (int i = 0; i < times.Num(); i++) {
		LogText += "[";
		LogText += FString::SanitizeFloat(times[i]);
		LogText += "]: ";
		LogText += messages[i];
		LogText += "\n";
	}


	if (FFileHelper::SaveStringToFile(LogText, *ProjectDirectory))
	{
		UE_LOG(LogTemp, Warning, TEXT("Successfully saved to the described File"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Failed saving to file"));

	}
	return true;
		//return false;
}

void UActionLogger::GetItems(TArray<float>& timeStamps, TArray<FString>& messageArray)
{
	timeStamps = times;
	messageArray = messages;
}



