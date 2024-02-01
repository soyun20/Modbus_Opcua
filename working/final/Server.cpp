#include "Server.h"

std::string trans_func(int functionCode) {
    static std::unordered_map<int, std::string> funcMap = {
        {1,  "MODBUS_FC_READ_COILS"},
        {2,  "MODBUS_FC_READ_DISCRETE_INPUTS"},
        {3,  "MODBUS_FC_READ_HOLDING_REGISTERS"},
        {4,  "MODBUS_FC_READ_INPUT_REGISTERS"},
        {5,  "MODBUS_FC_WRITE_SINGLE_COIL"},
        {6,  "MODBUS_FC_WRITE_SINGLE_REGISTER"},
        {7,  "MODBUS_FC_READ_EXCEPTION_STATUS"},
        {8,  "MODBUS_FC_DIAGNOSTIC"},
        {11, "MODBUS_FC_GET_COMM_EVENT_COUNTER"},
        {12, "MODBUS_FC_GET_COMM_EVENT_LOG"},
        {15, "MODBUS_FC_WRITE_MULTIPLE_COILS"},
        {16, "MODBUS_FC_WRITE_MULTIPLE_REGISTERS"},
        {17, "MODBUS_FC_REPORT_SLAVE_ID"},
        {20, "MODBUS_FC_READ_FILE_RECORD"},
        {21, "MODBUS_FC_WRITE_FILE_RECORD"},
        {22, "MODBUS_FC_MASK_WRITE_REGISTER"},
        {23, "MODBUS_FC_READ_WRITE_MULTIPLE_REGISTERS"},
        {24, "MODBUS_FC_READ_FIFO_QUEUE"},
        {43, "MODBUS_FC_ENCAPSULATED_INTERFACE_TRANSPORT"}
    };
    auto iter = funcMap.find(functionCode);
    if (iter != funcMap.end()) {
        return iter->second;
    } else {
        return "UNKNOWN_FUNCTION_CODE";
    }
}
uint16_t byte_swap_16(uint16_t value) {
    return (value << 8) | (value >> 8);
}
void word_swap_16(uint16_t *array, int start, int length) {
    int len = length - start;
    if (len % 2 == 1) {
        array[length] = 0x00;
        length++;
    }
    for (int i = start; i < length; i += 2) {
        uint16_t temp = array[i];
        array[i] = array[i + 1];
        array[i + 1] = temp;
    }
}
void byte_swap_8(uint8_t *array, int start, int length) {
    
    int len = length - start;
    printf("%d %d %d\n", start, length, len);
    if (len % 2 == 1) {
        array[length] = 0x00;
        length++;
    }
    for (int i = start; i < length; i += 2) {
        uint8_t temp = array[i];
        array[i] = array[i + 1];
        array[i + 1] = temp;
    }
}

void word_swap_8(uint8_t *array, int start, int length) {
    int len = length - start;
    if (len % 4 != 0) {
        while (length % 4 != 0) {
            array[length] = 0x00;
            length++;
        }
    }
    for (int i = start; i < length; i += 4) {
        uint8_t temp1 = array[i];
        uint8_t temp2 = array[i + 1];

        array[i] = array[i + 2];
        array[i + 1] = array[i + 3];

        array[i + 2] = temp1;
        array[i + 3] = temp2;
    }
}
char * value(uint8_t *array, int start, int end) {
  char *result = (char *)malloc((end - start + 1) * 4);
    if (!result) {
        return nullptr; 
    }
    
    char *currentPos = result;
    for (int i = start; i <= end; ++i) {
        currentPos += sprintf(currentPos, "%d ", array[i-1]);
    }
    *(currentPos-1) = '\0';
    
    return result;
}
Server::Server() {
    signal(SIGINT, close_sigint);
    signal(SIGPIPE, SIG_IGN);
    
    Send_Log.Publish_Initialize();
    close_server.Subscribe_Initialize();
}

Server::~Server() {

}

