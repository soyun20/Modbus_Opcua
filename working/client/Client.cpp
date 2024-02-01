#include "Client.h"

Json::Value origin_root;
int mqtt_slave_id;
std::vector<pthread_t> threads;
int numThreads = 1;
ClientMQTT Send_Log;
ClientMQTT close_client;
int initial_connect = 1;

void * Client::ThreadFunction(void *arg) {
    Client client;
    
    if(client.Setting(&origin_root)==-1){
        return nullptr;
    }
    modbus_t* ctx = client.Connection_Initialize();

    if(client.Send_msg(ctx)==-1){
        return nullptr;
    }
    free(client.tab_rp_bits);
    free(client.tab_rp_registers);
    modbus_close(ctx);
    modbus_free(ctx);
    return nullptr;
}

void Client::mqtt_process(Json::Value* setting)
{
    Json::Value root = *setting;
	Json::Value combinedJson;
	std::vector<Json::Value> jsonArray;

    for (int i = 0; i < root["networkData"]["msgCount"].asInt(); ++i) {
        combinedJson["networkData"] = root["networkData"];
        combinedJson["msgData"] = root["msgData"][i];
		
		Json::Value mergedJson = combinedJson["networkData"];
		for (auto &key : combinedJson["msgData"].getMemberNames()) {
			mergedJson[key] = combinedJson["msgData"][key];
		}
        if(!(mergedJson.isMember("role"))){
            mergedJson["role"] = "write";
        }

		for (auto &key : mergedJson.getMemberNames()) {
			if (mergedJson[key].isString()) {
				std::string valueStr = mergedJson[key].asString();
				try {
                    if(key!="protocol" && key!="ip" && key!="name" && key!="andMask"&& key!="orMask" && key!="hexValues"){
                        int intValue = std::stoi(valueStr);
                        mergedJson[key] = intValue;
                    }
				}
				catch (const std::exception &e) {
				}
			}
		}
        jsonArray.push_back(mergedJson);
    }

    for (int i = 0; i < root["networkData"]["msgCount"].asInt(); ++i) {
        origin_root = jsonArray[i];
        pthread_t newThread;
        pthread_create(&newThread, nullptr, ThreadFunction, nullptr);
        threads.push_back(newThread);
        numThreads++;
        sleep(1);
    }
}

int Client::start(Json::Value* setting)
{
    mqtt_slave_id = 1;
    close_client.Subscribe_Initialize();
    Send_Log.Publish_Initialize();
    sleep(1);

    write_log_setting("CLIENT_START", "/modbus/log");

    mqtt_process(setting);
    

    while(1)
    {
        printf("mqtt를 받는 중....\n");
        origin_root = mqtt_client(mqtt_slave_id);
        if(close_client.is_exit){
            write_log_setting("CLIENT_DISCONNECT", "/modbus/log");
            break;
        }
        mqtt_process(&origin_root);
        mqtt_slave_id++;
    }

    return 0;
}

Json::Value Client::mqtt_client(int mqtt_slave_id)
{
    ClientMQTT client_MQTT;
    client_MQTT._sub_client_id = std::to_string(mqtt_slave_id);
	client_MQTT.Subscribe_Initialize();
	std::string setting_input;
	printf("waiting setting...\n");

    while(1)
    {
        setting_input = client_MQTT.getMessage();
        if (!setting_input.empty()) {
            std::cout << "Received message: " << setting_input << std::endl;
            break;
        }
    }

	Json::CharReaderBuilder rbuilder;
	Json::Value root;
	std::string errs;
    std::istringstream stream(setting_input);

    Json::parseFromStream(rbuilder, stream, &root, &errs);

    return root;
}

