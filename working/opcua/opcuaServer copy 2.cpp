#include "opcuaServer.h"

int GetSecondLastNumberFromRoute(const std::string& route, std::string& parent) {
    // 문자열을 '/'를 기준으로 분할
    std::vector<std::string> parts;
    std::istringstream ss(route);
    std::string part;
    while (std::getline(ss, part, '/')) {
        parts.push_back(part);
    }

    // 분할된 문자열 중 뒤에서 두 번째 요소를 숫자로 파싱
    if (parts.size() >= 2) {
        parent =  parts[parts.size() - 2];
    } else{
        return -1;
    }
    return 0;
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
addMonitoredItemToCurrentTimeVariable(UA_Server *server, std::string label) {
    UA_NodeId currentTimeNodeId = UA_NODEID_STRING_ALLOC(1, label.c_str());
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
    if(root.isMember("nodeCount")){
        nodeCount = root["nodeCount"].asInt();
    }
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
        for (const Json::Value& input: root["inputArguments"]){
            std::vector<std::string> input_argument;
            input_argument.push_back(input["name"].asString());
            input_argument.push_back(input["dataType"].asString());
            inputArguments.push_back(input_argument);
        }
    }
    if(root.isMember("outputArguments")){
        for (const Json::Value& output: root["outputArguments"]){
            std::vector<std::string> output_argument;
            output_argument.push_back(output["name"].asString());
            output_argument.push_back(output["dataType"].asString());
            outputArguments.push_back(output_argument);
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

    Json::Value root = *setting;
    Json::Value OUSNetworkData = root["OUSNetworkData"];
    Json::Value OUSMemoryTreeData = root["OUSMemoryTreeData"];

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

    // setting port number
    if(OUSNetworkData.isMember("port")){
        res = UA_ServerConfig_setMinimal(config, port, NULL);
        if (res != UA_STATUSCODE_GOOD) {
            printf("Failed to run the server.\n");
            UA_Server_delete(server);
            return EXIT_FAILURE;
        }
    }

    // setting certFile and keyFile
    // if(OUSNetworkData.isMember("certFile") && OUSNetworkData.isMember("keyFile")){
    //     UA_ByteString certificate = UA_STRING_ALLOC(certFile.c_str());
    //     UA_ByteString privateKey = UA_STRING_ALLOC(keyFile.c_str());
    //     UA_ServerConfig_setDefaultWithSecurityPolicies(config, port, &certificate, &privateKey, NULL, 0, NULL, 0, NULL, 0);
    //     UA_ServerConfig_addAllEndpoints(config);
    // }
    if(OUSNetworkData.isMember("certFile") && OUSNetworkData.isMember("keyFile")){
        UA_ByteString certificate = UA_BYTESTRING_NULL;
        UA_ByteString privateKey = UA_BYTESTRING_NULL;
        certificate = loadFile(certFile.c_str());
        privateKey = loadFile(keyFile.c_str());
        UA_ServerConfig_setDefaultWithSecurityPolicies(config, port, &certificate, &privateKey, NULL, 0, NULL, 0, NULL, 0);
        UA_ServerConfig_addAllEndpoints(config);
    }

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
        }
    }

    for(int i=0;i<OUSNetworkData["nodeCount"].asInt();i++){

        nodeSetting(&OUSMemoryTreeData[i]);

        // @일단 Variable만 가능함
        // address space에 노드 추가
        UA_VariableAttributes attr;
        UA_NodeId myValueNodeId;
        UA_QualifiedName myValueName;

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
            UA_String myValue = UA_STRING_NULL;
            UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_STRING]);
        } else if(type=="ByteString"){
            UA_ByteString myValue = UA_BYTESTRING_NULL;
            UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_BYTESTRING]);
        } else if(type=="XmlElement"){
            UA_XmlElement myValue = UA_STRING_NULL;
            UA_Variant_setScalarCopy(&attr.value, &myValue, &UA_TYPES[UA_TYPES_XMLELEMENT]);
        }

        attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US",label.c_str()); //속성을 영문으로 설정
        attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US",label.c_str());

        // 변수 노드의 속성을 수정하여 읽기/쓰기 변경
        if(accessRight == "Read & Write"){
            attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
            attr.userAccessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        } else if(accessRight == "ReadOnly"){
            attr.accessLevel = UA_ACCESSLEVELMASK_READ;
            attr.userAccessLevel = UA_ACCESSLEVELMASK_READ;
        }

        myValueNodeId = UA_NODEID_STRING_ALLOC(1, label.c_str()); //Node ID 설정
        myValueName = UA_QUALIFIEDNAME_ALLOC(1, label.c_str()); //변수 노드 이름 설정
        printf("Namespace: %d, Identifier: %s\n", myValueNodeId.namespaceIndex, myValueNodeId.identifier.string.data);
        
        std::string parent;
        GetSecondLastNumberFromRoute(route, parent);

        UA_NodeId parentNodeId;
        UA_NodeId parentReferenceNodeId;

        if(parent == "Objects"){
            parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); 
            parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
        } else{
            parentNodeId = UA_NODEID_STRING_ALLOC(1, parent.c_str());
            parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES); //부모 노드 지정
        }
        
        UA_Server_addVariableNode(server, myValueNodeId, parentNodeId,
                                parentReferenceNodeId, myValueName,
                                UA_NODEID_NULL, attr, NULL, NULL); // 서버에 변수 노드 추가

        // 할당된 메모리 해제
        UA_VariableAttributes_clear(&attr);
        UA_NodeId_clear(&myValueNodeId);
        UA_QualifiedName_clear(&myValueName);

        addMonitoredItemToCurrentTimeVariable(server, label);

    }
    
    std::cout << "[Setting Success]\n" << std::endl;

    // 변화된 값 모니터링 후 출력
    // addMonitoredItemToCurrentTimeVariable(server);

    res = UA_Server_runUntilInterrupt(server);

    if (res != UA_STATUSCODE_GOOD) {
        printf("Failed to run the server.\n");
        UA_Server_delete(server);
        return EXIT_FAILURE;
    }

    UA_Server_delete(server);
    return res == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;

    return 0;



