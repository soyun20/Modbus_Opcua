#include "db.h"

ClientDB::ClientDB()
{  
}
ClientDB::~ClientDB()
{
}

void ClientDB::get_settings()
{
    FILE *fp;
    char tmp[32];
    char setting_arr[16][16];
    char *ptr;
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
    printf("setting SUCCESS\n");

    return;
}

int ClientDB::db_conn(MYSQL *conn)
{
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

    printf("connect SUCCESS\n");
    return 0;
}

int ClientDB::write_db(int write_size)
{
    int IDX = 0;
    char query[1024] = {'\0',};
    unsigned int DbRow[128] = {0,};
    char buf[20] = {0,};

    for(int i=0;i<5;i++)
    {
        BYTE lowByte = 0x34; // 하위 8비트 값
        BYTE highByte = 0x12; // 상위 8비트 값
    
        // Save value of r_buff in the proper format
        // DATA
        int DbRow_cnt = 0;
        memset(DbRow, 0, sizeof(DbRow));
        
        for (int i = 0; i < write_size; i++)
        {
            DbRow[DbRow_cnt] = MAKEWORD(lowByte, highByte);
            DbRow_cnt++;
        }

        // Put variable in DB
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
    printf("write SUCCESS\n");
    return 0;
}

int ClientDB::finish_with_error(MYSQL *conn)
{
    printf("error\n");
    fprintf(stderr, "%s\n", mysql_error(conn));
    mysql_close(conn);
    return -1;
    error = true;
}