int Client::Setting(Json::Value* setting)
{
    printf("Setting start\n");
    Json::Value root = *setting;

    if(root.isMember("role")){
        role = root["role"].asString();
    }
    if(root.isMember("protocol")){
        protocol = root["protocol"].asString();
    }
    if(root.isMember("ip")){
        ip = root["ip"].asString();
    }
    if(root.isMember("port")){
        port = root["port"].asInt();
    }
    if(root.isMember("transactionDelay")){
        transactionDelay = root["transactionDelay"].asInt();
    }
    if(root.isMember("timeout")){
        timeout = root["timeout"].asInt();
    }
    if(root.isMember("name")){
        name = root["name"].asString();
    }
    if(root.isMember("slaveId")){
        slaveId = root["slaveId"].asInt();
    }
    if(root.isMember("area")){
        area = root["area"].asString();
    }
    if(root.isMember("quantity")){
        quantity = root["quantity"].asInt();
    }
    if(root.isMember("scanTime")){
        scanTime = root["scanTime"].asInt();
    }
    if(root.isMember("byteSwap")){
        byteSwap = root["byteSwap"].asInt();
    }
    if(root.isMember("wordSwap")){
        wordSwap = root["wordSwap"].asInt();
    }
    if(root.isMember("type")){
        type = root["type"].asString();
    }
    if(root.isMember("values")){
        int intValue;
        for (const Json::Value& jvalue: root["values"]){
            std::string valueStr = jvalue.asString();
            if(valueStr == "true"){
                intValue = 1;
            }
            else if(valueStr == "false"){
                intValue = 0;
            }
            else{
            intValue = std::stoi(valueStr);
            }
            values.push_back(intValue);
        }
    }
    if(root.isMember("andMask")){
        std::string AndMask = root["andMask"].asString();
        std::bitset<16> bits(AndMask);
        andMask = bits.to_ulong();
    }
    if(root.isMember("orMask")){
        std::string OrMask = root["orMask"].asString();
        std::bitset<16> bits(OrMask);
        orMask = bits.to_ulong();
    }
    if(root.isMember("readAddress")){
        readAddress = root["readAddress"].asInt();
    }
    if(root.isMember("readQuantity")){
        readQuantity = root["readQuantity"].asInt();
    }
    if(root.isMember("writeAddress")){
        writeAddress = root["writeAddress"].asInt();
    }
    if(root.isMember("hexValues")){
        hexValues = root["hexValues"].asString();
    }
    if(root.isMember("invalidFunction")){
        invalidFunction = root["invalidFunction"].asInt();
    }
    if(root.isMember("invalidLength")){
        invalidLength = root["invalidLength"].asInt();
    }
    if(root.isMember("random")){
        random = root["random"].asInt();
    }
    /*RTU*/
    if(root.isMember("comPort")){
        comPort = root["comPort"].asString();
    }
    if(root.isMember("baudrate")){
        baudrate = root["baudrate"].asInt();
    }
    if(root.isMember("dataBit")){
        dataBit = root["dataBit"].asInt();
    }
    if(root.isMember("stopBit")){
        stopBit = root["stopBit"].asInt();
    }
    if(root.isMember("parity")){
        parity = root["parity"].asString();
    }
    if(root.isMember("invalidChecksum")){
        invalidChecksum = root["invalidChecksum"].asInt();
    }
    std::cout << "\n[Setting Success]\n" << std::endl;
    return 0;
}

void Client::signal_handler(int sig)
{
    if(sig == SIGINT){
        printf("\nSIGINT received...\nShutdown\n");
        exit(0);
    }
    else if(sig == SIGPIPE){
        printf("\nSIGPIPE received...\nIgnored\n");
    }
}