int Server::start(Json::Value* root, std::string ID)
{
    uint8_t *query;
    
    int current_Session = 0;
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;
    int fdmax;
   
    std::string protocol;
    
    if ((*root).isMember("networkData"))
    {
        if ((*root)["networkData"].isMember("protocol")) 
            protocol = (*root)["networkData"]["protocol"].asString();
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

    // START 로그 출력
    int start_time = time(NULL);
    while(!Send_Log.isConnected)
	{
        sleep(1);
		if(time(NULL) - start_time > 5)
		{
			printf("MQTT Publish Connection Timeout : 5 Seconds\n");
			return 0;
		}
	}
    write_log   ("SYSTEM", 
                (protocol == "RTU")   ? "MODBUS RTU SERVER START"   :
                (protocol == "TCP")   ? "MODBUS TCP SERVER START"   :
                (protocol == "ASCII") ? "MODBUS ASCII SERVER START" :
                                        "UNKNOWN PROTOCOL", "/log/system");
    
    
    // ctx 설정
    if (protocol == "TCP") {
        ctx = modbus_new_tcp(ip, port);
        query = (uint8_t *)malloc(MODBUS_TCP_MAX_ADU_LENGTH);
    } else if (protocol == "RTU") {
        ctx = modbus_new_rtu(ip, port, 'N', 8, 1);
        query = (uint8_t *)malloc(MODBUS_RTU_MAX_ADU_LENGTH);
    } else {
        ctx = modbus_new_tcp(ip, port);
    }
    
    // Slave ID 설정
    if (modbus_set_slave(ctx, slaveId) == -1) {
        fprintf(stderr, "Failed to set slave id: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return 0;
    }
    
    // Slave 메모리 주소 설정
    mb_mapping = modbus_mapping_new_start_address( 0,  coils,
                                                   0,  distreteInputs,
                                                   0,  inputRegisters,
                                                   0,  holdingRegisters);
 
    // Slave 메모리 값 설정
    const uint8_t UT_BITS_TAB[] = { 0xCD, 0x6B, 0xB2, 0x0E, 0x1B };
    const uint8_t UT_INPUT_BITS_TAB[] = { 0xAC, 0xDB, 0x35 };
    const uint16_t UT_INPUT_REGISTERS_TAB[] = { 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A };
    const uint16_t UT_REGISTERS_TAB[] = { 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A};

    modbus_set_bits_from_bytes(mb_mapping->tab_bits, 0, coils, UT_BITS_TAB);
    modbus_set_bits_from_bytes(mb_mapping->tab_input_bits, 0, distreteInputs, UT_INPUT_BITS_TAB);

    if (byteSwap)
    {
        for (int i = 0; i < inputRegisters; i++) {
            mb_mapping->tab_input_registers[i] = byte_swap_16(UT_INPUT_REGISTERS_TAB[i]);
        }
        for (int i = 0; i < holdingRegisters; i++) {
            mb_mapping->tab_registers[i] = byte_swap_16(UT_REGISTERS_TAB[i]);
        }
    } else {
        for (int i = 0; i < inputRegisters; i++) {
            mb_mapping->tab_input_registers[i] = UT_INPUT_REGISTERS_TAB[i];
        }
        for (int i = 0; i < holdingRegisters; i++) {
            mb_mapping->tab_registers[i] = UT_REGISTERS_TAB[i];
        }
    }
    if(wordSwap)
    {
        word_swap_16(mb_mapping->tab_input_registers, 0, mb_mapping->nb_input_registers);
        word_swap_16(mb_mapping->tab_registers, 0, mb_mapping->nb_registers);
    }
    
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
        write_log("ERROR", "Failed to allocate the mapping", "/log/system");
        modbus_free(ctx);
        return -1;
    }
    
    // TCP 소켓 Listen
    server_socket = modbus_tcp_listen(ctx, maxSessionCount);
    if (server_socket == -1) {
        fprintf(stderr, "Unable to listen connection\n");
        write_log("ERROR", "Unable to listen connection", "/log/system");
        modbus_free(ctx);
        return -1;
    }
    
    FD_ZERO(&refset);
    FD_SET(server_socket, &refset);

    fdmax = server_socket;
    ClientInfo clients[maxSessionCount];
    while(1) {  
        if(close_server.is_exit == ID)
        {
            write_log   ("SYSTEM", 
                (protocol == "RTU")   ? "MODBUS RTU SERVER CLOSE"   :
                (protocol == "TCP")   ? "MODBUS TCP SERVER CLOSE"   :
                (protocol == "ASCII") ? "MODBUS ASCII SERVER CLOSE" :
                                        "UNKNOWN PROTOCOL", "/log/system");
            
            if (server_socket != -1) {
                close(server_socket);
            }
            modbus_free(ctx);
            modbus_mapping_free(mb_mapping);
            break;
        }

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
                    socklen_t addrlen;
                    struct sockaddr_in clientaddr;
                    int newfd;

                    addrlen = sizeof(clientaddr);
                    memset(&clientaddr, 0, sizeof(clientaddr));
                    newfd = accept(server_socket, (struct sockaddr *) &clientaddr, &addrlen);
                    if (newfd == -1) {
                        perror("Server accept() error");
                    } else {
                        FD_SET(newfd, &refset);
                        
                        if (newfd > fdmax) {
                            fdmax = newfd;
                        }
                        printf("New connection from %s:%d on socket %d\n",
                            inet_ntoa(clientaddr.sin_addr),
                            clientaddr.sin_port,
                            newfd);

                        memset(content, 0, sizeof(content));
                        snprintf(content, sizeof(content), "New connection from %s:%d on socket %d",
                                inet_ntoa(clientaddr.sin_addr),
                                ntohs(clientaddr.sin_port),
                                newfd);
                        write_log("SYSTEM", content, "/log/system");
                        clients[newfd].socket = newfd;
                        clients[newfd].address = clientaddr;
                    }
                }
                else{
                    printf("Maximum Connection income\n");
                    return 0;
                }
                
            } else {
                // Receive
                modbus_set_socket(ctx, master_socket);
                rc = modbus_receive(ctx, query);

                // ByteSwap일 때 받아온 패킷 swap
                if (byteSwap)
                {   
                    if (query[modbus_get_header_length(ctx)] == MODBUS_FC_WRITE_SINGLE_REGISTER)
                        byte_swap_8(query, 9, rc);
                    if (query[modbus_get_header_length(ctx)] == MODBUS_FC_WRITE_MULTIPLE_REGISTERS)
                        byte_swap_8(query, 13, rc);
                }
                
                // WordSwap일 때 받아온 패킷 swap
                if (wordSwap)
                {   
                    if (query[modbus_get_header_length(ctx)] == MODBUS_FC_WRITE_MULTIPLE_REGISTERS)
                        word_swap_8(query, 13, rc);
                }

                if (rc > 0) {
                    memset(content, 0, sizeof(content));
                    sprintf(content, "[%s:%d on Socket %d] RECEIVE - Function : %d (%s)  /  Value : %s",inet_ntoa(clients[master_socket].address.sin_addr), ntohs(clients[master_socket].address.sin_port), master_socket, query[modbus_get_header_length(ctx)], trans_func(query[modbus_get_header_length(ctx)]).c_str(), value(query, 9, rc));
                    write_log("INPUT_INFO", content, "/log/trans");

                    memset(content, 0, sizeof(content));
                    int offset = sprintf(content, "paket : ");
                    for (int i = 0; i < rc; i++) {
                        offset += sprintf(content + offset, "%02X ", query[i]);
                    }
                    content[offset - 1] = '\0';
                    printf("receive %s\n", content);
                    write_log("SERVER_INPUT", content, "/log/trans");
                    
                    
                    modbus_reply(ctx, query, rc, mb_mapping);


                    memset(content, 0, sizeof(content));
                    sprintf(content, "[%s:%d on Socket %d] RESPONSE - Function : %d (%s)  /  Value : %s",inet_ntoa(clients[master_socket].address.sin_addr), ntohs(clients[master_socket].address.sin_port), master_socket, query[modbus_get_header_length(ctx)], trans_func(query[modbus_get_header_length(ctx)]).c_str(), value(get_msg("INPUT"), 9, get_msgLen("INPUT")));
                    write_log("OUTPUT_INFO", content, "/log/trans");

                    memset(content, 0, sizeof(content));
                    offset = sprintf(content, "paket : ");
                    for (int i = 0; i < get_msgLen("INPUT"); i++) {
                        offset += sprintf(content + offset, "%02X ", get_msg("INPUT")[i]);
                    }
                    content[offset - 1] = '\0';
                    printf("reply %s\n", content);
                    write_log("SERVER_OUTPUT", content, "/log/trans");
                    

                } else if (rc == -1) {
                    memset(content, 0, sizeof(content));
                    snprintf(content, sizeof(content), "Connection closed on socket %d", master_socket);
                    printf("%s\n", content);
                    write_log("SYSTEM", content, "/log/system");
                    
                    close(master_socket);
                    
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

void Server::write_log(char* type, const char* content, char* topic){
    struct timeval val;
    struct tm *ptm;
    char day[11];
    char filename[15];
    char microsecond[27];
    char log[3000];
    
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

void Server::close_sigint(int sig)
{
    if (server_socket != -1) {
        close(server_socket);
    }
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);

    exit(sig);
}
