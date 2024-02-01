#include "MQTT.h"
#include "Server.h"
#include "Client.h"

#include "opcuaClient.h"
#include "opcuaServer.h"

#include <jsoncpp/json/json.h>
#include <pthread.h>
#include <algorithm>

std::vector<std::string> ids;
std::map<std::string, pthread_t> threadMap;
std::map<std::string, bool> shouldRun;

struct ThreadArgs {
    std::string id;
    std::string topic;
    Json::Value root;
};

time_t start_time = time(NULL);


void* threadFunction(void* arg) {
    ThreadArgs* targs = (ThreadArgs*) arg;
    std::string id = targs->id;
    std::string topic = targs->topic;
    Json::Value root = targs->root;

    ClientMQTT thread_MQTT;
    thread_MQTT._sub_client_id = id;
    thread_MQTT.Subscribe_Initialize();
    std::string MQTT_input;
    bool is_running = true;
    bool is_first = true;
    std::cout << "new Thread - id : " << id << ", topic : " << topic << std::endl;
    std::cout << is_first << std::endl;
    while(is_running) {
        while (!is_first) {
            MQTT_input = thread_MQTT.getMessage();
            // std::cout << MQTT_input << std::endl;
            if (!MQTT_input.empty()) {
                Json::Value combinedJson;
                Json::CharReaderBuilder builder;
                std::string errs;
                std::istringstream is_stream(MQTT_input);
                if (!Json::parseFromStream(builder, is_stream, &root, &errs)) {
                    std::cerr << "Error parsing JSON: " << errs << std::endl;
                }
                topic = thread_MQTT.currentTopic;   
                break;
            }
        }
        
        is_first = false;

        if(topic == "/modbus/exit" && id == targs->id) {
            is_running = false;
            break;
        }

        // if(!root.isMember("networkData") || !root.isMember("msgData") || !root.isMember("ousNetworkData") || !root.isMember("ousMemoryTreeData"))
        // {
        //     thread_MQTT.resetMessage();
        //     MQTT_input.clear();
        //     continue;
        // }
        
        
        std::istringstream iss(topic);
        std::string token;
        std::vector<std::string> topics;
        while (std::getline(iss, token, '/')) {
            if (!token.empty()) {
                topics.push_back(token);
            }
        }
        std::cout << topics[0] << topics[1] << std::endl << std::endl << std::endl << std::endl;
        if (topics[0] == "modbus")
        {
            if (topics[1] == "slave")
            {
                Server server;
                server.start(&root);
            }
            else if (topics[1] == "master")
            {
                if (topics[2] == "read")
                {
                    Json::Value& msgData = root["msgData"];
                    for (Json::Value& msg : msgData) {
                        msg["role"] = "read";
                    }
                    Client client;
                    client.start(&root, id);
                }
                if (topics[2] == "write")
                {
                    Json::Value& msgData = root["msgData"];
                    for (Json::Value& msg : msgData) {
                        msg["role"] = "write";
                    }
                    Client client;
                    client.start(&root, id);
                }
            }
        }
         else if (topics[0] == "opcua")
        {
            // if (topics[1] == "server")
            // {
            //     OpcuaServer server;
            //     server.start(&root);

            // }
            // else if (topics[1] == "client")
            // {
            //     OPC_Client opc_Client;
            //     opc_Client.start(&root);
            // }
        }
        
        break;
    }
    
    
    thread_MQTT.resetMessage();
    MQTT_input.clear();
    delete targs;
    std::cout << "[main] exit : " << id << std::endl;
    return nullptr;
}

int main(int argc, char *argv[])
{
    ClientMQTT main_MQTT;
    main_MQTT._sub_client_id = "main";
    main_MQTT.Subscribe_Initialize();
    std::string MQTT_input;
    std::string topic;
    ThreadArgs* args;
    while(1) {
        main_MQTT.resetMessage();
        MQTT_input.clear();
        topic.clear();
        while (1) {
            MQTT_input = main_MQTT.getMessage();
            if (!MQTT_input.empty()) {
                break;
            }
        }

        topic = main_MQTT.currentTopic;

        
        Json::Value root;
        Json::Value combinedJson;
        Json::CharReaderBuilder builder;
        std::string errs;
        std::istringstream is_stream(MQTT_input);
        
        if (!Json::parseFromStream(builder, is_stream, &root, &errs)) {
            std::cerr << "Error parsing JSON: " << errs << std::endl;
        }
        
        std::string id = root["id"].asString();
        
        if (topic == "/modbus/exit") {
            if (std::find(ids.begin(), ids.end(), id) != ids.end()) {
                ids.erase(std::remove(ids.begin(), ids.end(), id), ids.end());
                threadMap.erase(id);
            }
            continue;
        }
        
        if (std::find(ids.begin(), ids.end(), id) == ids.end()) {
            ids.push_back(id);
            std::cout << "[main] new ID income : " << id << "\n";
            
            args = new ThreadArgs {id, topic, root};
            pthread_t thread_id;
            
            pthread_create(&thread_id, NULL, threadFunction, args);
            threadMap[id] = thread_id;
        }

    }
    for (const auto& pair : threadMap) {
        pthread_join(pair.second, nullptr);
    }
    return 0;
}