modbus_t* Client::Connection_Initialize()
{
    signal(SIGINT, signal_handler);
    signal(SIGPIPE, signal_handler);
    modbus_t* ctx = nullptr;

    if (protocol.length() > 1) {
        if (protocol == "TCP") {
            use_backend = TCP;
        } else if (protocol == "RTU") {
            use_backend = RTU;
        } else {
            std::cerr << "Modbus server for unit testing." << std::endl;
            std::cerr << "Usage: " << protocol << " [tcp|tcppi|rtu] [<ip or device>]"<< std::endl;
            std::cerr << "Eg. tcp 127.0.0.1 or rtu /dev/ttyUSB0" << std::endl << std::endl;
            connection = -1;
        }
    } else {
        use_backend = TCP;
    }

    if (ip.length() > 2) {
        ip_or_device = ip.data();
    } else {
        std::string tmp;
        switch (use_backend) {
        case TCP:
            tmp = "0.0.0.0";
            ip_or_device = tmp.data();
            break;
        case RTU:
            tmp = "/dev/ttyUSB0";
            ip_or_device = tmp.data();
            break;
        default:
            break;
        }
    }

    if (use_backend == TCP) {
        ctx = modbus_new_tcp(ip_or_device, port);
    } else if (use_backend == RTU){
        //ctx = modbus_new_rtu(ip_or_device, baudrate, parity[0], dataBit, stopBit);
        ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);
    } else if (use_backend == ASCII){
        ctx = modbus_new_ascii(ip_or_device, baudrate, parity[0], dataBit, stopBit);
    }
    if (ctx == NULL) {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        connection = -1;
    }
    modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_LINK);
    modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_PROTOCOL);

    if (use_backend == TCP) {
        modbus_set_slave(ctx, slaveId);
    } else if (use_backend == RTU) {
        modbus_set_slave(ctx, slaveId);
    } else if (use_backend == ASCII) {
        modbus_set_slave(ctx, slaveId);
    }

    transaction_delay_to_sec = transactionDelay / 1000;
    transaction_delay_to_usec = (transactionDelay % 1000) * 1000;
    modbus_set_response_timeout(ctx, transaction_delay_to_sec, transaction_delay_to_usec);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        connection = -1;
    }
    else{
        printf("Connection Success\n");
    }
    
    /* Allocate and initialize the memory to store the bits */
    nb_points = (UT_BITS_NB > UT_INPUT_BITS_NB) ? UT_BITS_NB : UT_INPUT_BITS_NB;
    tab_rp_bits = (uint8_t *) malloc(nb_points * sizeof(uint8_t));
    memset(tab_rp_bits, 0, nb_points * sizeof(uint8_t));

    /* Allocate and initialize the memory to store the registers */
    nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ? UT_REGISTERS_NB
                                                          : UT_INPUT_REGISTERS_NB;
    tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
    memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

    modbus_get_response_timeout(ctx, &response_to_sec, &response_to_usec);

    timeout_to_sec = timeout;
    timeout_to_usec = 0;
    if(response_to_sec >= timeout_to_sec){
        fprintf(stderr, "Modbus communication error: Timeout exceeded\n");
    }

    if(initial_connect){
        write_log_setting("CLIENT_CONNECT", "/modbus/log");
        initial_connect = 0;
    }
    
    return ctx;
}
int Client::Send_msg(modbus_t *ctx)
{
    printf("SEND_MSG\n");

    /* Modbus Exception Code Handling*/
    if(invalidFunction == 1 ||  invalidLength == 1){
        printf("Send Invalid\n");
        if(invalidFunction == 1){
            Invalid_Function();
        }
        if(invalidLength == 1){
            Invalid_Length();
        }
    }

    /* Modbus TCP Master (READ) */
    if(role == "read"){
        printf("READ\n");
        TcpMasterRead(ctx);
        return 0;
    }

    /* Modbus TCP Master (WRITE) */
    if(role == "write"){
        printf("WRITE\n");
        TcpMasterWrite(ctx);
        return 0;
    }
    return 0;
}

void Client::TcpMasterRead(modbus_t *ctx)
{
    // scantime_to_sec = scanTime / 1000;
    // scantime_to_usec = (scanTime % 1000) * 1000;
    scantime_to_sec = scanTime;
    scantime_to_usec = 0;
    modbus_set_response_timeout(ctx, scantime_to_sec, scantime_to_usec);

    while(1)
    {
        Byte_Swap(byteSwap);
        Word_Swap(wordSwap);
        if(area=="Coil"){
            printf("Read Coils\n");
            ReadCoils(ctx);
        }
        else if(area=="DiscreteInput"){
            printf("Read Discrete Inputs\n");
            ReadDisInputs(ctx);
        }
        else if(area=="HoldingRegister"){
            printf("Read Holding Registers\n");
            ReadHoldingRegs(ctx);
        }
        else if(area=="InputRegister"){
            printf("Read Input Registers\n");
            ReadInputRegs(ctx);
        }
        usleep(scantime_to_sec* 1000000 + scantime_to_usec);
    }
}
    

