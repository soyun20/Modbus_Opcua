#include "string.h"
#include "MQTT.h"
#include <iostream>


ClientMQTT::ClientMQTT()
{
}


ClientMQTT::~ClientMQTT()
{
}

int gmessage_arrive(void * context, char * topicName, int topicLen, MQTTAsync_message * message)
{
	ClientMQTT* pMQTT = (ClientMQTT*)context;

	return pMQTT->OnMessageArrived(context, topicName, topicLen, message);
}
void gOnSuccess(void * context, MQTTAsync_successData * response)
{
	ClientMQTT* pMQTT = (ClientMQTT*)context;
	pMQTT->OnSuccess(context, response);
}
void gOnFailure(void * context, MQTTAsync_failureData * response)
{
	ClientMQTT* pMQTT = (ClientMQTT*)context;
	pMQTT->OnFailure(context, response);
}
void ClientMQTT::createSubscriber()
{
	MQTTAsync_connectOptions opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_willOptions wopts = MQTTAsync_willOptions_initializer;
	MQTTAsync_SSLOptions sslopts = MQTTAsync_SSLOptions_initializer;

	int rc = 0;

	rc = MQTTAsync_create(&_sub, _server_url.c_str(), _sub_client_id.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		return;
	}

	rc = MQTTAsync_setCallbacks(_sub, this, NULL, gmessage_arrive, NULL);

	opts.keepAliveInterval = 20;
	opts.cleansession = 1;

	opts.will = NULL; /* don't need will for this client, as it's going to be connected all the time */
	opts.context = this;
	opts.onSuccess = gOnSuccess;
	opts.onFailure = gOnFailure;

	rc = MQTTAsync_connect(_sub, &opts);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		return;
	}
}
void ClientMQTT::createPublisher()
{
	
	memset(&_pub_msg, 0, sizeof(MQTTAsync_message));

	_pub_msg.struct_id[0] = 'M';
	_pub_msg.struct_id[1] = 'Q';
	_pub_msg.struct_id[2] = 'T';
	_pub_msg.struct_id[3] = 'M';

	MQTTAsync_connectOptions opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_willOptions wopts = MQTTAsync_willOptions_initializer;
	MQTTAsync_SSLOptions sslopts = MQTTAsync_SSLOptions_initializer;

	//_client_id = "DIM_Pub";

	int rc = MQTTAsync_create(&_pub, _server_url.c_str(), _pub_client_id.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if (rc != MQTTASYNC_SUCCESS)
	{
		return;
	}

	opts.keepAliveInterval = 10;
	opts.connectTimeout = 1;
	opts.cleansession = 1;
	opts.context = this;
	opts.onSuccess = gOnSuccess;
	opts.onFailure = gOnFailure;

	rc = MQTTAsync_connect(_pub, &opts);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		return;
	}
}
void ClientMQTT::Initialize()
{
	createSubscriber();
	createPublisher();
	
}
void ClientMQTT::OnSuccess(void * context, MQTTAsync_successData * response)
{
	printf("OnSuccess\n");

	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;

	rc = MQTTAsync_subscribe(_sub, _sub_topic.c_str(), 1, &opts);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		printf("Subscribe Fail , Error(%d)\n", rc);
	}
}
void ClientMQTT::OnFailure(void * context, MQTTAsync_failureData * response)
{
	printf("OnFailure\n");
}
int ClientMQTT::OnMessageArrived(void * context, char * topicName, int topicLen, MQTTAsync_message * message)
{
	if (message->payloadlen == 0 || message->payload == NULL)
		return 0;

	
	memcpy(szMessage, message->payload, message->payloadlen);
	szMessage[message->payloadlen] = '\0';

	printf("topic = {%s}, {%s}\n", topicName, szMessage);

	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);

	return 1;
}
const char* ClientMQTT::getMessage(){
	return szMessage;
}

int ClientMQTT::Publish(const char* szTopic, const char* szMessage)
{
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

	char* buff = (char*)malloc(strlen(szMessage));
	_pub_msg.payload = buff;
	memset(_pub_msg.payload, 0, strlen(szMessage));
	_pub_msg.payloadlen = strlen(szMessage);
	_pub_msg.payload = (void*)szMessage;
	_pub_msg.dup = 0;
	_pub_msg.qos = 1;
	
	int rc = MQTTAsync_sendMessage(_pub, szTopic, &_pub_msg, &opts);

	if (rc != MQTTASYNC_SUCCESS)
	{
		printf("MQTTAsync_sendMessage failed =%d\n", rc);
	}

	free(buff);
	return 0;
}
int ClientMQTT::Subscribe(const char* szTopic)
{
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

    int rc = MQTTAsync_subscribe(_sub, szTopic, 1, &opts);
    if (rc != MQTTASYNC_SUCCESS)
    {
        printf("Subscribe Fail , Error(%d)\n", rc);
    }
	return 0;
}
void ClientMQTT::Uninitialize()
{
	MQTTAsync_disconnect(_pub, NULL);
	MQTTAsync_destroy(&_pub);
	MQTTAsync_free(&_pub);

	MQTTAsync_disconnect(_sub, NULL);
	MQTTAsync_destroy(&_sub);
	MQTTAsync_free(&_sub);
}