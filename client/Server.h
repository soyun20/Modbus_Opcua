#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jsoncpp/json/json.h>
#include <modbus/modbus.h>
#include "unit-test.h.in"
#include "MQTT.h"
#include <iostream>
#include <string>
#include <mariadb/mysql.h>

#if defined(_WIN32)
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif

#define NB_CONNECTION 5

static modbus_t *ctx = NULL;
static modbus_mapping_t *mb_mapping;

static int server_socket = -1;

class Server {
public:
    Server();
    ~Server();
    static void close_sigint(int dummy);
    int start(Json::Value* root);
    void write_log(char* type, char* content, char* topic);

    struct timeval val;
    struct tm *ptm;
    char day[11];
    char filename[15];
    char microsecond[27];
    char log[1000];

    ClientMQTT Send_Log;

    char server[40] = "10.7.12.143";
    char *user = "root";
    char *password = "root";
    char *database = "LOG";
    char *table = "Server_Log";
    char SQL_query[1024];

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    // Json::Value* root;
    int close_MQTT();
    ClientMQTT close_server;
    
    
    
};
