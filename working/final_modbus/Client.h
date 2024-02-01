#pragma once

#include <modbus/modbus.h>
#include <string>
#include <signal.h>
#include <jsoncpp/json/json.h>
#include <mariadb/mysql.h>
#include <sys/time.h>
#include "MQTT.h"
#include "unit-test.h.in"
#include <iostream>
#include <bitset>
#include <chrono>
#include <cstdlib>
#include <random>
#include <time.h>
#include <algorithm>

struct ThreadArgs_client {
    std::string id;
    ClientMQTT Send_Log;
};

class Client
{
public:
    Client(){
        tab_rp_bits = nullptr;
        tab_rp_registers = nullptr;
        invalidFunction = 0;
        invalidLength = 0;
    }
    ~Client(){
    }

    void write_log_setting(std::string stype, std::string stopic, char* info_msg);
    void write_log(const char* type, const char* topic, char* content);
    static void* ThreadFunction(void *arg);
    void mqtt_process(Json::Value* setting);
    int start(Json::Value* setting, std::string ID);
    int Setting(Json::Value* setting);

    static void signal_handler(int sig);

    modbus_t* Connection_Initialize();
    int Send_msg(modbus_t *ctx);
    Json::Value mqtt_client(int mqtt_slave_id);    

    void TcpMasterRead(modbus_t *ctx);
    void TcpMasterWrite(modbus_t *ctx);
    void ReadCoils(modbus_t *ctx);
    void ReadDisInputs(modbus_t *ctx);
    void ReadHoldingRegs(modbus_t *ctx);
    void ReadInputRegs(modbus_t *ctx);
    void WriteCoil(modbus_t *ctx);
    void WriteReg(modbus_t *ctx);
    void WriteMultiCoils(modbus_t *ctx);
    void WrtieMultiRegs(modbus_t *ctx);
    void WriteMaskReg(modbus_t *ctx);
    void RWMultiRegs(modbus_t *ctx);
    void SendCustomHexString(modbus_t *ctx);

private:
int initial_connect = 1;
ClientMQTT close_client;
    ClientMQTT Send_Log;
    struct timeval val;
    struct tm *ptm;
    char day[11];
    char filename[15];
    char microsecond[27];
    char log[1000];
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char server[40] = "10.7.12.143";
    char *user = "root";
    char *password = "root";
    char *database = "LOG";
    char *table = "Client_Log";
    char SQL_query[1024];
    uint8_t* query;
    int length;
    uint8_t rsp[MODBUS_RTU_MAX_ADU_LENGTH];
    int connection = 0;
    char info[1024];

    std::string role;
    std::string protocol;
    std::string ip;
    int port;
    int transactionDelay;
    int timeout;


    std::string name;
    std::string clientId;
    int slaveId;
    std::string area;
    int quantity;
    int scanTime;
    bool byteSwap;
    bool wordSwap;

    std::string type;
    std::vector<uint16_t> values;
    uint16_t andMask;
    uint16_t orMask;
    int readAddress;
    int readQuantity;
    int writeAddress;
    bool random;
    std::string hexValue;
    bool invalidFunction;
    bool invalidLength;

    const char *ip_or_device;
    int rc, use_backend;
    int nb_points;
    uint8_t *tab_rp_bits;
    uint16_t *tab_rp_registers;
    uint32_t transaction_delay_to_sec;
    uint32_t transaction_delay_to_usec;
    uint32_t timeout_to_sec;
    uint32_t timeout_to_usec;
    uint32_t scantime_to_sec;
    uint32_t scantime_to_usec;
    uint32_t response_to_sec;
    uint32_t response_to_usec;

    /* RTU */
    std::string comPort;
    int baudrate;
    int dataBit;
    int stopBit;
    std::string parity;
    int invalidChecksum;


    enum Backend {
        TCP,
        RTU,
        ASCII
    };

};
