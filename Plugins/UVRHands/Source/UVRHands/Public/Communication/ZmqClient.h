// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include <MessageReceivedCallbackContainer.h>


class UVRHANDS_API ZmqClient : public FRunnable {
public:
	ZmqClient(UMessageReceivedCallbackContainer* cont){
		Thread = FRunnableThread::Create(this, TEXT("ZmqClient"));
		EventContainer = cont;
	};
	~ZmqClient();


	virtual bool Init() override;
	virtual uint32 Run() override;
	FString ConvertMessageToString(const zmq::message_t& zmqMessage);
	virtual void Exit() override;
	virtual void Stop() override;
	bool EnqueueMessage(FString Message);

	
	FRunnableThread* Thread;
	
	//Contains normal and blueprint events
	UMessageReceivedCallbackContainer* EventContainer;

	// if true the client will shutdown next iteration
	bool bShutdown = false;

	//Queue to send Messages
	TQueue<FString> queue;

};
