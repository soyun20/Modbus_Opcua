#include "MQTT.h"
#include "Server.h"
#include "Client.h"
#include <jsoncpp/json/json.h>
#include <pthread.h>

time_t start_time = time(NULL);

void* threadFunction(void* arg)
{
    std::string ID;
    std::string setting_input_id;
    ClientMQTT client_MQTT;
	client_MQTT.Subscribe_Initialize();
	std::string setting_input;
	while (true)
	{
		setting_input = client_MQTT.getMessage();
		if (!setting_input.empty()) {
            break;
        }
	}
    setting_input_id = setting_input;

    Json::Value root_id;
    Json::CharReaderBuilder builder_id;
    std::string errs_id;
    std::istringstream is_stream_id(setting_input);
    if (!Json::parseFromStream(builder_id, is_stream_id, &root_id, &errs_id)) {
		if (!Json::parseFromStream(builder_id, is_stream_id, &root_id, &errs_id)) {
			std::cerr << "Error parsing JSON: " << errs_id << std::endl;
			return nullptr;
		}
    }

    ID = root_id["id"].asString();

    while (true)
	{
		setting_input = client_MQTT.getMessage();
		if (setting_input!=setting_input_id) {
            break;
        }
	}

	std::istringstream iss(client_MQTT.currentTopic);
    std::string token;
    std::vector<std::string> topic;
    while (std::getline(iss, token, '/')) {
        if (!token.empty()) {
            topic.push_back(token);
        }
    }

    Json::Value root;
	Json::Value combinedJson;
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream is_stream(setting_input);
    if (!Json::parseFromStream(builder, is_stream, &root, &errs)) {
		if (!Json::parseFromStream(builder, is_stream, &root, &errs)) {
			std::cerr << "Error parsing JSON: " << errs << std::endl;
			return nullptr;
		}
    }

	client_MQTT.Uninitialize();
	while(1)
	{
		if (topic[0] == "modbus")
		{
			if (topic[1] == "slave")
			{
				Server server;
				server.start(&root, ID);
			}
			else if (topic[1] == "master")
			{
				if (topic[2] == "read")
				{
					Json::Value& msgData = root["msgData"];
					for (Json::Value& msg : msgData) {
						msg["role"] = "read";
					}
					Client client;
					client.start(&root, ID);
				}
				if (topic[2] == "write")
				{
					Json::Value& msgData = root["msgData"];
					for (Json::Value& msg : msgData) {
						msg["role"] = "write";
					}
					Client client;
					client.start(&root, ID);
				}
			}
			else if(topic[1] == "exit")
			{
				return nullptr;
			}
		}
		else if (topic[0] == "opcua")
		{
			std::cout << "opcua_temp" << "\n";
		}
	}
    return nullptr;

}

int main(int argc, char *argv[])
{
    pthread_t newThread;
    std::vector<pthread_t> threads;

    if (pthread_create(&newThread, nullptr, threadFunction, nullptr) != 0) {
        std::cerr << "Error creating thread" << std::endl;
        return 1;
    }
    threads.push_back(newThread);
    pthread_join(newThread, nullptr);
	
    return 0;
}
