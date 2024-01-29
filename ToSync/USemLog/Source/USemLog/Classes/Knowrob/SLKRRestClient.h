#pragma once

#include "HttpModule.h"
#include "Http.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "SLKRResponseStruct.h"
#include <string>


class USEMLOG_API FSLKRRestClient
{
public:

	FSLKRRestClient();
	
	~FSLKRRestClient();


	void Init(const FString& InHost, const FString& InPort, const FString& InProtocol, const FString& GameUser);

	bool IsConnected();

	void SendRequest(FString RequestContent);
	
	//TArray<FString> SendGetActorsRequest(FString find_all_actors_url);

	bool SendCreateEpisodeRequest();

	EHttpRequestStatus::Type SendFinishEpisodeRequest();

	FString getEpisodeIri();

	FString getActionIri();

	double GetGameStartUnixTime();

	void SetGameStartUnixTime(double StartTime);

	void SendCreateSubActionRequest(FString SubActionType, FString TaskType, 
		FString ObjectsPartcipated, FString AdditionalEventInfo, double StartTime, double EndTime);

	void SendPouringAdditionalRequest(FString SubActionType, FString MaxPouringAngle, FString MinPouringAngle,
		FString SourceContainer, FString DestinationContainer, FString PouringPose);

private:
	
	FString URL;

	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void ProcessKnowrobResponse(const FString& ResponseContent);

	FString EpisodeIri;

	FString ActionIri;

	// get episode start time from knowrob
	double GameStartUnixTime;

	double GameStopUnixTime;

	FString GameParticipant;

	

};