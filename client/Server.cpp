#include "Server.h"

void signal_handler(int sig) {
    if (server_socket != -1) {
        close(server_socket);
    }
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);

    exit(sig);
}
void Server::close_sigint(int dummy)
{
    printf("close\n");
    if (server_socket != -1) {
        close(server_socket);
    }
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);

    exit(dummy);
}
Server::Server() {
    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN);
    Send_Log.Publish_Initialize();
}

Server::~Server() {
}

void Server::write_log(char* type, char* content, char* topic){
    memset(filename , 0 , sizeof(filename));
    memset(microsecond , 0 , sizeof(microsecond));
    memset(log , 0 , sizeof(log));
    
    gettimeofday(&val, NULL);
    ptm = localtime(&val.tv_sec);
    sprintf(filename, "%04d-%02d-%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
    sprintf(microsecond, "%04d-%02d-%02d %02d:%02d:%02d.%06ld"
        , ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday
        , ptm->tm_hour, ptm->tm_min, ptm->tm_sec
        , val.tv_usec);
    sprintf(log, "{\"time\" : \"%s\", \"type\" : \"%s\", \"content\" : \"%s\"}", microsecond, type, content);
    
    Send_Log.Publish(topic, log);

    snprintf(filename + strlen(filename), sizeof(filename) - strlen(filename), ".log");
    FILE *file = fopen(filename, "a");
    if (!file) {
        perror("Failed to open log file");
        return;
    }
    fprintf(file, "[%s] [%s] %s\n", microsecond, type, content);
    fclose(file);


    conn = mysql_init(NULL);

    if (mysql_real_connect(conn, server, user, password, database, 3306, NULL, 0) == NULL) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }

    
    snprintf(SQL_query, sizeof(SQL_query), "INSERT INTO %s (time, type, content) VALUES ('%s', '%s', '%s')", table, microsecond, type, content);

    if (mysql_query(conn, SQL_query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
    }

    mysql_close(conn);

}

int Server::start(Json::Value* root)
{
    close_server.Subscribe_Initialize();
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;
    int fdmax;
    
    const char* use_backend;
    const char* ip;
    int port, transaction_delay, timeout;
    Json::Value setting = *root;

    if (setting.isMember("protocol")) 
        use_backend = setting["protocol"].asString().c_str();
    if (setting.isMember("ip"))
        ip = setting["ip"].asString().c_str();
    if (setting.isMember("port"))
        port = setting["port"].asInt();
    if (setting.isMember("transaction_delay"))
        transaction_delay = setting["transaction_delay"].asInt();
    if (setting.isMember("timeout"))
        timeout = setting["timeout"].asInt();
    

    ctx = modbus_new_tcp(ip, port);
    
    mb_mapping = modbus_mapping_new_start_address(
        UT_BITS_ADDRESS,            UT_BITS_NB,
        UT_INPUT_BITS_ADDRESS,      UT_INPUT_BITS_NB,
        UT_REGISTERS_ADDRESS,       UT_REGISTERS_NB_MAX,
        UT_INPUT_REGISTERS_ADDRESS, UT_INPUT_REGISTERS_NB);
    
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
    }

    modbus_set_bits_from_bytes(
        mb_mapping->tab_input_bits, 0, UT_INPUT_BITS_NB, UT_INPUT_BITS_TAB);

    for (int i = 0; i < UT_REGISTERS_NB; i++) {
        mb_mapping->tab_registers[i] = UT_REGISTERS_TAB[i];
    }

    // mb_mapping =
    //     modbus_mapping_new(MODBUS_MAX_READ_BITS, 0, MODBUS_MAX_READ_REGISTERS, 0);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);
    if (server_socket == -1) {
        fprintf(stderr, "Unable to listen TCP connection\n");
        modbus_free(ctx);
        return -1;
    }

    signal(SIGINT, close_sigint);

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = server_socket;

    while(!close_server.is_exit) {
        rdset = refset;
        if (select(fdmax + 1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Server select() failure.");
            close_sigint(1);
        }

        /* Run through the existing connections looking for data to be
         * read */
         for (master_socket = 0; master_socket <= fdmax; master_socket++) {

            if (!FD_ISSET(master_socket, &rdset)) {
                continue;
            }

            if (master_socket == server_socket) {
                /* A client is asking a new connection */
                socklen_t addrlen;
                struct sockaddr_in clientaddr;
                int newfd;

                /* Handle new connections */
                addrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, sizeof(clientaddr));
                newfd = accept(server_socket, (struct sockaddr *) &clientaddr, &addrlen);
                if (newfd == -1) {
                    perror("Server accept() error");
                } else {
                    FD_SET(newfd, &refset);

                    if (newfd > fdmax) {
                        /* Keep track of the maximum */
                        fdmax = newfd;
                    }
                    printf("New connection from %s:%d on socket %d\n",
                           inet_ntoa(clientaddr.sin_addr),
                           clientaddr.sin_port,
                           newfd);
                }
            } else {
                modbus_set_socket(ctx, master_socket);
                rc = modbus_receive(ctx, query);
                
                char content[5 * rc + 8];
                int offset = sprintf(content, "paket : ");

                for (int i = 0; i < rc; i++) {
                    offset += sprintf(content + offset, "%02X ", query[i]);
                }
                content[offset - 1] = '\0';
                
                
                printf("%s\n", content);
                write_log("SERVER_INPUT", content, "/modbus/log");
                
                if (rc > 0) {
                    modbus_reply(ctx, query, rc, mb_mapping);
                } else if (rc == -1) {
                    /* This example server in ended on connection closing or
                     * any errors. */
                    printf("Connection closed on socket %d\n", master_socket);
                    close(master_socket);

                    /* Remove from reference set */
                    FD_CLR(master_socket, &refset);

                    if (master_socket == fdmax) {
                        fdmax--;
                    }
                }
            }
        }
    }

    return 0;
}

