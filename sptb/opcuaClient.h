#include <open62541/client_highlevel.h>
#include <open62541/plugin/pki_default.h>
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel_async.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server_config_default.h>
#include <open62541/src/ua_securechannel.h>
#include <open62541/src/client/ua_client_internal.h>
#include <mutex>
#include "MQTT.h"
#include <jsoncpp/json/json.h>
#include <stdlib.h>
#include <iostream>
#include <open62541/client_subscriptions.h>
#include <mariadb/mysql.h>
#include <thread>
#include <sys/time.h>


// Json::Value origin_root;

class OPC_Client {
    public:
    
    int start(Json::Value* root);
    static void readValueAttributeCallback(UA_Client *client, void *userdata, UA_UInt32 requestId, UA_StatusCode status, UA_DataValue *var);
    static void writeValueAttributeCallback(UA_Client *client, void *userdata, UA_UInt32 requestId, UA_WriteResponse *response);
    static void methodCalled(UA_Client *client, void *userdata, UA_UInt32 requestId, UA_CallResponse *response);
    static void subscriptionInactivityCallback (UA_Client *client, UA_UInt32 subId, void *subContext);
    static void DataChangeNotificationCallback(UA_Client *client, UA_UInt32 subId, void *subContext, UA_UInt32 monId, void *monContext, UA_DataValue *value);
    void setVariantValue(UA_Variant* variant, const std::string& value, const std::string& valuetype);
    
    void alloc_json(Json::Value* root);
    static void printUAVariant(const UA_Variant *variant);
    static void printTimestamp(char *name, UA_DateTime date);
    static UA_Boolean readHist(UA_Client *client, const UA_NodeId *nodeId, UA_Boolean moreDataAvailable, const UA_ExtensionObject *data, void *unused);
    
    static void fileBrowsed(UA_Client *client, void *userdata, UA_UInt32 requestId, UA_BrowseResponse *response);
    
    static void onConnect (UA_Client *client, UA_SecureChannelState channelState, UA_SessionState sessionState, UA_StatusCode connectStatus);
    void Client_Process(Json::Value* root);
    void OPC_Client_Uninitialize();
    static void* run_iterate_thread(void* clientPtr);
    
    Json::Value recv_MQTT(void);
    // void printUAVariant(const UA_Variant *variant);

    ClientMQTT Recv_OPC;

    std::string clientID;
    int mqttid;
    UA_Client *client;

    UA_String securityPolicyUri = UA_String_fromChars("http://opcfoundation.org/UA/SecurityPolicy#None");
    UA_MessageSecurityMode securityMode = UA_MESSAGESECURITYMODE_INVALID;
    int port = 4880;
    std::string serverUrl;
    std::string securityPolicy;
    std::string username;
    std::string pwd;
    std::string certfile;
    std::string keyfile = "client_cert/own/client_key.der";
    std::string trustList;
    std::string certAuthFile;
    std::string keyAuthFile;
    std::string nodeid;
    std::string valuetype;
    std::string value;
    std::string type;
    std::string raw;
    std::string useridentify;
    char s[260];
    int ns;
    int msgCount;
    std::string applicationUri;
    
    UA_ByteString certificate;
    UA_ByteString privateKey;

    
    
};
  