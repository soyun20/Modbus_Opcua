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
/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

/* Constants defined by configure.ac */
#define HAVE_INTTYPES_H @HAVE_INTTYPES_H@
#define HAVE_STDINT_H @HAVE_STDINT_H@

// clang-format off
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# ifndef _MSC_VER
# include <stdint.h>
# else
# include "stdint.h"
# endif
#endif
// clang-format on

#define SERVER_ID         17
#define INVALID_SERVER_ID 18

const uint16_t UT_BITS_ADDRESS = 0x130;
const uint16_t UT_BITS_NB = 0x25;
const uint8_t UT_BITS_TAB[] = { 0xCD, 0x6B, 0xB2, 0x0E, 0x1B };

const uint16_t UT_INPUT_BITS_ADDRESS = 0x1C4;
const uint16_t UT_INPUT_BITS_NB = 0x16;
const uint8_t UT_INPUT_BITS_TAB[] = { 0xAC, 0xDB, 0x35 };

const uint16_t UT_REGISTERS_ADDRESS = 0x160;
const uint16_t UT_REGISTERS_NB = 0x3;
const uint16_t UT_REGISTERS_NB_MAX = 0x20;
const uint16_t UT_REGISTERS_TAB[] = { 0x0001, 0x0002, 0x0003 };

/* Raise a manual exception when this address is used for the first byte */
const uint16_t UT_REGISTERS_ADDRESS_SPECIAL = 0x170;
/* The response of the server will contains an invalid TID or slave */
const uint16_t UT_REGISTERS_ADDRESS_INVALID_TID_OR_SLAVE = 0x171;
/* The server will wait for 1 second before replying to test timeout */
const uint16_t UT_REGISTERS_ADDRESS_SLEEP_500_MS = 0x172;
/* The server will wait for 5 ms before sending each byte */
const uint16_t UT_REGISTERS_ADDRESS_BYTE_SLEEP_5_MS = 0x173;

/* If the following value is used, a bad response is sent.
   It's better to test with a lower value than
   UT_REGISTERS_NB_POINTS to try to raise a segfault. */
const uint16_t UT_REGISTERS_NB_SPECIAL = 0x2;

const uint16_t UT_INPUT_REGISTERS_ADDRESS = 0x108;
const uint16_t UT_INPUT_REGISTERS_NB = 0x1;
const uint16_t UT_INPUT_REGISTERS_TAB[] = { 0x000A };

/*
 * This float value is 0x47F12000 (in big-endian format).
 * In Little-endian(intel) format, it will be stored in memory as follows:
 * 0x00 0x20 0xF1 0x47
 *
 * You can check this with the following code:

   float fl = UT_REAL;
   uint8_t *inmem = (uint8_t*)&fl;
   int x;
   for(x = 0; x < 4; x++){
       printf("0x%02X ", inmem[ x ]);
   }
   printf("\n");
 */
const float UT_REAL = 123456.00;

/*
 * The following arrays assume that 'A' is the MSB,
 * and 'D' is the LSB.
 * Thus, the following is the case:
 * A = 0x47
 * B = 0xF1
 * C = 0x20
 * D = 0x00
 *
 * There are two sets of arrays: one to test that the setting is correct,
 * the other to test that the getting is correct.
 * Note that the 'get' values must be constants in processor-endianness,
 * as libmodbus will convert all words to processor-endianness as they come in.
 */
const uint8_t UT_IREAL_ABCD_SET[] = {0x47, 0xF1, 0x20, 0x00};
const uint16_t UT_IREAL_ABCD_GET[] = {0x47F1, 0x2000};
const uint8_t UT_IREAL_DCBA_SET[] = {0x00, 0x20, 0xF1, 0x47};
const uint16_t UT_IREAL_DCBA_GET[] = {0x0020, 0xF147};
const uint8_t UT_IREAL_BADC_SET[] = {0xF1, 0x47, 0x00, 0x20};
const uint16_t UT_IREAL_BADC_GET[] = {0xF147, 0x0020};
const uint8_t UT_IREAL_CDAB_SET[] = {0x20, 0x00, 0x47, 0xF1};
const uint16_t UT_IREAL_CDAB_GET[] = {0x2000, 0x47F1};

/* const uint32_t UT_IREAL_ABCD = 0x47F12000);
const uint32_t UT_IREAL_DCBA = 0x0020F147;
const uint32_t UT_IREAL_BADC = 0xF1470020;
const uint32_t UT_IREAL_CDAB = 0x200047F1;*/

#endif /* _UNIT_TEST_H_ */


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