// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#include "zmq.hpp"
#include "ZmqServer.h"




ZmqServer::~ZmqServer()
{
}

bool ZmqServer::Init()
{
	/* Should the thread start? */
	return true;
}

uint32 ZmqServer::Run()
{

	int major, minor, patch;
	zmq::version(&major, &minor, &patch);
	UE_LOG(LogTemp, Log, TEXT("Server:ZeroMQ version: v%d.%d.%d"), major, minor, patch);
	UE_LOG(LogTemp, Log, TEXT("Server:ZeroMQ started"));

	// initialize the zmq context with a single IO thread
	zmq::context_t context{ 1 };
	zmq::socket_t socket{ context, zmq::socket_type::rep };


	//bind the socket
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

	//server loop
	int i = 0;
	while (!bShutdown) {
		UE_LOG(LogTemp, Log, TEXT("Server: iteration %d waiting for message"), i);

		zmq::message_t request;
		try
		{
			//this blocks the thread
			socket.recv(&request, 0);
			// Message received successfully
			FString rMessage = ConvertMessageToString(request);
			UE_LOG(LogTemp, Log, TEXT("Server: received message: %s"), *rMessage);

			//Tell the Events in the eventContainer that a message was received
			EventContainer->OnMessageReceivedEvent.Broadcast(rMessage);
			EventContainer->OnMessageReceivedEventBP.Broadcast(rMessage);

			FString response = "Message received";
			const char* stringValue = TCHAR_TO_ANSI(*response);;
			zmq::message_t yourMessage(stringValue, strlen(stringValue));
			UE_LOG(LogTemp, Log, TEXT("Server: sending response...: \"%s\""),*response);
			socket.send(yourMessage);

		}
		catch (const zmq::error_t& e)
		{
			int errorCode = e.num();
			FString emessage = e.what();
			UE_LOG(LogTemp, Error, TEXT("Server: ZeroMQ error code %d and type %s"), errorCode, *emessage);
		}
		i++;
	}
	return 1;
	
}

//Convert a message to string
FString ZmqServer::ConvertMessageToString(const zmq::message_t& zmqMessage)
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


void ZmqServer::Exit()
{
	/* Post-Run code, threaded */
}

void ZmqServer::Stop()
{
	bShutdown = true;
}



