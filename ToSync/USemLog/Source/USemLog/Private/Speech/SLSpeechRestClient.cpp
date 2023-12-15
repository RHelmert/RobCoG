#include "Speech/SLSpeechRestClient.h"
#include "Knowrob/SLKRRestClient.h"
#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "Containers/Queue.h"
#include <string>


FSLSpeechRestClient::FSLSpeechRestClient()
{
    //GameParticipant = TEXT("");
}

FSLSpeechRestClient::~FSLSpeechRestClient()
{
}

//void FSLSpeechRestClient::Init(const FString& InHost, const FString& InPort, const FString& InProtocol, const FString& GameUser) {

    //URL = TEXT("172.31.115.208:62226/knowrob/api/v1.0/query");
    //URL = InHost + TEXT(":") + InPort + TEXT("/knowrob/api/v1.0/");
    //URL = InHost + TEXT(":") + InPort;
    

    //GameParticipant = GameUser;
    
    //IsConnected();

//}
//\"query\": \"true\",
bool FSLSpeechRestClient::IsConnected() {
    //SendRequest(TEXT("{ \"query\": \"A=2.\", \"maxSolutionCount\" : 1}"));
    return true;
}

void FSLSpeechRestClient::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
    /*
    TSharedPtr<FJsonObject> ResponseObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    FJsonSerializer::Deserialize(Reader, ResponseObj);

    UE_LOG(LogTemp, Display, TEXT("Response %s"), *Response->GetContentAsString());*/
    //UE_LOG(LogTemp, Display, TEXT("Title: %s"), *ResponseObj->GetStringField("title"));
}

FString FSLSpeechRestClient::getEpisodeIri() {
    return EpisodeIri;
}


FString FSLSpeechRestClient::getActionIri() {
    return ActionIri;
}



bool FSLSpeechRestClient::SendRecordAudioRequest() {

    FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

    URL = TEXT("127.0.0.1:5000");
    FString FINAL_URL = URL + TEXT("/record");
    UE_LOG(LogTemp, Warning, TEXT("URL : %s"), *FINAL_URL);

    Request->SetURL(FINAL_URL);
    Request->SetVerb((TEXT("GET")));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    Request->ProcessRequest();
    Request->OnProcessRequestComplete().BindLambda(
        [&](
            FHttpRequestPtr Request,
            FHttpResponsePtr Response,
            bool connectedSuccessfully) mutable {
                if (connectedSuccessfully) {
                    FString ResponseJsonString = *Response->GetContentAsString(); //Json converted to FString
                    UE_LOG(LogTemp, Display, TEXT("Response %s"), *ResponseJsonString);
                    // Parse Json Message Here
                    TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(ResponseJsonString);
                    TSharedPtr< FJsonObject > JsonObject;
                    bool DeserializeState = FJsonSerializer::Deserialize(Reader, JsonObject);
                    if (!DeserializeState)
                    {
                        UE_LOG(LogTemp, Error, TEXT("[%s] Deserialization Error. Message Contents: %s"),
                            *FString(__FUNCTION__), *ResponseJsonString);
                        return;
                    }
                    else {
                        if (JsonObject != NULL) {
                            //Get the value of the json object by field name
                            FString message = JsonObject->GetStringField("status");
                            UE_LOG(LogTemp, Warning, TEXT("STATUS : %s"), *message);
                            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("STATUS: %s"), *message));

                        }
                    }
                }
                else {
                    switch (Request->GetStatus()) {
                    case EHttpRequestStatus::Failed_ConnectionError:
                        UE_LOG(LogTemp, Error, TEXT("Connection failed."));
                    default:
                        UE_LOG(LogTemp, Error, TEXT("Request failed."));
                    }
                }
        });

    EHttpRequestStatus::Type Status = Request->GetStatus();
    const TCHAR* status = EHttpRequestStatus::ToString(Status);
    UE_LOG(LogTemp, Warning, TEXT("Episode Start request response status: %s"), status);

    return true;
}

