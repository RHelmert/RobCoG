// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ActionLogger.generated.h"


/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UVRHANDS_API UActionLogger : public UActorComponent
{
	GENERATED_BODY()

private:

	TArray<float> times;
	TArray<FString> messages;
	FString FileName;
	FString username;


public:
	UActionLogger();
	~UActionLogger();


	//LogAction
	UFUNCTION(BlueprintCallable, Category = "Logging")
		void LogAction(float time, FString message);

	//ClearLog
	UFUNCTION(BlueprintCallable, Category = "Logging")
		void ClearLog();

	//Init
	UFUNCTION(BlueprintCallable, Category = "Logging")
		void Init();
	//Sets the current Name
	UFUNCTION(BlueprintCallable, Category = "Logging")
		void SetUsername(FString name);


	//ClearSpecific Entry
	UFUNCTION(BlueprintCallable, Category = "Logging")
		void RemoveEntry(int index);

	//Replace/SetArrays
	UFUNCTION(BlueprintCallable, Category = "Logging")
		void ReplaceArray(TArray<float> newTimes, TArray<FString> neWesssages);

	//Write to File
	UFUNCTION(BlueprintCallable, Category = "Logging")
		bool WriteLogToFile(
			FString NameOfFile);

	//GetItems
	UFUNCTION(BlueprintCallable, Category = "Logging")
		void GetItems(TArray<float>& timeStamps, TArray<FString>& messageArray);


	









};