///////////////////////////////////////////////////
/*
    UA_Server *server = UA_Server_new(); //서버 객체 생성
    UA_ServerConfig *config = UA_Server_getConfig(server);
    UA_StatusCode res = UA_STATUSCODE_GOOD;
    UA_ServerConfig_setDefault(config);

    // setting port number
    res = UA_ServerConfig_setMinimal(config, port, NULL);
    if (res != UA_STATUSCODE_GOOD) {
        printf("Failed to run the server.\n");
        UA_Server_delete(server);
        return EXIT_FAILURE;
    }

    UA_ByteString certificate = UA_BYTESTRING_NULL;
    UA_ByteString privateKey = UA_BYTESTRING_NULL;
    certificate = loadFile(certFile.c_str());
    privateKey = loadFile(keyFile.c_str());
    UA_ServerConfig_setDefaultWithSecurityPolicies(config, port, &certificate, &privateKey, NULL, 0, NULL, 0, NULL, 0);
    UA_ServerConfig_addAllEndpoints(config);

    Json::Value root = *setting;
    if(root.isMember("users")){
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
        }
    }
*/
    
/*
    // address space에 노드 추가
    UA_VariableAttributes attr;
    UA_NodeId myValueNodeId;
    UA_QualifiedName myValueName;

    // adress space에 변수 노드 추가
    attr = UA_VariableAttributes_default; //변수 노드의 속성 초기화
    UA_Int32 myInteger = NULL;
    UA_Variant_setScalarCopy(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]); //변수 값 설정
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US",label.c_str()); //속성을 영문으로 설정
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US",label.c_str());

    // 변수 노드의 속성을 수정하여 쓰기 가능하게 만듭니다
    if(accessRight == "Read & Write"){
        attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        attr.userAccessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    }

    myValueNodeId = UA_NODEID_STRING_ALLOC(1, "testNode"); //Node ID 설정
    myValueName = UA_QUALIFIEDNAME_ALLOC(1, "testNode"); //변수 노드 이름 설정
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); 
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES); //부모 노드 지정
    UA_Server_addVariableNode(server, myValueNodeId, parentNodeId,
                            parentReferenceNodeId, myValueName,
                            UA_NODEID_NULL, attr, NULL, NULL); // 서버에 변수 노드 추가

    // 할당된 메모리 해제
    UA_VariableAttributes_clear(&attr);
    UA_NodeId_clear(&myValueNodeId);
    UA_QualifiedName_clear(&myValueName);

    // 변화된 값 모니터링 후 출력
    addMonitoredItemToCurrentTimeVariable(server);

    res = UA_Server_runUntilInterrupt(server);

    if (res != UA_STATUSCODE_GOOD) {
        printf("Failed to run the server.\n");
        UA_Server_delete(server);
        return EXIT_FAILURE;
    }

    UA_Server_delete(server);
    return res == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
*/


}