void Client::TcpMasterWrite(modbus_t *ctx)
{
    std::random_device rd;
    if(type=="Write Single Coil"){
        if(random == 1){
            values[0] = rd()%2;
        }
        printf("Write Single Coil\n");
        WriteCoil(ctx);
    }
    else if(type=="Write Single Register"){
        if(random == 1){
            values[0] = rd()%30000;
        }
        printf("Write Single Register\n");
        WriteReg(ctx);
    }
    else if(type=="Write Multiple Coils"){
        printf("Write Multiple Coils\n");
        WriteMultiCoils(ctx);
    }
    else if(type=="Write Multiple Registers"){
        printf("Write Multiple Registers\n");
        WrtieMultiRegs(ctx);
    }
    else if(type=="Write Mask Register"){
        printf("Write Mask Register\n");
        WriteMaskReg(ctx);
    }
    else if(type=="Read/Write Multiple Registers"){
        if(random == 1){
            readQuantity = rd()%10;
        }
        printf("Read/Write Multiple Registers\n");
        RWMultiRegs(ctx);
    }
    else if(type=="Send Custom Hex String"){
        printf("Send Custom Hex String\n");
        SendCustomHexString(ctx);
    }
}

void Client::ReadCoils(modbus_t *ctx)
{
    /* Read Coils (Function Code 0x01) */
    rc = modbus_read_bits(ctx, readAddress, quantity, tab_rp_bits);
    if (rc == -1) {
        fprintf(stderr, "Read Coils error: %s\n", modbus_strerror(errno));
    } else {
        for (int i = 0; i < quantity; i++) {
            printf("Coil %d: %d\n", readAddress+i, tab_rp_bits[i]);
        }
    }
    write_log_setting("CLIENT_INPUT","/modbus/log");
    write_log_setting("CLIENT_OUTPUT","/modbus/log");

}

