#include "opcuaServer.h"

std::string id;
ClientMQTT Send_Log;

void replaceString(std::string &content) {
    size_t found = content.find("\u0009");
    while (found != std::string::npos) {
        content.replace(found, 1, "    ");
        found = content.find("\u0009", found + 1);
    }

    size_t found2 = content.find('"');
    while (found2 != std::string::npos) {
        content.replace(found2, 1, " ");
        found2 = content.find('"', found2 + 1);
    }
}

void write_log(std::string stype, std::string stopic, char* content){
    const char* type = stype.c_str();
    const char* topic = stopic.c_str();

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char server[40] = "10.7.12.143";
    char *user = "root";
    char *password = "root";
    char *database = "LOG";
    char *table = "Server_Log";
    char SQL_query[1024];
    char SQL_query_send_packet[1000000];
    char SQL_query_recv_packet[1000000];
    uint8_t* query;

    char filename[15];
    char microsecond[27];
    char log[1000];
    char packetLog[1000000];

    struct timeval val;
    struct tm *ptm;

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

    std::string contentStr(content);
    replaceString(contentStr);

    sprintf(log, "{\"id\" : \"%s\", \"time\" : \"%s\", \"type\" : \"%s\", \"content\" : \"%s\"}", id.c_str(), microsecond, type, contentStr.c_str());

    std::cout << "topic:" << topic << " log: " <<log <<std::endl;
    
    Send_Log.Publish(topic, log);


    std::string packet_send;
    std::string packet_recv;

    uint8_t* recvBuf = NULL;
    int recvBufLen;
    uint8_t* sendBuf = NULL;
    int sendBufLen;

    recvBuf = GetRecvMsg();
    recvBufLen = GetRecvMsgLen();

    sendBuf = GetSendMsg();
    sendBufLen = GetSendMsgLen();

    char *packetRecv = "OPCUA_SEND";
    char *packetSend = "OPCUA_RECEIVE";

    if(recvBufLen != 0){
        for (size_t i = 0; i < recvBufLen; ++i) {
            char hex[4];
            snprintf(hex, sizeof(hex), "%02X ", recvBuf[i]);
            packet_recv += hex;
        }

        sprintf(packetLog, "{\"id\" : \"%s\", \"time\" : \"%s\", \"type\" : \"%s\", \"content\" : \"%s\"}", id.c_str(), microsecond, type, packet_recv.c_str());
        std::cout << "topic:" << topic << " packetLog: " <<packetLog <<std::endl;
        Send_Log.Publish(topic, packetLog);
    }

    if(sendBufLen != 0){
        for (size_t i = 0; i < sendBufLen; ++i) {
            char hex[4];
            snprintf(hex, sizeof(hex), "%02X ", sendBuf[i]);
            packet_send += hex;
        }

        sprintf(packetLog, "{\"id\" : \"%s\", \"time\" : \"%s\", \"type\" : \"%s\", \"content\" : \"%s\"}", id.c_str(), microsecond, type, packet_send.c_str());
        std::cout << "topic:" << topic << " packetLog: " <<packetLog <<std::endl;
        Send_Log.Publish(topic, packetLog);
    }

    snprintf(filename + strlen(filename), sizeof(filename) - strlen(filename), ".log");
    FILE *file = fopen(filename, "a");
    if (!file) {
        perror("Failed to open log file");
        return;
    }
    fprintf(file, "[%s] [%s] %s\n", microsecond, type, contentStr.c_str());
    if(recvBufLen != 0){
        fprintf(file, "[%s] [%s] %s\n", microsecond, packetRecv, packet_recv.c_str());
    }
    if(sendBufLen != 0){
        fprintf(file, "[%s] [%s] %s\n", microsecond, packetSend, packet_send.c_str());
    }
    fclose(file);

    conn = mysql_init(NULL);

    if (mysql_real_connect(conn, server, user, password, database, 3306, NULL, 0) == NULL) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }

    snprintf(SQL_query, sizeof(SQL_query), "INSERT INTO %s (time, type, content) VALUES ('%s', '%s', '%s')", table, microsecond, type, contentStr.c_str());
    if(recvBufLen != 0){
        snprintf(SQL_query_recv_packet, sizeof(SQL_query_recv_packet), "INSERT INTO %s (time, type, content) VALUES ('%s', '%s', '%s')", table, microsecond, packetRecv, packet_recv.c_str());
    }
    if(sendBufLen != 0){
        snprintf(SQL_query_send_packet, sizeof(SQL_query_send_packet), "INSERT INTO %s (time, type, content) VALUES ('%s', '%s', '%s')", table, microsecond, packetSend, packet_send.c_str());
    }


    if (mysql_query(conn, SQL_query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
    }
    if (recvBufLen != 0 && mysql_query(conn, SQL_query_recv_packet)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
    }
    if (sendBufLen != 0 && mysql_query(conn, SQL_query_send_packet)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
    }

    mysql_close(conn);
}