/*
void OpcuaServer::write_log(char* type, const char* content, char* topic){
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
    sprintf(log, "{\"id\" : \"%s\", \"time\" : \"%s\", \"type\" : \"%s\", \"content\" : \"%s\"}", id.c_str(), microsecond, type, content);
    
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
*/









/*

int main() {

    int check = 0;

    UA_Server *server = UA_Server_new(); //서버 객체 생성
    UA_ServerConfig *config = UA_Server_getConfig(server);
    UA_StatusCode res = UA_STATUSCODE_GOOD;
    UA_ServerConfig_setDefault(config);

    // setting port number
    res = UA_ServerConfig_setMinimal(config, 4880, NULL);
    if (res != UA_STATUSCODE_GOOD) {
        printf("Failed to run the server.\n");
        UA_Server_delete(server);
        return EXIT_FAILURE;
    }

    //수정 필요 조건문 (username, password)
    if(check){
        UA_ByteString certificate = UA_BYTESTRING_NULL;
        UA_ByteString privateKey = UA_BYTESTRING_NULL;
        UA_UInt16 portNumber = 4880;
        certificate = loadFile("server_cert.der");
        privateKey = loadFile("server_key.der");
        UA_ServerConfig_setDefaultWithSecurityPolicies(config, portNumber, &certificate, &privateKey, NULL, 0, NULL, 0, NULL, 0);
        UA_ServerConfig_addAllEndpoints(config);
    }
    else{
        size_t usernamePasswordLoginSize = 2;
        static UA_UsernamePasswordLogin userNamePW[2] = {
            {UA_STRING_STATIC("test"), UA_STRING_STATIC("test")},
            {UA_STRING_STATIC("paula"), UA_STRING_STATIC("paula123")}
        };
        UA_Boolean allowAnonymous = false;
        UA_String encryptionPolicy = config->securityPolicies[config->securityPoliciesSize-1].policyUri;
        config->accessControl.clear(&config->accessControl);
        UA_AccessControl_default(config, allowAnonymous, &encryptionPolicy, usernamePasswordLoginSize, userNamePW);
    }


    // adress space에 변수 노드 추가
    UA_VariableAttributes attr = UA_VariableAttributes_default; //변수 노드의 속성 초기화
    UA_Int32 myInteger = 1234;
    //UA_Int32 myInteger = NULL;
    UA_Variant_setScalarCopy(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]); //변수 값 설정
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US","testNode"); //속성을 영문으로 설정
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US","testNode");

    // 변수 노드의 속성을 수정하여 쓰기 가능하게 만듭니다
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    attr.userAccessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId myIntegerNodeId = UA_NODEID_STRING_ALLOC(1, "testNode"); //Node ID 설정
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME_ALLOC(1, "testNode"); //변수 노드 이름 설정
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); 
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES); //부모 노드 지정
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
                              parentReferenceNodeId, myIntegerName,
                              UA_NODEID_NULL, attr, NULL, NULL); // 서버에 변수 노드 추가

    // //할당된 메모리 해제
    UA_VariableAttributes_clear(&attr);
    UA_NodeId_clear(&myIntegerNodeId);
    UA_QualifiedName_clear(&myIntegerName);


    addMonitoredItemToCurrentTimeVariable(server);

    res = UA_Server_runUntilInterrupt(server);

    if (res != UA_STATUSCODE_GOOD) {
        printf("Failed to run the server.\n");
        UA_Server_delete(server);
        return EXIT_FAILURE;
    }

    UA_Server_delete(server);
    return res == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}

*/