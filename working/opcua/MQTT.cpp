#include "MQTT.h"


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

void gOnSuccess_pub(void * context, MQTTAsync_successData * response)
{
	ClientMQTT* pMQTT = (ClientMQTT*)context;
	pMQTT->OnSuccess_pub(context, response);
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
	opts.will = NULL;
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

	int rc = MQTTAsync_create(&_pub, _server_url.c_str(), _pub_client_id.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if (rc != MQTTASYNC_SUCCESS)
	{
		return;
	}
	
	opts.keepAliveInterval = 10;
	opts.connectTimeout = 1;
	opts.cleansession = 1;
	opts.context = this;
	opts.onSuccess = gOnSuccess_pub;
	opts.onFailure = gOnFailure;

	rc = MQTTAsync_connect(_pub, &opts);

	if (rc != MQTTCLIENT_SUCCESS)
	{
		return;
	}
}

void ClientMQTT::Subscribe_Initialize()
{
	createSubscriber();
}

void ClientMQTT::Publish_Initialize()
{
	createPublisher();
}

void ClientMQTT::OnSuccess_pub(void * context, MQTTAsync_successData * response)
{
	printf("Publish Initialize Success\n");
	isConnected = true;
}

void ClientMQTT::OnSuccess(void * context, MQTTAsync_successData * response)
{
	_sub_topics.push_back("/modbus/master/read");
    _sub_topics.push_back("/modbus/master/write");
	_sub_topics.push_back("/modbus/slave");
	_sub_topics.push_back("/modbus/exit");
	_sub_topics.push_back("/opcua/server");
	_sub_topics.push_back("/opcua/exit");
	_sub_topics.push_back("/id");
    
    int num_topics = _sub_topics.size();
    int *qos_values = new int[num_topics];

	char *str1 = "/modbus/master/read";
	char *str2 = "/modbus/master/write";
	char *str3 = "/modbus/slave";
	char *str4 = "/modbus/exit";
	char *str5 = "/opcua/server";
	char *str6 = "/opcua/exit";
	char *str7 = "/id";
	char *const array[] = {str1, str2, str3, str4, str5, str6, str7};

	char *const *topic = array;
	qos_values[0] = 1;
	qos_values[1] = 1;
	qos_values[2] = 1;
	qos_values[3] = 1;
	qos_values[4] = 1;
	qos_values[5] = 1;
	qos_values[6] = 1;
	
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    rc = MQTTAsync_subscribeMany(_sub, num_topics, topic, qos_values, &opts);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        printf("Subscribe Many Fail , Error (%d)\n", rc);
    }
	printf("Subscribe Initialize Success\n");
}

void ClientMQTT::OnFailure(void * context, MQTTAsync_failureData * response)
{
	printf("OnFailure\n");
}

int ClientMQTT::OnMessageArrived(void * context, char * topicName, int topicLen, MQTTAsync_message * message)
{
	if (message->payloadlen == 0 || message->payload == NULL)
		return 0;

	currentTopic = topicName;

	memcpy(szMessage, message->payload, message->payloadlen);
	szMessage[message->payloadlen] = '\0';
	
	Json::Value root;
	Json::CharReaderBuilder builder;
	std::string errs;
	std::istringstream is_stream(szMessage);
	if (!Json::parseFromStream(builder, is_stream, &root, &errs)) {
		std::cerr << "Error parsing JSON: " << errs << std::endl;
		return 0;
	}
	
	printf("topic = {%s}, {%s}\n", topicName, szMessage);

	if (currentTopic == "/modbus/exit") {
        ClientMQTT::is_exit = root["id"].asString();
    }
	if (currentTopic == "/opcua/exit") {
        ClientMQTT::is_exit = root["id"].asString();
    }
    
    return 1;
}

const char* ClientMQTT::getMessage(){
	return szMessage;
}

void* ClientMQTT::resetMessage(){
	memset(szMessage, 0, sizeof(szMessage));
	return NULL;
}

std::string ClientMQTT::getExit()
{
	return is_exit;
}

int ClientMQTT::Publish(const char* szTopic, const char* szMessage)
{
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;
	char* buff = (char*)malloc(strlen(szMessage)+1);
	
	_pub_msg.payload = (buff);
	memset(_pub_msg.payload, 0, strlen(szMessage));
	_pub_msg.payloadlen = strlen(szMessage);
	_pub_msg.payload = (void*)szMessage;
	
	_pub_msg.dup = 0;
	_pub_msg.qos = 1;

	if ((rc = MQTTAsync_sendMessage(_pub, szTopic, &_pub_msg, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("MQTTAsync_sendMessage failed =%d\n", rc);
	}
	
	free(buff);
	return 0;
}

void ClientMQTT::Uninitialize()
{
	currentTopic.clear();

	MQTTAsync_disconnect(_pub, NULL);
	MQTTAsync_destroy(&_pub);
	MQTTAsync_free(&_pub);

	MQTTAsync_disconnect(_sub, NULL);
	MQTTAsync_destroy(&_sub);
	MQTTAsync_free(&_sub);
}