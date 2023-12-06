// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "zmq.hpp"
#include "ZmqClient.h"




ZmqClient::~ZmqClient()
{
}

bool ZmqClient::Init()
{
	/* Should the thread start? */
	//UE_LOG(LogTemp, Warning, TEXT("Init Client...."));
	//queue = TQueue<int>();
	return true;
}

uint32 ZmqClient::Run()
{
	int major, minor, patch;
	zmq::version(&major, &minor, &patch);
	UE_LOG(LogTemp, Log, TEXT("Client: ZeroMQ version: v%d.%d.%d"), major, minor, patch);
	UE_LOG(LogTemp, Log, TEXT("Client: ZeroMQ started"));


	// initialize the zmq context with a single IO thread
	zmq::context_t context{ 1 };

	// construct a REQ (request) socket and connect to interface
	FString ServerAddress = "tcp://localhost:5555";
	UE_LOG(LogTemp, Log, TEXT("Client:Connecting to Server socket: %s"),*ServerAddress);
	zmq::socket_t socket{ context, zmq::socket_type::req };
	socket.connect(TCHAR_TO_UTF8 (*ServerAddress));


	while (!bShutdown) {
		//Sleep as long as the queue is empty
		FPlatformProcess::ConditionalSleep([q = &queue]() -> bool {
			bool wait = !q->IsEmpty();
			return wait;
			});


		//sending test messages
		UE_LOG(LogTemp, Log, TEXT("Client:Trying to send a message in 3"));
		FPlatformProcess::Sleep(1);
		UE_LOG(LogTemp, Log, TEXT("Client:Trying to send a message in 2"));
		FPlatformProcess::Sleep(1);
		UE_LOG(LogTemp, Log, TEXT("Client:Trying to send a message in 1"));
		FPlatformProcess::Sleep(1);


		//Create a message and do some string magic
		FString message = "";
		queue.Dequeue(message);
		const char* stringValue = TCHAR_TO_UTF8(*message);
		zmq::message_t yourMessage(stringValue, strlen(stringValue));

		//lets send the message
		try
		{
			//send message
			UE_LOG(LogTemp, Log, TEXT("Client:Sending"));
			socket.send(yourMessage);


			//sending successful now wait for reply
			zmq::message_t reply{};
			socket.recv(&reply, 0);

			//Ok, we got the replay, lets print it
			FString rMessage = ConvertMessageToString(reply);
			UE_LOG(LogTemp, Log, TEXT("Client: received answer: \"%s\""), *rMessage);

			//Tell the Events in the eventContainer that a message was received
			EventContainer->OnMessageReceivedEvent.Broadcast(rMessage);
			EventContainer->OnMessageReceivedEventBP.Broadcast(rMessage);



			//This is also possible but maybe slower and between each word there is a 20 e.g the answer "Hi this is the server" becomes "Hi 20 this 20 is 20 the 20 server"
			//FString r = reply.str().c_str();
			//UE_LOG(LogTemp, Log, TEXT("Client received answer: %s"), *r);
		}
		catch (const zmq::error_t& e)
		{
			int errorCode = e.num();
			FString emessage = e.what();
			UE_LOG(LogTemp, Error, TEXT("Client: Sending message: ZeroMQ error code %d and description %s"), errorCode, *emessage);
		}
	}
	return 1;

}

FString ZmqClient::ConvertMessageToString(const zmq::message_t& zmqMessage)
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

void ZmqClient::Exit()
{
	/* Post-Run code, threaded */
}

void ZmqClient::Stop()
{
	bShutdown = true;
}

bool ZmqClient::EnqueueMessage(FString Message)
{
	return queue.Enqueue(Message);
}



