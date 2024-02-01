#pragma once

#include <cstring>
#include <string>

#include <mqtt/MQTTClient.h>
#include <mqtt/MQTTAsync.h>
#include <mqtt/MQTTClientPersistence.h>

class ClientMQTT
{
public:
	ClientMQTT();
	~ClientMQTT();

	void Setting();
	void Initialize();
	void Uninitialize();

	void createSubscriber();
	void createPublisher();

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

	std::string _server_url;
	std::string _sub_client_id;
	std::string _pub_client_id;
	std::string _sub_topic;
};

