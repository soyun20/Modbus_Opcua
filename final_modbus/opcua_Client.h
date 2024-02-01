#include <open62541/client_highlevel.h>
#include <open62541/plugin/pki_default.h>
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel_async.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server_config_default.h>

#include <jsoncpp/json/json.h>
#include <stdlib.h>
#include <iostream>

#include "common.h"

class OPC_Client {
    public:
    OPC_Client();
    ~OPC_Client();
    
    int start(Json::Value* root);
    
    std::string id;
    UA_String securityPolicyUri = UA_String_fromChars("http://opcfoundation.org/UA/SecurityPolicy#None");;
    UA_MessageSecurityMode securityMode = UA_MESSAGESECURITYMODE_INVALID;
    int port = 4880;
    const char* serverUrl = "opc.tcp://127.0.0.1:4880";
    const char* securityPolicy = NULL;
    const char* securityMod = NULL;
    const char* username = NULL;
    const char* password = NULL;
    
};
  