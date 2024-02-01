#pragma once

#include <string>

#include "MQTTClient.h"
#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"
#include <vector>

class ClientMQTT
{
public:
	char szMessage[8192] = { 0, };
	std::string currentTopic;
	std::vector<std::string> _sub_topics;
	int is_exit = 0;
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
	
	int getExit();

	const char* getMessage();
	std::string getTopic();

	void OnSuccess(void * context, MQTTAsync_successData * response);
	void OnFailure(void * context, MQTTAsync_failureData * response);
	int OnMessageArrived(void * context, char * topicName, int topicLen, MQTTAsync_message * message);
	int Publish(const char* szTopic, const char* szMessage);
	//Publish
	MQTTAsync _pub = nullptr;
	MQTTAsync_message _pub_msg;

	//Subscribe
	MQTTAsync _sub = nullptr;
	
	std::string _server_url = "10.7.12.143";
	std::string _sub_client_id = "client_sub_id";
	std::string _pub_client_id = "client_pub_id";
	std::string _sub_topic = "/modbus/master/write";

	MessageArrivedCallback userCallback = nullptr;
};
