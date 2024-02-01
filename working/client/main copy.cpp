
#include "MQTT.h"
#include "Server.h"
#include "Client.h"
#include <string>
#include <iostream>
#include <jsoncpp/json/json.h>
#include "Client.h"

int main(int argc, char *argv[])
{
	ClientMQTT client_MQTT;
	client_MQTT.Initialize();
	std::string setting_input;

	printf("waiting setting...\n");

	while (true)
	{
		setting_input = client_MQTT.getMessage();
		if (!setting_input.empty()) {
            std::cout << "Received message: " << setting_input << std::endl;
			break;
        }
	}

	Json::CharReaderBuilder rbuilder;
	Json::Value root;
	std::string errs;
    std::istringstream stream(setting_input);

    Json::parseFromStream(rbuilder, stream, &root, &errs);

	if(root["role"].asString()=="client")
	{
		Client client;
		client.start(root);
	}
	else if(root["role"].asString()=="server")
	{
		// Server server;
		// server.start(argc, argv);
	}
	else
	{
		return 0;
	}
	
	printf("program exited\n");

    return 0;
}
