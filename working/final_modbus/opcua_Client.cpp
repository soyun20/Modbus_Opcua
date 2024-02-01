#include "opcua_Client.h"

#define UA_ENABLE_ENCRYPTION

static void usage(void) {
    printf("Usage: client [-username name] [-password password] ");
#ifdef UA_ENABLE_ENCRYPTION
    printf("[-cert certfile.der] [-key keyfile.der] [-securityMode <0-3>] [-securityPolicy policyUri] ");
#endif
    printf("opc.tcp://<host>:<port>\n");
}

#define NODES_EXIST

static void
onConnect(UA_Client *client, UA_SecureChannelState channelState, UA_SessionState sessionState, UA_StatusCode connectStatus) {
    printf("Async connect returned with status code %s\n",  UA_StatusCode_name(connectStatus));
}
std::string printUAVariant(const UA_Variant *variant) {
    printf("UA_Variant Information:\n");

    // Data Type Description
    printf("Data Type: %s\n", variant->type->typeName);

    // Storage Type
    printf("Storage Type: %d\n", variant->storageType);

    // Array Length
    printf("Array Length: %zu\n", variant->arrayLength);

    // Data (Scalar or Array)
    if (variant->arrayLength > 0) {
        printf("Data (Array):\n");
        for (size_t i = 0; i < variant->arrayLength; i++) {
            if (i == 0) {
                printf("  [");
            }
            printf("%d", ((int*)variant->data)[i]);
            if (i < variant->arrayLength - 1) {
                printf(", ");
            } else {
                printf("]\n");
            }
        }
    } else {
        printf("Data (Scalar): %d\n", *((int*)variant->data));
    }

    // Array Dimensions
    printf("Number of Dimensions: %zu\n", variant->arrayDimensionsSize);
    if (variant->arrayDimensionsSize > 0) {
        printf("Array Dimensions: ");
        for (size_t i = 0; i < variant->arrayDimensionsSize; i++) {
            printf("%u", variant->arrayDimensions[i]);
            if (i < variant->arrayDimensionsSize - 1) {
                printf(" x ");
            }
        }
        printf("\n");
    }
}
static
void
fileBrowsed(UA_Client *client, void *userdata, UA_UInt32 requestId,
            UA_BrowseResponse *response) {
    printf("%-50s%u\n", "Received BrowseResponse for request ", requestId);
    UA_String us = *(UA_String *) userdata;
    printf("---%.*s passed safely \n", (int) us.length, us.data);
}

/*high-level function callbacks*/
static
void
readValueAttributeCallback(UA_Client *client, void *userdata, UA_UInt32 requestId, UA_StatusCode status, UA_DataValue *var) {
                            
    UA_Variant res = var->value;
    printUAVariant(&res);
    
    printf("%-50s%u\n", "Read value attribute for request", requestId);
    if(UA_Variant_hasScalarType(&var->value, &UA_TYPES[UA_TYPES_INT32])) {
        UA_Int32 int_val = *(UA_Int32*) var->value.data;
        printf("---%-40s%-8i\n",
               "Reading the value of node (1, \"the.answer\"):", int_val);
    }
}

static
void
writeValueAttributeCallback(UA_Client *client, void *userdata, UA_UInt32 requestId,
            UA_WriteResponse *response) {
    /*assuming no data to be retrieved by writing attributes*/
    if (response->responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        printf("%-50s%u\n", "Wrote value attribute for request ", requestId);
    } else {
        printf("Write failed\n");
    }
    UA_WriteResponse_clear(response);
}