EHttpRequestStatus::Type FSLSpeechRestClient::SendStopAudioRequest() {
    bool returnStatus;

    FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    URL = TEXT("127.0.0.1:5000");
    FString FINAL_URL = URL + TEXT("/stop");
    UE_LOG(LogTemp, Warning, TEXT("URL : %s"), *FINAL_URL);

    Request->SetURL(FINAL_URL);
    Request->SetVerb((TEXT("GET")));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->OnProcessRequestComplete().BindLambda(
        [&](
            FHttpRequestPtr Request,
            FHttpResponsePtr Response,
            bool connectedSuccessfully) mutable {
                
                if (connectedSuccessfully) {
                    returnStatus = true;
                    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("STATUS: Recording Stopped")));
                    UE_LOG(LogTemp, Display, TEXT("Successfully connected to the server"));
                    // We should have a JSON response - attempt to process it.
                    FString ResponseJsonString = *Response->GetContentAsString(); //Json converted to FString
                    UE_LOG(LogTemp, Display, TEXT("Response %s"), *ResponseJsonString); // log the response as string

                    // Parse Json Message Here
                    TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(ResponseJsonString);
                    TSharedPtr< FJsonObject > JsonObject;
                    bool DeserializeState = FJsonSerializer::Deserialize(Reader, JsonObject);
                    if (!DeserializeState)
                    {
                        UE_LOG(LogTemp, Error, TEXT("[%s] Deserialization Error. Message Contents: %s"),
                            *FString(__FUNCTION__), *ResponseJsonString);
                        return;
                    }
                    else {
                        if (JsonObject != NULL) {
                            //Get the value of the json object by field name
                            //ActionIri = JsonObject->GetStringField("Action");
                            //EpisodeIri = JsonObject->GetStringField("Episode");
                            
                            //FString message = JsonObject->GetStringField("transcription");
                            //UE_LOG(LogTemp, Warning, TEXT("JSON Content Received : %s"),*message);
                            //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Transcription: %s"), *message));
                            //Transcription = message;

                            FString message1;
                            const TArray<TSharedPtr<FJsonValue>> m1=  JsonObject->GetArrayField("transcription");
                            TArray<TMap<FString, FString>> contentlist;
                            for (int32 index = 0; index < m1.Num(); index++)
                            {                              
                                TSharedPtr<FJsonObject> obj = m1[index]->AsObject();
                                FString Sentence = obj->GetStringField("sentence");
                                FString SysStartTime = obj->GetStringField("system_start_time");
                                FString SysEndTime = obj->GetStringField("system_end_time");
                                FString UNIXStartTime = obj->GetStringField("UNIX_start_time");
                                FString UNIXEndTime = obj->GetStringField("UNIX_end_time");
                                UE_LOG(LogTemp, Warning, TEXT("JSON Content Received : %s, %s, %s, %s, %s"),*Sentence,*SysStartTime,*SysEndTime,*UNIXStartTime, *UNIXEndTime);
                                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Sentence: %s, StartTime:%s, EndTime:%s, UNIXStart:%s, UNIXEnd:%s"),
                                    *Sentence,*SysStartTime,*SysEndTime,*UNIXStartTime,*UNIXEndTime));
                                Message.Add(TEXT("Sentence"), Sentence);
                                Message.Add(TEXT("SysStartTime"), SysStartTime);
                                Message.Add(TEXT("SysEndTime"), SysEndTime);
                                Message.Add(TEXT("UNIXStartTime"), UNIXStartTime);
                                Message.Add(TEXT("UNIXEndTime"), UNIXEndTime);
                                message1 = Sentence + SysStartTime + SysEndTime+ "\n";
                                contentlist.Add(Message);
                            }
                            Transcription = message1;
                            Total.Add(TEXT("Transcription"), contentlist);

                            //Printing The Contents in Total Variable
                            //for (const TPair<FString, TArray<TMap<FString, FString>>>& trans : Total)
                            //{
                            //    FString key = trans.Key;
                            //    TArray<TMap<FString, FString>> v1 = trans.Value;
                            //    for (int32 index = 0; index < v1.Num(); index++)
                            //    {
                            //        for (const TPair<FString, FString>& trans2 : v1[index])
                            //        {
                            //            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Transcription FOUND: %s : %s"), *trans2.Key, *trans2.Value));
                            //            //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Transcription FOUND: %s"), ));
                            //        }
                            //    }
                            //}
                                         
                            
                            // Right now we are getting GameStartTime from KnowRob(linux side), 
                            // if the tfs are missmatched then sync both PC times 
                            //GameStartUnixTime = double(JsonObject->GetNumberField("Time"));
                            //UE_LOG(LogTemp, Display, TEXT("game start time: %lf"), GameStartUnixTime); // log time

                            //UE_LOG(LogTemp, Display, TEXT("ActionIri found: %s"), *ActionIri); // log actor as string
                            //UE_LOG(LogTemp, Display, TEXT("EpisodeIri found: %s"), *EpisodeIri); // log actor as string
                        }
                    }
                }
                else {
                    returnStatus = false;
                    switch (Request->GetStatus()) {
                    case EHttpRequestStatus::Failed_ConnectionError:
                        UE_LOG(LogTemp, Error, TEXT("Connection failed."));
                    default:
                        UE_LOG(LogTemp, Error, TEXT("Request failed."));
                    }
                }
        });

    Request->ProcessRequest();

    EHttpRequestStatus::Type Status = Request->GetStatus();
    const TCHAR* status = EHttpRequestStatus::ToString(Status);
    UE_LOG(LogTemp, Warning, TEXT("Episode Stop request response status: %s"), status);
    return Status;
}


