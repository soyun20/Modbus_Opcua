#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jsoncpp/json/json.h>
#include <modbus/modbus.h>
#include "MQTT.h"
#include <iostream>
#include <string>
#include <mariadb/mysql.h>
#include <unordered_map>

#if defined(_WIN32)
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif

typedef struct {
    int socket;
    struct sockaddr_in address;
} ClientInfo;

static modbus_t *ctx = NULL;
static modbus_mapping_t *mb_mapping;

static int server_socket = -1;

class Server {
  
public:
    Server();
    ~Server();
    static void close_sigint(int dummy);
    void signal_handler(int sig);
    int start(Json::Value* root, std::string ID);
    int TCP(Json::Value* root);
    int RTU(Json::Value* root);
    int ASCII(Json::Value* root);
    void write_log(char* type, const char* content, char* topic);

    const char* ip = "0.0.0.0";
    int port = 1502;
    int maxSessionCount = 10;
    int slaveId = 1;
    int waitTimeout = 10;
    int coils = 1024;
    int distreteInputs = 1024;
    int inputRegisters = 1024;
    int holdingRegisters = 1024;
    bool byteSwap = false;
    bool wordSwap = false;

    char content[2000];
    

    char server[40] = "10.7.12.143";
    char *user = "root";
    char *password = "root";
    char *database = "LOG";
    char *table = "Server_Log";
    char SQL_query[1024];

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    int close_MQTT();
    
    ClientMQTT Send_Log;
    ClientMQTT close_server;
    time_t start_time;
};