#ifdef NODES_EXIST
#ifdef UA_ENABLE_METHODCALLS
static void
methodCalled(UA_Client *client, void *userdata, UA_UInt32 requestId,
             UA_CallResponse *response) {

    printf("%-50s%u\n", "Called method for request ", requestId);
    size_t outputSize;
    UA_Variant *output;
    UA_StatusCode retval = response->responseHeader.serviceResult;
    if(retval == UA_STATUSCODE_GOOD) {
        if(response->resultsSize == 1)
            retval = response->results[0].statusCode;
        else
            retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    if(retval != UA_STATUSCODE_GOOD) {
        UA_CallResponse_clear(response);
        printf("---Method call was unsuccessful, returned %x values.\n", retval);
    } else {
        /* Move the output arguments */
        output = response->results[0].outputArguments;
        outputSize = response->results[0].outputArgumentsSize;
        response->results[0].outputArguments = NULL;
        response->results[0].outputArgumentsSize = 0;
        printf("---Method call was successful, returned %lu values.\n",
               (unsigned long)outputSize);
        UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
    }

    UA_CallResponse_clear(response);
}

#endif
#endif

int main(int argc, char *argv[]) {
    UA_String securityPolicyUri = UA_STRING_NULL;
    UA_MessageSecurityMode securityMode = UA_MESSAGESECURITYMODE_INVALID; /* allow everything */
    char *serverurl = NULL;
    char *username = NULL;
    char *password = NULL;
#ifdef UA_ENABLE_ENCRYPTION
    char *certfile = NULL;
    char *keyfile = NULL;
    char *trustList = NULL;
#endif
#if defined(UA_ENABLE_ENCRYPTION_OPENSSL) || defined(UA_ENABLE_ENCRYPTION_MBEDTLS)
    char *certAuthFile = NULL;
    char *keyAuthFile = NULL;
#endif

    /* At least one argument is required for the server uri */
    if(argc <= 1) {
        usage();
        return 0;
    }

    /* Parse the arguments */
    for(int argpos = 1; argpos < argc; argpos++) {
        if(strcmp(argv[argpos], "--help") == 0 ||
           strcmp(argv[argpos], "-h") == 0) {
            usage();
            return 0;
        }

        if(argpos + 1 == argc) {
            serverurl = argv[argpos];
            continue;
        }

        if(strcmp(argv[argpos], "-username") == 0) {
            argpos++;
            username = argv[argpos];
            continue;
        }

        if(strcmp(argv[argpos], "-password") == 0) {
            argpos++;
            password = argv[argpos];
            continue;
        }

#ifdef UA_ENABLE_ENCRYPTION
        if(strcmp(argv[argpos], "-cert") == 0) {
            argpos++;
            certfile = argv[argpos];
            continue;
        }

        if(strcmp(argv[argpos], "-key") == 0) {
            argpos++;
            keyfile = argv[argpos];
            continue;
        }

        if(strcmp(argv[argpos], "-trustList") == 0) {
            argpos++;
            trustList = argv[argpos];
            continue;
        }

        if(strcmp(argv[argpos], "-securityMode") == 0) {
            argpos++;
            if(sscanf(argv[argpos], "%i", (int*)&securityMode) != 1) {
                usage();
                return 0;
            }
            continue;
        }

        if(strcmp(argv[argpos], "-securityPolicy") == 0) {
            argpos++;
            securityPolicyUri = UA_String_fromChars(argv[argpos]);
            continue;
        }
#endif
#if defined(UA_ENABLE_ENCRYPTION_OPENSSL) || defined(UA_ENABLE_ENCRYPTION_MBEDTLS)
        if(strcmp(argv[argpos], "-certAuth") == 0) {
            argpos++;
            certAuthFile = argv[argpos];
            continue;
        }

        if(strcmp(argv[argpos], "-keyAuth") == 0) {
            argpos++;
            keyAuthFile = argv[argpos];
            continue;
        }
#endif
        usage();
        return 0;
    }

    /* Create the server and set its config */
    UA_Client *client = UA_Client_new();
    UA_ClientConfig *cc = UA_Client_getConfig(client);

    /* Set securityMode and securityPolicyUri */
    cc->securityMode = securityMode;
    cc->securityPolicyUri = securityPolicyUri;
    cc->stateCallback = onConnect;

#ifdef UA_ENABLE_ENCRYPTION
    if(certfile) {
        UA_ByteString certificate = loadFile(certfile);
        UA_ByteString privateKey  = loadFile(keyfile);
        if(trustList) {
            /* Load the trust list */
            size_t trustListSize = 1;
            UA_STACKARRAY(UA_ByteString, trustListAuth, trustListSize);
            trustListAuth[0] = loadFile(trustList);
            UA_ClientConfig_setDefaultEncryption(cc, certificate, privateKey,
                                                 trustListAuth, trustListSize, NULL, 0);
            UA_ByteString_clear(&trustListAuth[0]);
        }else {
            /* If no trust list is passed, all certificates are accepted. */
            UA_ClientConfig_setDefaultEncryption(cc, certificate, privateKey,
                                                 NULL, 0, NULL, 0);
            UA_CertificateVerification_AcceptAll(&cc->certificateVerification);
        }
        UA_ByteString_clear(&certificate);
        UA_ByteString_clear(&privateKey);
    } else {
        UA_ClientConfig_setDefault(cc);
        cc->securityMode = UA_MESSAGESECURITYMODE_NONE;
        UA_ByteString_clear(&cc->securityPolicyUri);
        cc->securityPolicyUri = UA_String_fromChars("http://opcfoundation.org/UA/SecurityPolicy#None");
    }
#else
    UA_ClientConfig_setDefault(cc);
#endif

    UA_ApplicationDescription_clear(&cc->clientDescription);
    cc->clientDescription.applicationUri = UA_STRING_ALLOC("urn:open62541.server.application");
    cc->clientDescription.applicationType = UA_APPLICATIONTYPE_CLIENT;

    /* Connect to the server */
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    if(username) {
        UA_ClientConfig_setAuthenticationUsername(cc, username, password);
        retval = UA_Client_connect(client, serverurl);
        /* Alternative */
        //retval = UA_Client_connectUsername(client, serverurl, username, password);
    }
#if defined(UA_ENABLE_ENCRYPTION_OPENSSL) || defined(UA_ENABLE_ENCRYPTION_MBEDTLS)
    else if(certAuthFile && certfile) {
        UA_ByteString certificateAuth = loadFile(certAuthFile);
        UA_ByteString privateKeyAuth  = loadFile(keyAuthFile);
        UA_ClientConfig_setAuthenticationCert(cc, certificateAuth, privateKeyAuth);
        
        retval = UA_Client_connect(client, serverurl);

        UA_ByteString_clear(&certificateAuth);
        UA_ByteString_clear(&privateKeyAuth);
    }
#endif
    else
        retval = UA_Client_connect(client, serverurl);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Could not connect");
        UA_Client_delete(client);
        return 0;
    }

    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Connected!");

    /* Read the server-time */
    UA_Variant value;
    UA_Variant_init(&value);
    UA_Client_readValueAttribute(client,
              UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME),
              &value);
    if(UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_DATETIME])) {
        UA_DateTimeStruct dts = UA_DateTime_toStruct(*(UA_DateTime *)value.data);
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                    "The server date is: %02u-%02u-%04u %02u:%02u:%02u.%03u",
                    dts.day, dts.month, dts.year, dts.hour, dts.min, dts.sec, dts.milliSec);
    }
    




    UA_UInt32 reqId = 0;
    UA_String userdata = UA_STRING("userdata");

    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */

    

    /*Windows needs time to response*/
    sleep_ms(100);


    /* Demo: high-level functions */
    UA_Int16 value1 = 0;
    UA_Variant myVariant;
    UA_Variant_init(&myVariant);

    UA_Variant input;
    UA_Variant_init(&input);

    for(UA_UInt16 i = 0; i < 5; i++) {
        UA_SessionState ss;
        UA_Client_getState(client, NULL, &ss, NULL);
        if(ss == UA_SESSIONSTATE_ACTIVATED) {
            /* writing and reading value 1 to 5 */
            UA_Variant_setScalarCopy(&myVariant, &value1 , &UA_TYPES[UA_TYPES_INT16]);
            value1++;
            UA_Client_writeValueAttribute_async(client,
                                                UA_NODEID_STRING(1, "node"), &myVariant,
                                                writeValueAttributeCallback, NULL,
                                                &reqId);
            UA_Variant_clear(&myVariant);

            UA_Client_readValueAttribute_async(client,
                                               UA_NODEID_STRING(1, "node"),
                                               readValueAttributeCallback, NULL,
                                               &reqId);

//TODO: check the existance of the nodes inside these functions (otherwise seg faults)
#ifdef NODES_EXIST
#ifdef UA_ENABLE_METHODCALLS
            UA_String stringValue = UA_String_fromChars("World");
            UA_Variant_setScalar(&input, &stringValue, &UA_TYPES[UA_TYPES_STRING]);

            UA_Client_call_async(client,
                                 UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                 UA_NODEID_STRING(1, "method"), 1, &input,
                                 methodCalled, NULL, &reqId);
            UA_String_clear(&stringValue);
#endif /* UA_ENABLE_METHODCALLS */
#endif
            /* How often UA_Client_run_iterate is called depends on the number of request sent */
            UA_Client_run_iterate(client, 0);
            UA_Client_run_iterate(client, 0);
        }
    }
    UA_Client_run_iterate(client, 0);

    /* Async disconnect kills unprocessed requests */
    // UA_Client_disconnect_async (client, &reqId); //can only be used when connected = true
    // UA_Client_run_iterate (client, &timedOut);




    UA_Variant_clear(&value);

    /* Clean up */
    UA_Client_disconnect(client);
    UA_Client_delete(client);
    return 0;
}


