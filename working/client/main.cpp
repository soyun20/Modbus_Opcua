#include "MQTT.h"
#include "Server.h"
#include "Client.h"
#include <string>
#include <iostream>
#include <string>
#include <jsoncpp/json/json.h> 

int exit_process = 0;

int main(int argc, char *argv[])
{
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
        std::cerr << "Error parsing JSON: " << errs << std::endl;
        return 1;
    }

	
    
	client_MQTT.Uninitialize();
	
	if (topic[0] == "modbus")
	{
		if (topic[1] == "slave")
		{
			Server server;
			server.start(&root);
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
				client.start(&root);
			}
			if (topic[2] == "write")
			{
				Json::Value& msgData = root["msgData"];
				for (Json::Value& msg : msgData) {
					msg["role"] = "write";
				}
				Client client;
				client.start(&root);
			}
		}
		else if(topic[1] == "exit")
		{
			return 0;
		}
	}
	else if (topic[0] == "opcua")
	{
		std::cout << "opcua_temp" << "\n";
	}
    return 0;
}

