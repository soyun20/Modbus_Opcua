#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>

#include "/usr/include/mysql/mysql.h"

typedef unsigned long ULONG_PTR;
typedef ULONG_PTR DWORD_PTR;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
#define MAKEWORD(a, b) ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define LOBYTE(w) ((BYTE)(w)) //하위 2바이트
#define HIBYTE(w) ((BYTE)(((WORD)(w)>>8)&0xff)) //상위 2바이트

void Print_Hexa_Buff(char *buff, size_t len);
void finish_with_error(MYSQL *conn);
void InterruptHandler(int sig);
void help();

#define MBAP_SIZE 7
#define SEND_SIZE 12
#define REGISTER_CNT 10

int RSTART = 0;
int RLENGTH = 10;
int error = 0;

char s_buff[SEND_SIZE];
int sock;
MYSQL *conn;

int main(int argc, char **argv)
{
    FILE *fp;
    char tmp[32];
    char setting_arr[16][16];
    char *ptr;
    char server_addr[32], DB_IP[32], DB_NAME[32], DB_PASS[32];
    int PORT;

    fp = fopen("setting.config", "r");
    for(int i=0;i<5;i++)
    {
        fgets(tmp, sizeof(tmp), fp);
        strtok(tmp, "=");
        ptr = strtok(NULL, "\n");
        strcpy(setting_arr[i], ptr);
    }
    strcpy(server_addr, setting_arr[0]);
    strcpy(DB_IP, setting_arr[1]);
    strcpy(DB_NAME, setting_arr[2]);
    strcpy(DB_PASS, setting_arr[3]);
    PORT = atoi(setting_arr[4]);

    signal(SIGINT, InterruptHandler);

    // getopt_long function
    int c;

    while (1)
    {
        static struct option long_options[] =
            {
                {"help", no_argument, 0, 'h'},
                {"RegisterStart", required_argument, 0, 's'},
                {"RegisterLength", required_argument, 0, 'l'},
                {0, 0, 0, 0}};

        int option_index = 0;
        c = getopt_long(argc, argv, "s:l:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 'h':
            help();
            break;
        case 's':
            RSTART = atoi(optarg) - 1;
            break;
        case 'l':
            RLENGTH = atoi(optarg);
            break;

        default:
            break;
        }
    }
    const int DATA_SIZE = RLENGTH;
    const int READ_SIZE = 9 + DATA_SIZE * 2;

    conn = mysql_init(NULL);
    int IDX = 0;
    char r_buff[READ_SIZE];
    char query[1024] = {'\0',};
    unsigned int DbRow[128] = {0,};

    char buf[20] = {0,};

    if (!conn)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return -1;
    }

    if (!mysql_real_connect(conn, DB_IP, DB_NAME, DB_PASS, NULL, 0, NULL, 0))
    {
        finish_with_error(conn);
        if (error)
            return -1;
    }

    // create DB, table
    mysql_query(conn, "DROP DATABASE IF EXISTS testdb;");
    mysql_query(conn, "CREATE DATABASE testdb;");
    mysql_query(conn, "USE testdb;");
    mysql_query(conn, "DROP TABLE IF EXISTS generator;");
    mysql_query(conn, "CREATE TABLE generator (IDX INT UNIQUE KEY, TID INT, PID INT, LEN INT, UID INT, Func INT, Bytes INT,\
                                               R1 INT, R2 INT, R3 INT, R4 INT, R5 INT, R6 INT, R7 INT, R8 INT, R9 INT, R10 INT, DATE TIMESTAMP DEFAULT NOW());");
    mysql_query(conn, "DROP TABLE IF EXISTS register;");
    mysql_query(conn, "CREATE TABLE register (IDX INT UNIQUE KEY, R1 INT, R2 INT, R3 INT, R4 INT, R5 INT, R6 INT, R7 INT, R8 INT, R9 INT, R10 INT, DATE TIMESTAMP DEFAULT NOW(),\
                                              FOREIGN KEY (IDX) REFERENCES `generator` (IDX) ON UPDATE CASCADE);");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("errno is %d", errno);
        return -1;
    }
    // connect
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(server_addr);
    serveraddr.sin_port = htons(PORT);

    int ret = connect(sock, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr));
    if (ret != 0)
    {
        printf("errno is %d", errno);
        return -1;
    }

    while (error == 0)
    {
        printf("accept Stand by...\n");
        memset(s_buff, 0, SEND_SIZE);
        memset(r_buff, 0, READ_SIZE);

        for (int i = 0; i < 5; i++)
            s_buff[i] = 0x00; // TID, PID, Length_1
        
        s_buff[5] = 0x06;     // Length_2
        s_buff[6] = 0x01;     // Unit ID
        s_buff[7] = 0x03;     // Function Code

        s_buff[8] = HIBYTE(RSTART);          // Data Start Adress_1
        s_buff[9] = LOBYTE(RSTART);          // Data Start Adress_2

        s_buff[10] = HIBYTE(RLENGTH);        // Data Read Length_1
        s_buff[11] = LOBYTE(RLENGTH);        // Data Read Length_2

        printf("%s", s_buff);
        while (error == 0)
        {
            if(error == 1)
                return -1;
            
            ret = send(sock, s_buff, SEND_SIZE, 0);
            if (ret >= 0)
            {
                printf("Sended Byte\t:");
                Print_Hexa_Buff(s_buff, SEND_SIZE);
            }
            else
            {
                printf("Send Failed \n");
                return -1;
            }

            ret = recv(sock, r_buff, READ_SIZE, 0);

            
            if (ret >= 0)
            {
                printf("Received Byte\t:");
                Print_Hexa_Buff(r_buff, READ_SIZE);
                printf("Received Data\t:");
                for (int i = 0; i < DATA_SIZE * 2; i += 2)
                    printf("%d ", (r_buff[i + 9] << 8) | r_buff[i + 10]);
                printf("\n\n");
            }
            else
            {
                printf("Receive Failed");
                break;
            }
            sleep(1);

            // Save value of r_buff in the proper format
            // TID, PID, LEN
            int DbRow_cnt = 0;
            memset(DbRow, 0, sizeof(DbRow));
            for (int i = 0; i < MBAP_SIZE - 1; i += 2)
            {
                DbRow[DbRow_cnt] = MAKEWORD(r_buff[i+1],r_buff[i]);
                DbRow_cnt++;
            }

            // UID, Func, Bytes
            for (int i = MBAP_SIZE - 1; i < MBAP_SIZE + 2; i++)
            {
                DbRow[DbRow_cnt] = MAKEWORD(r_buff[i],0);
                DbRow_cnt++;
            }

            // R1 ~ R10
            int register_number = 0;
            int i = MBAP_SIZE + 2;
            for (int j = 0; j < REGISTER_CNT; j++)
            {
                if (register_number >= RSTART && register_number < (RSTART + RLENGTH))
                {
                    DbRow[DbRow_cnt] = MAKEWORD(r_buff[i+1],r_buff[i]);
                    i += 2;
                }
                DbRow_cnt++;
                register_number++;
            }

            if(error == 1)
                return -1;

            printf("\n\n\n");

            // Put variable in DB
            //#define SET_VARIABLE "INSERT INTO generator VALUES(@IDX = ?, @TID = ?, @PID = ?, @LEN = ?, @UID = ?, @Func = ?, @Bytes = ?,\
                                                       @R1 = ?, @R2 = ?, @R3 = ?, @R4 = ?, @R5 = ?, @R6 = ?, @R7 = ?, @R8 = ?, @R9 = ?, @R10 = ?, NOW());"
            #define SET_VARIABLE "SET @IDX = ?, @TID = ?, @PID = ?, @LEN = ?, @UID = ?, @Func = ?, @Bytes = ?,\
                                                       @R1 = ?, @R2 = ?, @R3 = ?, @R4 = ?, @R5 = ?, @R6 = ?, @R7 = ?, @R8 = ?, @R9 = ?, @R10 = ?;"
            MYSQL_STMT *stmt;
            MYSQL_BIND bind[17];

            stmt = mysql_stmt_init(conn);

            mysql_stmt_prepare(stmt, SET_VARIABLE, strlen(SET_VARIABLE));
            memset(bind, 0, sizeof(bind));

            for(int i=0; i<=DbRow_cnt; i++)
            {
                if(i==0)
                {
                    ++IDX;
                    bind[i].buffer_type = MYSQL_TYPE_LONG;
                    bind[i].buffer = (char*)&IDX;
                    bind[i].is_null= 0;
                    bind[i].length= 0;
                }
                else
                {
                    bind[i].buffer_type = MYSQL_TYPE_LONG;
                    bind[i].buffer = (char*)&DbRow[i-1];
                    bind[i].is_null= 0;
                    bind[i].length= 0;
                }
            }

            mysql_stmt_bind_param(stmt, bind);
            mysql_stmt_execute(stmt);
            mysql_stmt_close(stmt);

            if (mysql_query(conn, "INSERT INTO generator VALUES(@IDX, @TID, @PID, @LEN, @UID, @Func, @Bytes, @R1, @R2, @R3, @R4, @R5, @R6, @R7, @R8, @R9, @R10, NOW());"))
            {
                finish_with_error(conn);
                if (error)
                    return -1;
            }
            if (mysql_query(conn, "INSERT INTO register VALUES(@IDX, @R1, @R2, @R3, @R4, @R5, @R6, @R7, @R8, @R9, @R10, NOW());"))
            {
                finish_with_error(conn);
                if (error)
                    return -1;
            }
        }
    }
}

void Print_Hexa_Buff(char *buff, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++)
    {
        printf("%02X ", (unsigned char)buff[i]);
    }
    printf("\n");
}

void finish_with_error(MYSQL *conn)
{
    printf("error\n");
    fprintf(stderr, "%s\n", mysql_error(conn));
    mysql_close(conn);
    error = 1;
}

void help()
{
    printf("Usage options\n");
    printf("--help / -h\tOption Guide\n--RegisterStart / -s\tSet start register\n--RegisterLength / -l\tSet length register\n");
}

void InterruptHandler(int sig)
{
    printf("\nDone\n");
    mysql_close(conn);
    close(sock);
    error = 1;
}