int OPC_Client::start(Json::Value* root)
{
    if ((*root).isMember("networkData"))
    {
        if ((*root).isMember("id")) 
            id = (*root)["id"].asString();
        if ((*root)["networkData"].isMember("port")) 
            serverUrl = (*root)["networkData"]["serverUrl"].asString().c_str();
        if ((*root)["networkData"].isMember("port")) 
            username = (*root)["networkData"]["username"].asString().c_str();
        if ((*root)["networkData"].isMember("port")) 
            password = (*root)["networkData"]["password"].asString().c_str();
        if ((*root)["networkData"].isMember("port")) 
        {
            securityPolicy = (*root)["networkData"]["securityPolicy"].asString().c_str();
            UA_String securityPolicyUri = UA_String_fromChars("http://opcfoundation.org/UA/SecurityPolicy#None");
        }
        if ((*root)["networkData"].isMember("port")) 
            securityMode = (*root)["networkData"]["securityMode"].asString().c_str();
        UA_MessageSecurityMode securityMode = UA_MESSAGESECURITYMODE_INVALID; /* allow everything */
            
    }
    if ((*root).isMember("msgData"))
    {
        
    }

    
    
    char *serverurl = NULL;
    char *username = NULL;
    char *password = NULL;
#ifdef UA_ENABLE_ENCRYPTION
    char *certfile = NULL;
    char *keyfile = NULL;
    char *trustList = NULL;
#endif
#if defined(UA_ENABLE_ENCRYPTION_OPENSSL) || defined(UA_ENABLE_ENCRYPTION_MBEDTLS)
    char *certAuthFile = NULL;
    char *keyAuthFile = NULL;
#endif
}