void Client::ReadDisInputs(modbus_t *ctx)
{
    /* Read Discrete Inputs (Function Code 0x02) */
    rc = modbus_read_input_bits(ctx, readAddress, quantity, tab_rp_bits);
    if (rc == -1) {
        fprintf(stderr, "Read Discrete Inputs error: %s\n", modbus_strerror(errno));
    } else {
        for (int i = 0; i < quantity; i++) {
            printf("Discrete Input %d: %d\n", readAddress+i, tab_rp_bits[i]);
        }
    }
    write_log_setting("CLIENT_INPUT","/modbus/log");
    write_log_setting("CLIENT_OUTPUT","/modbus/log");
    
}
void Client::ReadHoldingRegs(modbus_t *ctx)
{
    /* Read Holding Registers (Function Code 0x03) */
    rc = modbus_read_registers(ctx, readAddress, quantity, tab_rp_registers);
    if (rc == -1) {
        fprintf(stderr, "Read Holding Registers error: %s\n", modbus_strerror(errno));
    } else {
        for (int i = 0; i < quantity; i++) {
            printf("Holding Register %d: %d\n", readAddress+i, tab_rp_registers[i]);
        }
    }
    write_log_setting("CLIENT_INPUT","/modbus/log");
    write_log_setting("CLIENT_OUTPUT","/modbus/log");
}
void Client::ReadInputRegs(modbus_t *ctx)
{
    /* Read Input Registers (Function Code 0x04) */
    rc = modbus_read_input_registers(ctx, readAddress, quantity, tab_rp_registers);
    if (rc == -1) {
        fprintf(stderr, "Read Input Registers error: %s\n", modbus_strerror(errno));
    } else {
        for (int i = 0; i < quantity; i++) {
            printf("Input Register %d: %d\n", readAddress+i, tab_rp_registers[i]);
        }
    }
    write_log_setting("CLIENT_INPUT","/modbus/log");
    write_log_setting("CLIENT_OUTPUT","/modbus/log");
}
void Client::WriteCoil(modbus_t *ctx)
{
    /* Write Single Coil (Function Code 0x05) */
    rc = modbus_write_bit(ctx, writeAddress, values[0]);
    if (rc == -1) {
        fprintf(stderr, "Write Single Coil error: %s\n", modbus_strerror(errno));
    } else {
        printf("Write Single Coil Success\n");
    }
    write_log_setting("CLIENT_INPUT","/modbus/log");
    write_log_setting("CLIENT_OUTPUT","/modbus/log");
}
void Client::WriteReg(modbus_t *ctx)
{
    /* Write Single Register (Function Code 0x06) */
    if(byteSwap == 1){
        values[0] = ((values[0] >> 8) & 0xFF) | ((values[0] << 8) & 0xFF00);
    }
    rc = modbus_write_register(ctx, writeAddress, values[0]);
    if (rc == -1) {
        fprintf(stderr, "Write Single Register error: %s\n", modbus_strerror(errno));
    } else {
        printf("Write Single Register Success\n");
    }
    write_log_setting("CLIENT_INPUT","/modbus/log");
    write_log_setting("CLIENT_OUTPUT","/modbus/log");
}
void Client::WriteMultiCoils(modbus_t *ctx)
{
    /* Write Multiple Coils (Function Code 0x0F) */
    uint8_t tab_value[values.size()];
    for(int i=0;i<values.size();i++){
        tab_value[i] = values[i];
    }

    rc = modbus_write_bits(ctx, writeAddress, values.size(), tab_value);
    if (rc == -1) {
        fprintf(stderr, "Write Multi Coils error: %s\n", modbus_strerror(errno));
    } else {
        printf("Write Multi Coils Success\n");
    }
    write_log_setting("CLIENT_INPUT","/modbus/log");
    write_log_setting("CLIENT_OUTPUT","/modbus/log");
}
void Client::WrtieMultiRegs(modbus_t *ctx)
{
    /* Write Multiple Registers (Function Code 0x10) */
    if(byteSwap == 1){
        for(int i=0;i<values.size();i++){
            values[i] = ((values[i] >> 8) & 0xFF) | ((values[i] << 8) & 0xFF00);
        }
    }
    if(wordSwap == 1){
        for(int i=0;i<values.size()/2;i+=2){
            uint16_t tmp1 = values[i];
            values[i] = values[i+1];
            values[i+1] = tmp1;
        }
    }
    uint16_t* tab_value = values.data();

    rc = modbus_write_registers(ctx, writeAddress, values.size(), tab_value);
    if (rc == -1) {
        fprintf(stderr, "Write Multiple Registers error: %s\n", modbus_strerror(errno));
    } else {
        printf("Write Multiple Registers Success\n");
    }
    write_log_setting("CLIENT_INPUT","/modbus/log");
    write_log_setting("CLIENT_OUTPUT","/modbus/log");
}
void Client::WriteMaskReg(modbus_t *ctx)
{
    /* Mask Write Register (Function Code 0x16) */
    //(current value AND 'and') OR ('or' AND (NOT 'and'))
    rc = modbus_write_register(ctx, writeAddress, 18);
    rc = modbus_mask_write_register(ctx, writeAddress, andMask, orMask);
    rc = modbus_read_registers(ctx, writeAddress, 1, tab_rp_registers);
    if (rc == -1) {
        fprintf(stderr, "Write Mask Register error: %s\n", modbus_strerror(errno));
    } else {
        for (int i = 0; i < 1; i++) {
            printf("Write Mask Register %d: %d\n", i, tab_rp_registers[i]);
        }
    }
    write_log_setting("CLIENT_INPUT","/modbus/log");
    write_log_setting("CLIENT_OUTPUT","/modbus/log");
}
void Client::RWMultiRegs(modbus_t *ctx)
{
    /* Read/Write Multiple Registers (Function Code 0x17) */

    /* Write Multiple Registers (Function Code 0x10) */
    uint16_t* tab_value = values.data();

    rc = modbus_write_registers(ctx, writeAddress, values.size(), tab_value);
    if (rc == -1) {
        fprintf(stderr, "Write Multiple Registers error: %s\n", modbus_strerror(errno));
    } else {
        printf("Write Multiple Registers Success\n");
    }

    /* Read Holding Registers (Function Code 0x03) */
    rc = modbus_read_registers(ctx, readAddress, readQuantity, tab_rp_registers);
    if (rc == -1) {
        fprintf(stderr, "Read Holding Registers error: %s\n", modbus_strerror(errno));
    } else {
        for (int i = 0; i < readQuantity; i++) {
            printf("Holding Register %d: %d\n", i, tab_rp_registers[i]);
        }
    }
    write_log_setting("CLIENT_INPUT","/modbus/log");
    write_log_setting("CLIENT_OUTPUT","/modbus/log");
}
void Client::SendCustomHexString(modbus_t *ctx)
{
    int length = hexValues.length()/2;
    uint8_t raw_req[length];
    int idx = 0;
    for (size_t i = 0; i < hexValues.length(); i += 2) {
        std::string tmp = "0x" + hexValues.substr(i, 2);
        uint8_t packet = static_cast<uint8_t>(std::stoi(tmp, nullptr, 16));
        raw_req[idx]=packet;
        idx++;
    }
    printf("raw_request: \n");
    for(int i=0;i<sizeof(raw_req);i++){
        printf("%02x ",raw_req[i]);
    }
    printf("\n");
    rc = modbus_send_raw_request(ctx, raw_req, sizeof(raw_req));
    if (rc == -1) {
        fprintf(stderr, "Failed to send the request packet: %s\n", modbus_strerror(errno));
    }
    write_log_setting("CLIENT_INPUT","/modbus/log");

    rc = modbus_receive_confirmation(ctx, rsp);
    if (rc == -1) {
        fprintf(stderr, "Failed to receive confirmation: %s\n", modbus_strerror(errno));
    }
    write_log_setting("CLIENT_OUTPUT","/modbus/log");
}