void customLogger(void *logContext, UA_LogLevel level, UA_LogCategory category, const char *msg, va_list args) {
    char logMessage[2048]; 
    vsnprintf(logMessage, sizeof(logMessage), msg, args);
    write_log("OPCUA_LOG", "/log/trans",logMessage);
}

std::string extractAndStore(const std::string& route) {
    size_t lastSlashPos = route.find_last_of('/');
    
    if (lastSlashPos != std::string::npos) {
        std::string extractedString = route.substr(0, lastSlashPos);
        return extractedString;
    }
    
    return route;
}

UA_StatusCode AddInt16Method(UA_Server *server,
                             const UA_NodeId *sessionId, void *sessionContext,
                             const UA_NodeId *methodId, void *methodContext,
                             const UA_NodeId *objectId, void *objectContext,
                             size_t inputSize, const UA_Variant *input,
                             size_t outputSize, UA_Variant *output) {

    if (inputSize != 2 || outputSize != 1) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    UA_Int16 inputValue1 = *(UA_Int16*)input[0].data;
    UA_Int16 inputValue2 = *(UA_Int16*)input[1].data;

    UA_Int16 result = inputValue1 + inputValue2;

    UA_Variant_setScalarCopy(output, &result, &UA_TYPES[UA_TYPES_INT16]);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
helloWorld(UA_Server *server,
           const UA_NodeId *sessionId, void *sessionContext,
           const UA_NodeId *methodId, void *methodContext,
           const UA_NodeId *objectId, void *objectContext,
           size_t inputSize, const UA_Variant *input,
           size_t outputSize, UA_Variant *output) {
    /* input is a scalar string (checked by the server) */
    UA_String *name = (UA_String *)input[0].data;
    UA_String hello = UA_STRING("Hello ");
    UA_String greet;
    greet.length = hello.length + name->length;
    greet.data = (UA_Byte *)UA_malloc(greet.length);
    memcpy(greet.data, hello.data, hello.length);
    memcpy(greet.data + hello.length, name->data, name->length);
    UA_Variant_setScalarCopy(output, &greet, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&greet);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
noargMethod(UA_Server *server,
            const UA_NodeId *sessionId, void *sessionContext,
            const UA_NodeId *methodId, void *methodContext,
            const UA_NodeId *objectId, void *objectContext,
            size_t inputSize, const UA_Variant *input,
            size_t outputSize, UA_Variant *output) {
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
outargMethod(UA_Server *server,
             const UA_NodeId *sessionId, void *sessionContext,
             const UA_NodeId *methodId, void *methodContext,
             const UA_NodeId *objectId, void *objectContext,
             size_t inputSize, const UA_Variant *input,
             size_t outputSize, UA_Variant *output) {
    UA_Int32 out = 42;
    UA_Variant_setScalarCopy(output, &out, &UA_TYPES[UA_TYPES_INT32]);
    return UA_STATUSCODE_GOOD;
}

static const char *dataTypeToString(const UA_DataType *type) {
    if (type == NULL) {
        return "UA_TYPES_BOOLEAN";
    } else if (type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
        return "UA_TYPES_BOOLEAN";
    } else if (type == &UA_TYPES[UA_TYPES_SBYTE]) {
        return "UA_TYPES_SBYTE";
    } else if (type == &UA_TYPES[UA_TYPES_BYTE]) {
        return "UA_TYPES_BYTE";
    } else if (type == &UA_TYPES[UA_TYPES_INT16]) {
        return "UA_TYPES_INT16";
    } else if (type == &UA_TYPES[UA_TYPES_UINT16]) {
        return "UA_TYPES_UINT16";
    } else if (type == &UA_TYPES[UA_TYPES_INT32]) {
        return "UA_TYPES_INT32";
    } else if (type == &UA_TYPES[UA_TYPES_UINT32]) {
        return "UA_TYPES_UINT32";
    } else if (type == &UA_TYPES[UA_TYPES_STATUSCODE]) {
        return "UA_TYPES_STATUSCODE";
    } else if (type == &UA_TYPES[UA_TYPES_INT64]) {
        return "UA_TYPES_INT64";
    } else if (type == &UA_TYPES[UA_TYPES_UINT64]) {
        return "UA_TYPES_UINT64";
    } else if (type == &UA_TYPES[UA_TYPES_DATETIME]) {
        return "UA_TYPES_DATETIME";
    } else if (type == &UA_TYPES[UA_TYPES_FLOAT]) {
        return "UA_TYPES_FLOAT";
    } else if (type == &UA_TYPES[UA_TYPES_DOUBLE]) {
        return "UA_TYPES_DOUBLE";
    } else if (type == &UA_TYPES[UA_TYPES_STRING]) {
        return "UA_TYPES_STRING";
    } else if (type == &UA_TYPES[UA_TYPES_BYTESTRING]) {
        return "UA_TYPES_BYTESTRING";
    } else if (type == &UA_TYPES[UA_TYPES_XMLELEMENT]) {
        return "UA_TYPES_XMLELEMENT";
    } else {
        return "Unknown Type";
    }
}

static void dataChangeNotificationCallback(UA_Server *server, UA_UInt32 monitoredItemId,
                                           void *monitoredItemContext, const UA_NodeId *nodeId,
                                           void *nodeContext, UA_UInt32 attributeId,
                                           const UA_DataValue *value) {

    const char *dataTypeStr = dataTypeToString(value->value.type);

    Json::Value jsonRoot;
    jsonRoot["DataType"] = dataTypeStr;

    if (value->hasValue) {
        if (value->value.type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
            UA_Boolean *Value = (UA_Boolean *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_SBYTE]) {
            UA_SByte *Value = (UA_SByte *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_BYTE]) {
            UA_Byte *Value = (UA_Byte *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_INT16]) {
            UA_Int16 *Value = (UA_Int16 *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_UINT16]) {
            UA_UInt16 *Value = (UA_UInt16 *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_INT32]) {
            UA_Int32 *Value = (UA_Int32 *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_UINT32]) {
            UA_UInt32 *Value = (UA_UInt32 *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_STATUSCODE]) {
            UA_StatusCode *Value = (UA_StatusCode *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_INT64]) {
            UA_Int64 *Value = (UA_Int64 *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_UINT64]) {
            UA_UInt64 *Value = (UA_UInt64 *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_DATETIME]) {
            UA_DateTime *Value = (UA_DateTime *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_FLOAT]) {
            UA_Float *Value = (UA_Float *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            UA_Double *Value = (UA_Double *)value->value.data;
            jsonRoot["Value"] = *Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_STRING]) {
            UA_String *Value = (UA_String *)value->value.data;
            jsonRoot["Value"] = Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_BYTESTRING]) {
            UA_ByteString *Value = (UA_ByteString *)value->value.data;
            jsonRoot["Value"] = Value;
        } else if (value->value.type == &UA_TYPES[UA_TYPES_XMLELEMENT]) {
            UA_XmlElement *Value = (UA_XmlElement *)value->value.data;
            jsonRoot["Value"] = Value;
        } else {
            printf("Value type is not recognized\n");
            jsonRoot["Value"] = "N/A";
        }
    }
    else {
        printf("No value available for this node\n");
        jsonRoot["Value"] = "N/A";
    }
    Json::StreamWriterBuilder writer;
    std::string jsonStr = Json::writeString(writer, jsonRoot);

    printf("%s\n", jsonStr.c_str());
    
}

static void
addMonitoredItemToCurrentTimeVariable(UA_Server *server, std::string route) {
    UA_NodeId currentTimeNodeId = UA_NODEID_STRING_ALLOC(1, route.c_str());
    UA_MonitoredItemCreateRequest monRequest = UA_MonitoredItemCreateRequest_default(currentTimeNodeId);
    monRequest.requestedParameters.samplingInterval = 100.0; /* 100 ms interval */
    UA_Server_createDataChangeMonitoredItem(server, UA_TIMESTAMPSTORETURN_SOURCE,
                                            monRequest, NULL,
                                            dataChangeNotificationCallback);
}

int OpcuaServer::nodeSetting(Json::Value* setting)
{
    printf("Node setting start\n");
    

    Json::Value root = *setting;

    if(root.isMember("route")){
        route = root["route"].asString();
    }
    if(root.isMember("label")){
        label = root["label"].asString();
    }
    if(root.isMember("category")){
        category = root["category"].asString();
    }
    if(root.isMember("type")){
        type = root["type"].asString();
    }
    if(root.isMember("accessRight")){
        accessRight = root["accessRight"].asString();
    }
    if(root.isMember("inputArguments")){
        inArguments.clear();
        for (const Json::Value& input: root["inputArguments"]){
            std::vector<std::string> input_argument;
            input_argument.push_back(input["name"].asString());
            input_argument.push_back(input["dataType"].asString());
            inArguments.push_back(input_argument);
        }
    }
    if(root.isMember("outputArguments")){
        outArguments.clear();
        for (const Json::Value& output: root["outputArguments"]){
            std::vector<std::string> output_argument;
            output_argument.push_back(output["name"].asString());
            output_argument.push_back(output["dataType"].asString());
            outArguments.push_back(output_argument);
        }
    }
    std::cout << "[Node Setting Success]\n" << std::endl;
    return 0;
}

int OpcuaServer::start(Json::Value* setting)
{
    printf("start\n");

    UA_Server *server = UA_Server_new(); //서버 객체 생성
    UA_ServerConfig *config = UA_Server_getConfig(server);
    UA_StatusCode res = UA_STATUSCODE_GOOD;
    UA_ServerConfig_setDefault(config);

    config->maxSubscriptions = 100;
    config->lifeTimeCountLimits.min = 100;

    Json::Value root = *setting;
    Json::Value OUSNetworkData = root["ousNetworkData"];
    Json::Value OUSMemoryTreeData = root["ousMemoryTreeData"];

    if(root.isMember("id")){
        id = root["id"].asString();
    }
    if(OUSNetworkData.isMember("ip")){
        ip = OUSNetworkData["ip"].asString();
    }
    if(OUSNetworkData.isMember("port")){
        port = OUSNetworkData["port"].asInt();
    }
    if(OUSNetworkData.isMember("certFile")){
        certFile = OUSNetworkData["certFile"].asString();
    }
    if(OUSNetworkData.isMember("keyFile")){
        keyFile = OUSNetworkData["keyFile"].asString();
    }
    if(OUSNetworkData.isMember("users")){
        for (const Json::Value& juser: OUSNetworkData["users"]){
            std::vector<std::string> user;
            user.push_back(juser["id"].asString());
            user.push_back(juser["pw"].asString());
            users.push_back(user);
        }
    }

    char log_text[1024];
    Send_Log._pub_client_id = id+"_pub_server_send_log";
    Send_Log.Publish_Initialize();
    close_server._sub_client_id = id+"_sub_server_close_server";
    close_server.Subscribe_Initialize();
    sleep(3);

    UA_Logger myLogger;
    myLogger.log = customLogger;
    config->logger = myLogger;

    write_log("OPCUA_INFO", "/log/system","System Started");

    // setting port number
    if(OUSNetworkData.isMember("port")){

        UA_EndpointDescription *endpoints = (UA_EndpointDescription *)UA_Array_new(1, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
        config->endpoints = endpoints;
        config->endpointsSize = 1;

        UA_EndpointDescription *endpoint = &config->endpoints[0];
        UA_EndpointDescription_init(endpoint);

        std::string url = "opc.tcp://"+ip+":%d",port;
        endpoint->endpointUrl = UA_STRING_ALLOC(url.c_str());
    }


    // if(OUSNetworkData.isMember("port")){
    //     res = UA_ServerConfig_setMinimal(config, port, NULL);
    //     if (res != UA_STATUSCODE_GOOD) {
    //         printf("Failed to run the server.\n");
    //         UA_Server_delete(server);
    //         return EXIT_FAILURE;
    //     }
    // }

    sprintf(log_text, "설정 내용 : IP: %s, Port: %d", ip.c_str(), port);
    write_log("OPCUA_INFO", "/log/trans",log_text);
    write_log("OPCUA_INFO", "/log/system","Operation-Start");


    // setting certFile and keyFile
    if(OUSNetworkData.isMember("certFile") && OUSNetworkData.isMember("keyFile")){
        UA_ByteString certificate = UA_STRING_ALLOC(certFile.c_str());
        UA_ByteString privateKey = UA_STRING_ALLOC(keyFile.c_str());
        UA_ServerConfig_setDefaultWithSecurityPolicies(config, port, &certificate, &privateKey, NULL, 0, NULL, 0, NULL, 0);
        UA_ServerConfig_addAllEndpoints(config);
    }

    // if(OUSNetworkData.isMember("certFile") && OUSNetworkData.isMember("keyFile")){
    //     UA_ByteString certificate = UA_BYTESTRING_NULL;
    //     UA_ByteString privateKey = UA_BYTESTRING_NULL;
    //     certificate = loadFile(certFile.c_str());
    //     privateKey = loadFile(keyFile.c_str());
    //     UA_ServerConfig_setDefaultWithSecurityPolicies(config, port, &certificate, &privateKey, NULL, 0, NULL, 0, NULL, 0);
    //     UA_ServerConfig_addAllEndpoints(config);
    // }

    //setting users
    if(OUSNetworkData.isMember("users")){
        size_t usernamePasswordLoginSize = users.size();
        if(usernamePasswordLoginSize!=0){
            UA_UsernamePasswordLogin *userNamePW = (UA_UsernamePasswordLogin *)malloc(usernamePasswordLoginSize * sizeof(UA_UsernamePasswordLogin));
            for(int i=0;i<usernamePasswordLoginSize;i++){
                char *name = const_cast<char *>(users[i][0].c_str());
                char *pw = const_cast<char *>(users[i][1].c_str());
                userNamePW[i].username = UA_STRING(name);
                userNamePW[i].password = UA_STRING(pw);
            }
            UA_Boolean allowAnonymous = false;
            UA_String encryptionPolicy = config->securityPolicies[config->securityPoliciesSize-1].policyUri;
            config->accessControl.clear(&config->accessControl);
            UA_AccessControl_default(config, allowAnonymous, &encryptionPolicy, usernamePasswordLoginSize, userNamePW);

            write_log("OPCUA_INFO/SERVER", "/log/trans","AccessControl: Anonymous login is disabled");
        } else{
            UA_Boolean allowAnonymous = true;
            write_log("OPCUA_INFO/SERVER", "/log/trans","AccessControl: Anonymous login is enabled");
        }
    }
    sprintf(log_text, "TCP network layer listening on opc.tcp://%s:%d/", ip.c_str(), port);
    write_log("OPCUA_INFO/NETWORK", "/log/trans",log_text);

    UA_NodeId outNodeId;
    UA_HistorizingNodeIdSettings hisSetting;
    for(int i=0;i<OUSNetworkData["nodeCount"].asInt();i++){

        nodeSetting(&OUSMemoryTreeData[i]);
        std::string parent = extractAndStore(route);
        UA_NodeId parentNodeId;
        char *path = const_cast<char *>(route.c_str());

        if(parent == "Object"){
            parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
        }
        else{
            parentNodeId = UA_NODEID_STRING_ALLOC(1, parent.c_str());
        }

        if(category == "Folder"){
            UA_ObjectAttributes object_attr = UA_ObjectAttributes_default;
            object_attr.description = UA_LOCALIZEDTEXT("en-US", path);
            object_attr.displayName = UA_LOCALIZEDTEXT("en-US", path);

            UA_Server_addObjectNode(server, UA_NODEID_STRING_ALLOC(1, path),
                                    parentNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                    UA_QUALIFIEDNAME(1, path),
                                    UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), object_attr, NULL, NULL);
            
            sprintf(log_text, "[Add Folder Node] name: %s", path);
            write_log("OPCUA_INFO", "/log/trans",log_text);
        } else if(category == "Variable"){
            // address space에 노드 추가
            UA_VariableAttributes attr;
            UA_NodeId myValueNodeId;
            UA_QualifiedName myValueName;
            UA_HistoryDataGathering gathering = UA_HistoryDataGathering_Default(1);
            config->historyDatabase = UA_HistoryDatabase_default(gathering);

            // adress space에 변수 노드 추가
            attr = UA_VariableAttributes_default; //변수 노드의 속성 초기화
            if(type=="NULL" || type=="Boolean"){
                UA_Boolean myValue = 0;
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_BOOLEAN]);
            } else if(type=="SByte"){
                UA_SByte myValue = 0;
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_SBYTE]);
            } else if(type=="Byte"){
                UA_Byte myValue = 0;
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_BYTE]);
            } else if(type=="Int16"){
                UA_Int16 myValue = 0;
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_INT16]);
            } else if(type=="UInt16"){
                UA_UInt16 myValue = 0;
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_UINT16]);
            } else if(type=="Int32"){
                UA_Int32 myValue = 0;
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_INT32]);
            } else if(type=="UInt32"){
                UA_UInt32 myValue = 0;
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_UINT32]);
            } else if(type=="StatusCode"){
                UA_StatusCode myValue = 0;
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_STATUSCODE]);
            } else if(type=="Int64"){
                UA_Int64 myValue = 0;
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_INT64]);
            } else if(type=="DateTime"){
                UA_DateTime myValue = 0;
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_DATETIME]);
            } else if(type=="Float"){
                UA_Float myValue = 0;
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_FLOAT]);
            } else if(type=="Double"){
                UA_Double myValue = 0;
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_DOUBLE]);
            } else if(type=="String"){
                UA_String myValue = UA_STRING("");
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_STRING]);
            } else if(type=="ByteString"){
                UA_ByteString myValue = UA_BYTESTRING("");
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_BYTESTRING]);
            } else if(type=="XmlElement"){
                UA_XmlElement myValue = UA_STRING("");
                UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_XMLELEMENT]);
            }

            attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US",route.c_str()); //속성을 영문으로 설정
            attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US",route.c_str());

            // 변수 노드의 속성을 수정하여 읽기/쓰기 변경
            if(accessRight == "Read & Write"){
                attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE | UA_ACCESSLEVELMASK_HISTORYREAD | UA_ACCESSLEVELMASK_HISTORYWRITE;
                attr.userAccessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE | UA_ACCESSLEVELMASK_HISTORYREAD | UA_ACCESSLEVELMASK_HISTORYWRITE;
            } else if(accessRight == "ReadOnly"){
                attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_HISTORYREAD | UA_ACCESSLEVELMASK_HISTORYWRITE;
                attr.userAccessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_HISTORYREAD | UA_ACCESSLEVELMASK_HISTORYWRITE;
            }
            attr.historizing = true;
            
            UA_NodeId_init(&outNodeId);

            myValueNodeId = UA_NODEID_STRING(1, path); //Node ID 설정
            myValueName = UA_QUALIFIEDNAME(1, path); //변수 노드 이름 설정
  
            UA_Server_addVariableNode(server, myValueNodeId, parentNodeId,
                                    UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), myValueName,
                                    UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, &outNodeId); // 서버에 변수 노드 추가

            hisSetting.historizingBackend = UA_HistoryDataBackend_Memory(3, 100);
            hisSetting.maxHistoryDataResponseSize = 100;
            hisSetting.historizingUpdateStrategy = UA_HISTORIZINGUPDATESTRATEGY_VALUESET;
            res = gathering.registerNodeId(server, gathering.context, &outNodeId, hisSetting);

            // 할당된 메모리 해제
            // UA_VariableAttributes_clear(&attr);
            // UA_NodeId_clear(&myValueNodeId);
            // UA_QualifiedName_clear(&myValueName);

            addMonitoredItemToCurrentTimeVariable(server, route);

            sprintf(log_text, "[Add Variable Node] name: %s, data type: %s", path, type.c_str());
            write_log("OPCUA_INFO", "/log/trans",log_text);
        } else if(category == "Method"){
            
            if(type == "String"){
                UA_Argument inputArguments;
                UA_Argument_init(&inputArguments);
                UA_Argument outputArguments;
                UA_Argument_init(&outputArguments);

                std::string inputArg = inArguments[0][0];
                std::string outputArg = outArguments[0][0];
                std::string inputArgType = inArguments[0][1];
                std::string outputArgType = outArguments[0][1];

                char *input = const_cast<char *>(inputArg.c_str());
                char *output = const_cast<char *>(outputArg.c_str());

                
                if(inputArgType == "UA_String"){
                    inputArguments.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
                }
                inputArguments.description = UA_LOCALIZEDTEXT("en-US", "Say your name");
                inputArguments.name = UA_STRING(input);
                inputArguments.valueRank = UA_VALUERANK_SCALAR;

                outputArguments.arrayDimensionsSize = 0;
                outputArguments.arrayDimensions = NULL;
                if(outputArgType == "UA_String"){
                    outputArguments.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
                }
                outputArguments.description = UA_LOCALIZEDTEXT("en-US", "Receive a greeting");
                outputArguments.name = UA_STRING(output);
                outputArguments.valueRank = UA_VALUERANK_SCALAR;

                UA_MethodAttributes addmethodattributes = UA_MethodAttributes_default;
                addmethodattributes.displayName = UA_LOCALIZEDTEXT("en-US", path);
                addmethodattributes.executable = true;
                addmethodattributes.userExecutable = true;
                UA_Server_addMethodNode(server, UA_NODEID_STRING_ALLOC(1, route.c_str()),
                                        parentNodeId,
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                        UA_QUALIFIEDNAME(1, path), addmethodattributes,
                                        &helloWorld,
                                        1, &inputArguments, 1, &outputArguments, NULL, NULL);
                
                sprintf(log_text, "[Add Method Node] name: %s, data type: %s, method: hello name", path, type.c_str());
                write_log("OPCUA_INFO", "/log/trans",log_text);
            }
            else if(type == "Int16"){
                UA_Argument inputArguments[2];
                UA_Argument_init(&inputArguments[0]);
                UA_Argument_init(&inputArguments[1]);
                UA_Argument outputArguments;
                UA_Argument_init(&outputArguments);
                
                std::string inputArg1 = inArguments[0][0];
                std::string inputArg2 = inArguments[1][0];
                std::string outputArg = outArguments[0][0];
                std::string inputArgType1 = inArguments[0][1];
                std::string inputArgType2 = inArguments[1][1];
                std::string outputArgType = outArguments[0][1];

                char *input1 = const_cast<char *>(inputArg1.c_str());
                char *input2 = const_cast<char *>(inputArg2.c_str());
                char *output = const_cast<char *>(outputArg.c_str());


                if(inputArgType1 == "UA_Int16"){
                    inputArguments[0].dataType = UA_TYPES[UA_TYPES_INT16].typeId;
                }
                inputArguments[0].description = UA_LOCALIZEDTEXT("en-US", "a_value");
                inputArguments[0].name = UA_STRING(input1);
                inputArguments[0].valueRank = UA_VALUERANK_SCALAR;

                if(inputArgType2 == "UA_Int16"){
                    inputArguments[1].dataType = UA_TYPES[UA_TYPES_INT16].typeId;
                }
                inputArguments[1].description = UA_LOCALIZEDTEXT("en-US", "b_value");
                inputArguments[1].name = UA_STRING(input2);
                inputArguments[1].valueRank = UA_VALUERANK_SCALAR;

                outputArguments.arrayDimensionsSize = 0;
                outputArguments.arrayDimensions = NULL;
                if(outputArgType == "UA_Int16"){
                    outputArguments.dataType = UA_TYPES[UA_TYPES_INT16].typeId;
                }
                outputArguments.description = UA_LOCALIZEDTEXT("en-US", "add two number");
                outputArguments.name = UA_STRING("output");
                outputArguments.valueRank = UA_VALUERANK_SCALAR;

                UA_MethodAttributes methodAttr = UA_MethodAttributes_default;
                methodAttr.executable = true;
                methodAttr.userExecutable = true;
                UA_Server_addMethodNode(server, 
                                        UA_NODEID_STRING_ALLOC(1, route.c_str()), 
                                        parentNodeId,
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                        UA_QUALIFIEDNAME(1, path), methodAttr,
                                        &AddInt16Method,
                                        2, inputArguments, 1, &outputArguments, NULL, NULL);

                sprintf(log_text, "[Add Method Node] name: %s, data type: %s, method: add two variable", path, type.c_str());
                write_log("OPCUA_INFO", "/log/trans",log_text);
            }
        }
    }
    
    std::cout << "[Setting Success]\n" << std::endl;
    
    res = UA_Server_run_startup(server);
    UA_CHECK_STATUS(res, return res);

    while(1){
        if(close_server.is_exit == id){
            break;
        }      
        UA_Server_run_iterate(server, true);
    }

    res = UA_Server_run_shutdown(server);
    write_log("OPCUA_INFO", "/log/system","Operation-Stop");

    if (res != UA_STATUSCODE_GOOD) {
        printf("Failed to run the server.\n");
        UA_Server_delete(server);
        return EXIT_FAILURE;
    }

    UA_Server_delete(server);
    UA_NodeId_clear(&outNodeId);
    hisSetting.historizingBackend.deleteMembers(&hisSetting.historizingBackend);
    return res == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;

    return 0;

}

