#pragma once

#include <string>

#include "MQTTClient.h"
#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"
#include <vector>
#include "string.h"
#include <jsoncpp/json/json.h>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <unistd.h>

class ClientMQTT
{
public:
	char szMessage[8192] = { 0, };
	std::string currentTopic;
	std::vector<std::string> _sub_topics;
	std::string is_exit = "qwe";
	typedef void (*MessageArrivedCallback)(const char* topic, const char* message);
    void setMessageArrivedCallback(MessageArrivedCallback callback);
	
	ClientMQTT();
	~ClientMQTT();

	void Subscribe_Initialize();
	void Publish_Initialize();
	
	void Uninitialize();
	bool IsConnected();

	void createSubscriber();
	void createPublisher();
	void OnSuccess_pub(void * context, MQTTAsync_successData * response);
	
	std::string getExit();

	const char* getMessage();
	void* resetMessage();
	std::string getTopic();

	void OnSuccess(void * context, MQTTAsync_successData * response);
	void OnFailure(void * context, MQTTAsync_failureData * response);
	int OnMessageArrived(void * context, char * topicName, int topicLen, MQTTAsync_message * message);
	int Publish(const char* szTopic, const char* szMessage);
	//Publish
	MQTTAsync _pub = nullptr;
	MQTTAsync_message _pub_msg = MQTTAsync_message_initializer;
	bool isConnected = false;

	//Subscribe
	MQTTAsync _sub = nullptr;
	
	std::string _server_url = "10.7.12.143:1883";
	std::string _sub_client_id = "server_sub_id";
	std::string _pub_client_id = "server_pub_id";
	std::string _sub_topic = "test";

	MessageArrivedCallback userCallback = nullptr;
};


