#include "opcuaServer.h"


/*
int OpcuaServer::Setting(Json::Value* setting)
{
    printf("Setting start\n");
    Json::Value root = *setting;

    if(root.isMember("ipAddress")){
        ipAddress = root["ipAddress"].asString();
    }
    if(root.isMember("port")){
        port = root["port"].asString();
    }
    if(root.isMember("certFile")){
        certFile = root["certFile"].asString();
    }
    if(root.isMember("keyFile")){
        keyFile = root["keyFile"].asString();
    }
    if(root.isMember("userManagement")){
        userManagement = root["userManagement"].asString();
    }
    if(root.isMember("role")){
        role = root["role"].asString();
    }
    if(root.isMember("role")){
        role = root["role"].asString();
    }
    if(root.isMember("role")){
        role = root["role"].asString();
    }
    if(root.isMember("role")){
        role = root["role"].asString();
    }
    if(root.isMember("role")){
        role = root["role"].asString();
    }
    if(root.isMember("role")){
        role = root["role"].asString();
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
    std::cout << "\n[Setting Success]\n" << std::endl;
    return 0;
}
*/

int main() {

    int check = 1;

    UA_Server *server = UA_Server_new(); //서버 객체 생성
    UA_ServerConfig *config = UA_Server_getConfig(server);
    UA_StatusCode res = UA_STATUSCODE_GOOD;
    UA_ServerConfig_setDefault(config);

    //ip address 추가?

/*
    if (config->serverUrlsSize > 1) {
        UA_Array_delete(config->serverUrls, config->serverUrlsSize, &UA_TYPES[UA_TYPES_STRING]);
    } 
    config->serverUrls = (UA_String *)UA_Array_new(1, &UA_TYPES[UA_TYPES_STRING]);
    config->serverUrls[0] = UA_STRING("");
    config->serverUrls[0] = UA_STRING("opc.tcp://0.0.0.0:4880");
    config->serverUrlsSize = 1; // 새로운 URL의 수를 설정

    UA_ServerConfig_setDefault(config);

    if (config->serverUrlsSize > 0) {
        UA_Array_delete(config->serverUrls, config->serverUrlsSize, &UA_TYPES[UA_TYPES_STRING]);
    }

    // serverUrls 배열 할당 및 주소 설정
    config->serverUrlsSize = 1;
    config->serverUrls = (UA_String *)UA_Array_new(config->serverUrlsSize, &UA_TYPES[UA_TYPES_STRING]);
    config->serverUrls[0] = UA_STRING("opc.tcp://0.0.0.0:4880");
    config->serverUrls[1] = UA_STRING("");
*/

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
            {UA_STRING_STATIC("peter"), UA_STRING_STATIC("peter123")},
            {UA_STRING_STATIC("paula"), UA_STRING_STATIC("paula123")}
        };
        UA_Boolean allowAnonymous = false;
        UA_String encryptionPolicy = config->securityPolicies[config->securityPoliciesSize-1].policyUri;
        config->accessControl.clear(&config->accessControl);
        UA_AccessControl_default(config, allowAnonymous, &encryptionPolicy, usernamePasswordLoginSize, userNamePW);
    }

/*
    // Method with IO Arguments
    UA_Argument inputArguments;
    UA_Argument outputArguments;

    UA_Argument_init(&inputArguments);
    inputArguments.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    inputArguments.description = UA_LOCALIZEDTEXT("en-US", "Say your name");
    inputArguments.name = UA_STRING("Name");
    inputArguments.valueRank = UA_VALUERANK_SCALAR; // scalar argument

    
    UA_Argument_init(&outputArguments);
    outputArguments.arrayDimensionsSize = 0;
    outputArguments.arrayDimensions = NULL;
    outputArguments.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    outputArguments.description = UA_LOCALIZEDTEXT("en-US", "Receive a greeting");
    outputArguments.name = UA_STRING("greeting");
    outputArguments.valueRank = UA_VALUERANK_SCALAR;

    UA_MethodAttributes addmethodattributes = UA_MethodAttributes_default;
    addmethodattributes.displayName = UA_LOCALIZEDTEXT("en-US", "Hello World");
    addmethodattributes.executable = true;
    addmethodattributes.userExecutable = true;
    UA_Server_addMethodNode(server, UA_NODEID_NUMERIC(1, 62541),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(1, "hello_world"), addmethodattributes,
                            &helloWorld, // callback of the method node
                            1, &inputArguments, 1, &outputArguments, NULL, NULL);
*/


    // adress space에 변수 노드 추가
    UA_VariableAttributes attr = UA_VariableAttributes_default; //변수 노드의 속성 초기화
    UA_Int32 myInteger = 1234;
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

    res = UA_Server_runUntilInterrupt(server);

    if (res != UA_STATUSCODE_GOOD) {
        printf("Failed to run the server.\n");
        UA_Server_delete(server);
        return EXIT_FAILURE;
    }

    UA_Server_delete(server);
    return res == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}