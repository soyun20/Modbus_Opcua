#pragma once

#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>

typedef unsigned long ULONG_PTR;
typedef ULONG_PTR DWORD_PTR;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
#define MAKEWORD(a, b) ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define LOBYTE(w) ((BYTE)(w)) //하위 2바이트
#define HIBYTE(w) ((BYTE)(((WORD)(w)>>8)&0xff)) //상위 2바이트

class ClientDB
{
public:
    ClientDB();
    ~ClientDB();

    void get_settings();
    int db_conn(MYSQL *conn);
    int write_db(int read_size);
    int finish_with_error(MYSQL *conn);

    char server_addr[32];
    char DB_IP[32];
    char DB_NAME[32];
    char DB_PASS[32];

    bool error = false;

    MYSQL *conn;
};