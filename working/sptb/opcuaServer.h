#include <open62541/server.h>
#include <open62541/src/ua_securechannel.h>
#include <open62541/server_config_default.h>
#include <open62541/client_config_default.h>
#include <open62541/plugin/historydata/history_data_backend_memory.h>
#include <open62541/plugin/historydata/history_data_gathering_default.h>
#include <open62541/plugin/historydata/history_database_default.h>
#include <open62541/plugin/historydatabase.h>
#include <open62541/src/ua_util_internal.h>
#include <open62541/src/server/ua_server_internal.h>
#include <open62541/config.h>
#include <open62541/plugin/accesscontrol_default.h>
#include <open62541/examples/common.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/client_subscriptions.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <stdarg.h>
#include <cstdarg>
#include <string>
#include <sys/time.h>
#include <mariadb/mysql.h>
#include <errno.h>
#include <signal.h>
#include "MQTT.h"

class OpcuaServer {
  
public:
    // OpcuaServer();
    // ~OpcuaServer();

    int nodeSetting(Json::Value* setting);
    void signal_handler(int sig);
    int start(Json::Value* setting);
    // void write_log(std::string stype, std::string stopic, char* content);
    // void customLogger(void *logContext, UA_LogLevel level, UA_LogCategory category, const char *msg, va_list args);
    // void write_log(char* type, const char* content, char* topic);

private:
    // std::string id;
    std::string ip;
    int port;
    std::string certFile;
    std::string keyFile;
    std::vector<std::vector<std::string>> users;
    int nodeCount;
    std::string route;
    std::string label;
    std::string category;
    std::string type;
    std::string accessRight;
    std::vector<std::vector<std::string>> inArguments;
    std::vector<std::vector<std::string>> outArguments;

    int close_MQTT();
    
    // ClientMQTT Send_Log;
    ClientMQTT close_server;
    time_t start_time;

    // MYSQL *conn;
    // MYSQL_RES *res;
    // MYSQL_ROW row;
    // char server[40] = "10.7.12.143";
    // char *user = "root";
    // char *password = "root";
    // char *database = "LOG";
    // char *table = "Server_Log";
    // char SQL_query[1024];
    // uint8_t* query;

    // char filename[15];
    // char microsecond[27];
    // char log[1000];
    // char log_text[1024];

    // struct timeval val;
    // struct tm *ptm;


};
