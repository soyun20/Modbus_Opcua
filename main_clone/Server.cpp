#include "Server.h"

uint16_t swap_bytes(uint16_t value) {
    return (value << 8) | (value >> 8);
}


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
    int current_Session = 0;
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;
    int fdmax;
    
    std::string use_backend;
    const char* ip = "0.0.0.0";
    int port, maxSessionCount, slaveId, waitTimeout, coils, distreteInputs, inputRegisters, holdingRegisters;
    bool byteSwap = false, wordSwap = false;

    if ((*root).isMember("networkData"))
    {
        if ((*root)["networkData"].isMember("protocol")) 
            use_backend = (*root)["networkData"]["protocol"].asString();
        if ((*root)["networkData"].isMember("port")) 
            port = std::stoi((*root)["networkData"]["port"].asString());
        if ((*root)["networkData"].isMember("maxSessionCount")) 
            maxSessionCount = std::stoi((*root)["networkData"]["maxSessionCount"].asString());
        if ((*root)["networkData"].isMember("slaveId")) 
            slaveId = std::stoi((*root)["networkData"]["slaveId"].asString());
        if ((*root)["networkData"].isMember("waitTimeout")) 
            waitTimeout = std::stoi((*root)["networkData"]["waitTimeout"].asString());
    }
    if ((*root).isMember("msgData"))
    {
        if ((*root)["msgData"].isMember("coils"))
            coils = std::stoi((*root)["msgData"]["coils"].asString());
        if ((*root)["msgData"].isMember("distreteInputs")) 
            distreteInputs = std::stoi((*root)["msgData"]["distreteInputs"].asString());
        if ((*root)["msgData"].isMember("inputRegisters")) 
            inputRegisters = std::stoi((*root)["msgData"]["inputRegisters"].asString());
        if ((*root)["msgData"].isMember("holdingRegisters")) 
            holdingRegisters = std::stoi((*root)["msgData"]["holdingRegisters"].asString());
        if ((*root)["msgData"].isMember("byteSwap"))
            byteSwap = (*root)["msgData"]["byteSwap"].asBool();
        if ((*root)["msgData"].isMember("wordSwap")) 
            wordSwap = (*root)["msgData"]["wordSwap"].asBool();
    }
    printf(byteSwap ? "true" : "false");
    ctx = modbus_new_tcp(ip, port);
    
    // Slave ID 설정
    if (modbus_set_slave(ctx, slaveId) == -1) {
        fprintf(stderr, "Failed to set slave id: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return 0;
    }

    // Slave 메모리 Address 설정
    mb_mapping = modbus_mapping_new_start_address( 0,  coils,
                                                   0,  distreteInputs,
                                                   0,  inputRegisters,
                                                   0,  holdingRegisters);
    
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

    FD_ZERO(&refset);
    FD_SET(server_socket, &refset);

    fdmax = server_socket;


    while(!close_server.is_exit) {
        rdset = refset;
        if (select(fdmax + 1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Server select() failure.");
            close_sigint(1);
        }

        for (master_socket = 0; master_socket <= fdmax; master_socket++) {

            if (!FD_ISSET(master_socket, &rdset)) {
                continue;
            }

            if (master_socket == server_socket) {
                if (current_Session < maxSessionCount)
                {
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
                }
                else{
                    printf("Maximum Connection income\n");
                    return 0;
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
                
                printf("receive : %s\n", content);
                write_log("SERVER_INPUT", content, "/log/trans");
                
                if (rc > 0) {
                    modbus_reply(ctx, query, rc, mb_mapping);
                    
                    char content[5 * get_msgLen("INPUT") + 8];
                    int offset = sprintf(content, "paket : ");

                    for (int i = 0; i < get_msgLen("INPUT"); i++) {
                        offset += sprintf(content + offset, "%02X ", get_msg("INPUT")[i]);
                    }
                    content[offset - 1] = '\0';
                    printf("reply : %s\n", content);
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

