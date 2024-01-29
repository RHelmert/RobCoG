// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include <MessageReceivedCallbackContainer.h>




class UVRHANDS_API ZmqServer : public FRunnable {
public:
	ZmqServer(UMessageReceivedCallbackContainer* cont){
		Thread = FRunnableThread::Create(this, TEXT("ZmqServer"));
		EventContainer = cont;
	};
	~ZmqServer();


	virtual bool Init() override;
	virtual uint32 Run() override;
	FString ConvertMessageToString(const zmq::message_t& zmqMessage);
	virtual void Exit() override;
	virtual void Stop() override;


	//Contains normal and blueprint events
	UMessageReceivedCallbackContainer* EventContainer;


	FRunnableThread* Thread;
	bool bShutdown = false;

};