void Client::write_log_setting(std::string stype, std::string stopic){
    const char* type = stype.c_str();
    const char* topic = stopic.c_str();
    uint8_t* query;
    int length;

    if(!strcmp(type,"CLIENT_INPUT")){
        query = get_msg("INPUT");
        length = get_msgLen("INPUT");
    }
    else if(!strcmp(type,"CLIENT_OUTPUT")){
        query = get_msg("OUTPUT");
        length = get_msgLen("OUTPUT");
    }

    if(!strcmp(type, "CLIENT_START")){
        char content[1024];
        sprintf(content, "Client Start");
        write_log(type, topic, content);
    }
    else if(!strcmp(type,"CLIENT_CONNECT")){
        char content1[1024];
        char content2[1024];
        sprintf(content1, "setting content : Protocol: %s, IP: %s, Transaction Delay: %d, Timeout: %d",protocol.c_str(), ip.c_str(), transactionDelay, timeout);
        write_log(type, topic, content1);
        if(connection == 0){
            sprintf(content2, "Connection to '%s' was successful.",ip.c_str());
        }
        else{
            sprintf(content2, "Connection to '%s' was failed.",ip.c_str());
        }
        write_log(type, topic, content2);
    }
    else if(!strcmp(type,"CLIENT_DISCONNECT")){
        char content[1024];
        sprintf(content, "Disconnected");
        write_log(type, topic, content);
    }
    else{
        char content[length*5];
        int offset = sprintf(content, "paket : ");

        for (int i = 0; i < length; i++) {
            offset += sprintf(content + offset, "%02X ", query[i]);
        }
        content[offset - 1] = '\0';
        printf("%s\n", content);
        write_log(type, topic, content);
    }
}
void Client::write_log(const char* type, const char* topic, char* content){
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

    std::cout << "topic:" << topic << " log: " <<log <<std::endl;
    
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

// Modbus ASCII 프레임 생성 함수
void createModbusASCIIFrame(char* frame, int slaveAddr, int functionCode, int startAddr, int numRegisters) {
    // 프레임을 구성하는 각 필드의 값을 문자열로 변환하여 프레임 문자열에 추가합니다.
    sprintf(frame, ":%02X%02X%04X%04X%02X", slaveAddr, functionCode, startAddr >> 8, startAddr & 0xFF, numRegisters);

    // 체크섬 계산
    int checksum = 0;
    for (int i = 1; i < 13; i += 2) {
        int value;
        sscanf(&frame[i], "%2X", &value);
        checksum += value;
    }
    checksum = (0xFF - checksum) + 1;

    // 체크섬 값을 문자열로 추가합니다.
    sprintf(&frame[13], "%02X\r\n", checksum);
}

/*

// Modbus ASCII 클라이언트 역할 수행

    // 예: 주소 1에서 5개의 레지스터 읽기 (Modbus ASCII 프레임 생성 및 파싱)
    char frame[32];
    createModbusASCIIFrame(frame, 1, 3, 1, 5); // 주소 1에서 5개의 레지스터를 읽는 프레임 생성

    // 프레임 전송
    int numBytesSent = modbus_send_raw_request(ctx, frame, strlen(frame));
    if (numBytesSent == -1) {
        std::cerr << "Failed to send Modbus ASCII frame." << std::endl;
    } else {
        // 응답 수신 및 파싱
        char response[256];
        int numBytesReceived = modbus_receive_confirmation(ctx, response, sizeof(response));
        if (numBytesReceived == -1) {
            std::cerr << "Failed to receive Modbus ASCII response." << std::endl;
        } else {
            // 응답 데이터 파싱
            // 파싱 로직을 추가해야 합니다.
            std::cout << "Received response: " << response << std::endl;
        }
    }

*/