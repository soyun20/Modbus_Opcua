#pragma once

#include <string>

#include "MQTTClient.h"
#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"

class ClientMQTT
{
public:
	char szMessage[8192] = { 0, };

	ClientMQTT();
	~ClientMQTT();

	void Initialize();
	void Uninitialize();

	void createSubscriber();
	void createPublisher();

	const char* getMessage();

	void OnSuccess(void * context, MQTTAsync_successData * response);
	void OnFailure(void * context, MQTTAsync_failureData * response);
	int OnMessageArrived(void * context, char * topicName, int topicLen, MQTTAsync_message * message);
	int Publish(const char* szTopic, const char* szMessage);
	int Subscribe(const char* szTopic);

	//Publish
	MQTTAsync _pub = nullptr;
	MQTTAsync_message _pub_msg;

	//Subscribe
	MQTTAsync _sub = nullptr;

	//std::string _server_url ="10.7.12.143";
	std::string _server_url ="127.0.0.1";
	std::string _sub_client_id = "1";
	std::string _pub_client_id = "2";
	std::string _sub_topic = "test";
};

