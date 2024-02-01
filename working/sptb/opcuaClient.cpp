#include "opcuaClient.h"
#include <open62541/src/ua_securechannel.h>

#define UA_ENABLE_ENCRYPTION
#define NODES_EXIST
#define UA_ENABLE_EXPERIMENTAL_HISTORIZING 

std::string id;
ClientMQTT Send_OPC_Log;

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
    
    Send_OPC_Log.Publish(topic, log);


    std::string packet_send;
    std::string packet_recv;

    uint8_t* recvBuf = NULL;
    int recvBufLen;
    uint8_t* sendBuf = NULL;
    int sendBufLen;

    recvBuf = GetRecvMsg();
    recvBufLen = GetRecvMsgLen();

    sendBuf = ClientGetSendMsg();
    sendBufLen = ClientGetSendMsgLen();

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
        Send_OPC_Log.Publish(topic, packetLog);
    }

    if(sendBufLen != 0){
        for (size_t i = 0; i < sendBufLen; ++i) {
            char hex[4];
            snprintf(hex, sizeof(hex), "%02X ", sendBuf[i]);
            packet_send += hex;
        }

        sprintf(packetLog, "{\"id\" : \"%s\", \"time\" : \"%s\", \"type\" : \"%s\", \"content\" : \"%s\"}", id.c_str(), microsecond, type, packet_send.c_str());
        std::cout << "topic:" << topic << " packetLog: " <<packetLog <<std::endl;
        Send_OPC_Log.Publish(topic, packetLog);
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



void write_log_callback(void *logContext, UA_LogLevel level, UA_LogCategory category, const char *msg, va_list args) {
    char logMessage[2048]; 
    vsnprintf(logMessage, sizeof(logMessage), msg, args);
    write_log("OPCUA_LOG", "/log/trans", logMessage);
}

void OPC_Client::DataChangeNotificationCallback(UA_Client *client, UA_UInt32 subId, void *subContext, UA_UInt32 monId, void *monContext, UA_DataValue *value) {
    if (value->hasValue) {
        if (value->value.type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
            UA_Boolean data = *(UA_Boolean *)value->value.data;
            printf("Boolean Value changed: %s\n", data ? "true" : "false");
        } else if (value->value.type == &UA_TYPES[UA_TYPES_SBYTE]) {
            UA_SByte data = *(UA_SByte *)value->value.data;
            printf("SByte Value changed: %d\n", data);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_BYTE]) {
            UA_Byte data = *(UA_Byte *)value->value.data;
            printf("Byte Value changed: %u\n", data);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_INT16]) {
            UA_Int16 data = *(UA_Int16 *)value->value.data;
            printf("Int16 Value changed: %d\n", data);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_UINT16]) {
            UA_UInt16 data = *(UA_UInt16 *)value->value.data;
            printf("UInt16 Value changed: %u\n", data);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_INT32]) {
            UA_Int32 data = *(UA_Int32 *)value->value.data;
            printf("Int32 Value changed: %d\n", data);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_UINT32]) {
            UA_UInt32 data = *(UA_UInt32 *)value->value.data;
            printf("UInt32 Value changed: %u\n", data);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_INT64]) {
            UA_Int64 data = *(UA_Int64 *)value->value.data;
            printf("Int64 Value changed: %lld\n", (long long)data);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_UINT64]) {
            UA_UInt64 data = *(UA_UInt64 *)value->value.data;
            printf("UInt64 Value changed: %llu\n", (unsigned long long)data);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_FLOAT]) {
            UA_Float data = *(UA_Float *)value->value.data;
            printf("Float Value changed: %f\n", data);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            UA_Double data = *(UA_Double *)value->value.data;
            printf("Double Value changed: %lf\n", data);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_STRING]) {
            UA_String data = *(UA_String *)value->value.data;
            printf("String Value changed: %s\n", data.data);
        } else {
            printf("Unsupported data type\n");
        }
    } else {
        printf("No data\n");
    }
}


void* OPC_Client::run_iterate_thread(void* clientPtr)
{
    UA_Client* client = (UA_Client*)clientPtr;

    while (true) {
        sleep(1);
        UA_Client_run_iterate(client, 1000);
    }

    return NULL;
}


void printUAVariant(const UA_Variant *variant) {
    printf("UA_Variant Information:\n");

    if (!variant) {
        printf("Variant is null\n");
        return;
    }

    if (!variant->type || !variant->type->typeName) {
        printf("Type or typeName is null\n");
        return;
    }

    printf("Data Type: %s\n", variant->type->typeName);

    printf("Storage Type: %d\n", variant->storageType);

    printf("Array Length: %zu\n", variant->arrayLength);


    if (variant->arrayLength > 0) {
        printf("Data (Array):\n");
        if (variant->type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
            UA_Boolean *data = (UA_Boolean *)variant->data;
            printf("[");
            for (size_t i = 0; i < variant->arrayLength; i++) {
                printf("%s", data[i] ? "true" : "false");
                if (i < variant->arrayLength - 1) {
                    printf(", ");
                }
            }
            printf("]\n");
        } else if (variant->type == &UA_TYPES[UA_TYPES_SBYTE]) {
            UA_SByte *data = (UA_SByte *)variant->data;
            printf("[");
            for (size_t i = 0; i < variant->arrayLength; i++) {
                printf("%d", data[i]);
                if (i < variant->arrayLength - 1) {
                    printf(", ");
                }
            }
            printf("]\n");
        } else if (variant->type == &UA_TYPES[UA_TYPES_BYTE]) {
            UA_Byte *data = (UA_Byte *)variant->data;
            printf("[");
            for (size_t i = 0; i < variant->arrayLength; i++) {
                printf("%u", data[i]);
                if (i < variant->arrayLength - 1) {
                    printf(", ");
                }
            }
            printf("]\n");
        } else if (variant->type == &UA_TYPES[UA_TYPES_INT16]) {
            UA_Int16 *data = (UA_Int16 *)variant->data;
            printf("[");
            for (size_t i = 0; i < variant->arrayLength; i++) {
                printf("%d", data[i]);
                if (i < variant->arrayLength - 1) {
                    printf(", ");
                }
            }
            printf("]\n");
        } else if (variant->type == &UA_TYPES[UA_TYPES_UINT16]) {
            UA_UInt16 *data = (UA_UInt16 *)variant->data;
            printf("[");
            for (size_t i = 0; i < variant->arrayLength; i++) {
                printf("%u", data[i]);
                if (i < variant->arrayLength - 1) {
                    printf(", ");
                }
            }
            printf("]\n");
        } else if (variant->type == &UA_TYPES[UA_TYPES_INT32]) {
            UA_Int32 *data = (UA_Int32 *)variant->data;
            printf("[");
            for (size_t i = 0; i < variant->arrayLength; i++) {
                printf("%d", data[i]);
                if (i < variant->arrayLength - 1) {
                    printf(", ");
                }
            }
            printf("]\n");
        } else if (variant->type == &UA_TYPES[UA_TYPES_UINT32]) {
            UA_UInt32 *data = (UA_UInt32 *)variant->data;
            printf("[");
            for (size_t i = 0; i < variant->arrayLength; i++) {
                printf("%u", data[i]);
                if (i < variant->arrayLength - 1) {
                    printf(", ");
                }
            }
            printf("]\n");
        } else if (variant->type == &UA_TYPES[UA_TYPES_INT64]) {
            UA_Int64 *data = (UA_Int64 *)variant->data;
            printf("[");
            for (size_t i = 0; i < variant->arrayLength; i++) {
                printf("%lld", (long long)data[i]);
                if (i < variant->arrayLength - 1) {
                    printf(", ");
                }
            }
            printf("]\n");
        } else if (variant->type == &UA_TYPES[UA_TYPES_UINT64]) {
            UA_UInt64 *data = (UA_UInt64 *)variant->data;
            printf("[");
            for (size_t i = 0; i < variant->arrayLength; i++) {
                printf("%llu", (unsigned long long)data[i]);
                if (i < variant->arrayLength - 1) {
                    printf(", ");
                }
            }
            printf("]\n");
        } else if (variant->type == &UA_TYPES[UA_TYPES_FLOAT]) {
            UA_Float *data = (UA_Float *)variant->data;
            printf("[");
            for (size_t i = 0; i < variant->arrayLength; i++) {
                printf("%f", data[i]);
                if (i < variant->arrayLength - 1) {
                    printf(", ");
                }
            }
            printf("]\n");
        } else if (variant->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            UA_Double *data = (UA_Double *)variant->data;
            printf("[");
            for (size_t i = 0; i < variant->arrayLength; i++) {
                printf("%lf", data[i]);
                if (i < variant->arrayLength - 1) {
                    printf(", ");
                }
            }
            printf("]\n");
        } else if (variant->type == &UA_TYPES[UA_TYPES_STRING]) {
            UA_String *data = (UA_String *)variant->data;
            printf("[");
            for (size_t i = 0; i < variant->arrayLength; i++) {
                printf("\"%.*s\"", (int)data[i].length, data[i].data);
                if (i < variant->arrayLength - 1) {
                    printf(", ");
                }
            }
            printf("]\n");
        } else {
            printf("Unsupported data type\n");
        }
    } else {
        if (variant->type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
            UA_Boolean data = *(UA_Boolean *)variant->data;
            printf("Data (Scalar): %s\n", data ? "true" : "false");
        } else if (variant->type == &UA_TYPES[UA_TYPES_SBYTE]) {
            UA_SByte data = *(UA_SByte *)variant->data;
            printf("Data (Scalar): %d\n", data);
        } else if (variant->type == &UA_TYPES[UA_TYPES_BYTE]) {
            UA_Byte data = *(UA_Byte *)variant->data;
            printf("Data (Scalar): %u\n", data);
        } else if (variant->type == &UA_TYPES[UA_TYPES_INT16]) {
            UA_Int16 data = *(UA_Int16 *)variant->data;
            printf("Data (Scalar): %d\n", data);
        } else if (variant->type == &UA_TYPES[UA_TYPES_UINT16]) {
            UA_UInt16 data = *(UA_UInt16 *)variant->data;
            printf("Data (Scalar): %u\n", data);
        } else if (variant->type == &UA_TYPES[UA_TYPES_INT32]) {
            UA_Int32 data = *(UA_Int32 *)variant->data;
            printf("Data (Scalar): %d\n", data);
        } else if (variant->type == &UA_TYPES[UA_TYPES_UINT32]) {
            UA_UInt32 data = *(UA_UInt32 *)variant->data;
            printf("Data (Scalar): %u\n", data);
        } else if (variant->type == &UA_TYPES[UA_TYPES_INT64]) {
            UA_Int64 data = *(UA_Int64 *)variant->data;
            printf("Data (Scalar): %lld\n", (long long)data);
        } else if (variant->type == &UA_TYPES[UA_TYPES_UINT64]) {
            UA_UInt64 data = *(UA_UInt64 *)variant->data;
            printf("Data (Scalar): %llu\n", (unsigned long long)data);
        } else if (variant->type == &UA_TYPES[UA_TYPES_FLOAT]) {
            UA_Float data = *(UA_Float *)variant->data;
            printf("Data (Scalar): %f\n", data);
        } else if (variant->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            UA_Double data = *(UA_Double *)variant->data;
            printf("Data (Scalar): %lf\n", data);
        } else if (variant->type == &UA_TYPES[UA_TYPES_STRING]) {
            UA_String data = *(UA_String *)variant->data;
            printf("Data (Scalar): \"%.*s\"\n", (int)data.length, data.data);
        } else {
            printf("Unsupported data type\n");
        }
    }
}

void OPC_Client::printTimestamp(char *name, UA_DateTime date) {
    UA_DateTimeStruct dts = UA_DateTime_toStruct(date);
    if (name)
        printf("%s: %02u-%02u-%04u %02u:%02u:%02u.%03u, ", name,
               dts.day, dts.month, dts.year, dts.hour, dts.min, dts.sec, dts.milliSec);
    else
        printf("%02u-%02u-%04u %02u:%02u:%02u.%03u, ",
               dts.day, dts.month, dts.year, dts.hour, dts.min, dts.sec, dts.milliSec);
}

UA_Boolean OPC_Client::readHist(UA_Client *client, const UA_NodeId *nodeId, UA_Boolean moreDataAvailable, const UA_ExtensionObject *data, void *unused) {
    printf("\nRead historical callback:\n");
    printf("\tHas more data:\t%d\n\n", moreDataAvailable);

    if (data->content.decoded.type != &UA_TYPES[UA_TYPES_HISTORYDATA]) {
        return true;
    }

    UA_HistoryData *historyData = (UA_HistoryData*)data->content.decoded.data;
    printf("readRaw Value count: %lu\n", (long unsigned)historyData->dataValuesSize);

    for (UA_UInt32 i = 0; i < historyData->dataValuesSize; ++i) {
        UA_DataValue *value = &historyData->dataValues[i];
        if (value->hasServerTimestamp) {
            printTimestamp("ServerTime", value->serverTimestamp);
        }
        if (value->hasSourceTimestamp) {
            printTimestamp("SourceTime", value->sourceTimestamp);
        }
        if (value->hasStatus) {
            printf("Status 0x%08x, ", value->status);
        }

        if (value->value.type == &UA_TYPES[UA_TYPES_INT16]) {
            UA_Int16 hrValue = *(UA_Int16 *)value->value.data;
            printf("Int16Value: %d\n", hrValue);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_INT32]) {
            UA_Int32 hrValue = *(UA_Int32 *)value->value.data;
            printf("Int32Value: %d\n", hrValue);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            UA_Double hrValue = *(UA_Double *)value->value.data;
            printf("DoubleValue: %lf\n", hrValue);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_FLOAT]) {
            UA_Float hrValue = *(UA_Float *)value->value.data;
            printf("FloatValue: %f\n", hrValue);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_INT64]) {
            UA_Int64 hrValue = *(UA_Int64 *)value->value.data;
            printf("Int64Value: %lld\n", (long long)hrValue);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_UINT64]) {
            UA_UInt64 hrValue = *(UA_UInt64 *)value->value.data;
            printf("UInt64Value: %llu\n", (unsigned long long)hrValue);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_STRING]) {
            UA_String hrValue = *(UA_String *)value->value.data;
            printf("StringValue: %.*s\n", (int)hrValue.length, hrValue.data);
        } else if (value->value.type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
            UA_Boolean hrValue = *(UA_Boolean *)value->value.data;
            printf("BooleanValue: %s\n", hrValue ? "true" : "false");
        } else {
            printf("Unsupported data type\n");
        }
    }

    return true;
}

void OPC_Client::setVariantValue(UA_Variant* variant, const std::string& value, const std::string& valuetype) {
    if (valuetype == "Boolean")
    {
        UA_Boolean convertedValue = value == "true";
        UA_Variant_setScalarCopy(variant, &convertedValue, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }
    else if (valuetype == "Int8")
    {
        UA_SByte convertedValue = static_cast<int8_t>(std::stoi(value));
        UA_Variant_setScalarCopy(variant, &convertedValue, &UA_TYPES[UA_TYPES_SBYTE]);    
    }
    else if (valuetype == "UInt8")
    {
        UA_Byte convertedValue = static_cast<uint8_t>(std::stoi(value));
        UA_Variant_setScalarCopy(variant, &convertedValue, &UA_TYPES[UA_TYPES_BYTE]);    
    }
    else if (valuetype == "Int16")
    {
        UA_Int16 convertedValue = static_cast<int16_t>(std::stoi(value));
        UA_Variant_setScalarCopy(variant, &convertedValue, &UA_TYPES[UA_TYPES_INT16]);
    }
    else if (valuetype == "UInt16")
    {
        UA_UInt16 convertedValue = static_cast<uint16_t>(std::stoi(value));
        UA_Variant_setScalarCopy(variant, &convertedValue, &UA_TYPES[UA_TYPES_UINT16]);
    }
    else if (valuetype == "Int32")
    {
        UA_Int32 convertedValue = std::stoi(value);
        UA_Variant_setScalarCopy(variant, &convertedValue, &UA_TYPES[UA_TYPES_INT32]);
    }
    else if (valuetype == "UInt32")
    {
        UA_UInt32 convertedValue = static_cast<uint32_t>(std::stoul(value));
        UA_Variant_setScalarCopy(variant, &convertedValue, &UA_TYPES[UA_TYPES_UINT32]);
    }
    else if (valuetype == "Int64")
    {
        UA_Int64 convertedValue = std::stoll(value);
        UA_Variant_setScalarCopy(variant, &convertedValue, &UA_TYPES[UA_TYPES_INT64]);
    }
    else if (valuetype == "UInt64")
    {
        UA_UInt64 convertedValue = std::stoull(value);
        UA_Variant_setScalarCopy(variant, &convertedValue, &UA_TYPES[UA_TYPES_UINT64]);
    }
    else if (valuetype == "Float")
    {
        UA_Float convertedValue = std::stof(value);
        UA_Variant_setScalarCopy(variant, &convertedValue, &UA_TYPES[UA_TYPES_FLOAT]);
    }
    else if (valuetype == "Double")
    {
        UA_Double convertedValue = std::stod(value);
        UA_Variant_setScalarCopy(variant, &convertedValue, &UA_TYPES[UA_TYPES_DOUBLE]);
    }
    else if (valuetype == "String")
    {
        UA_String convertedValue = {value.size(), (UA_Byte*) value.data()};
        UA_Variant_setScalarCopy(variant, &convertedValue, &UA_TYPES[UA_TYPES_STRING]);
    }
    else{
        std::cout << "Invalid Type Income" << std::endl;
    }
    printUAVariant(variant);
    return;
}

UA_ByteString StringToByteString(std::string input) {
    int len = strlen(input.c_str());
    int numBytes = (len + 1) / 3;
    UA_Byte *data = (UA_Byte *)malloc(numBytes);
    int index = 0;

    for(int i = 0; i < len; i += 3) {
        sscanf(&input[i], "%02hhx", &data[index]);
        index++;
    }

    UA_ByteString byteString;
    byteString.data = data;
    byteString.length = numBytes;

    return byteString;
}

void OPC_Client::onConnect (UA_Client *client, UA_SecureChannelState channelState, UA_SessionState sessionState, UA_StatusCode connectStatus) {
    // printf("Async connect returned with status code %s\n",  UA_StatusCode_name(connectStatus));
}
void OPC_Client::subscriptionInactivityCallback (UA_Client *client, UA_UInt32 subId, void *subContext) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Inactivity for subscription %u", subId);
}


void OPC_Client::fileBrowsed(UA_Client *client, void *userdata, UA_UInt32 requestId, UA_BrowseResponse *response) {
    printf("%-50s%u\n", "Received BrowseResponse for request ", requestId);
    printf("Number of browse results: %zu\n", response->resultsSize);
    
    for (size_t i = 0; i < response->resultsSize; i++) {
        UA_BrowseResult *result = &response->results[i];

        printf("Browse result #%zu:\n", i + 1);
        printf("  StatusCode: %s\n", UA_StatusCode_name(result->statusCode));
        printf("  ContinuationPoint: %.*s\n", (int) result->continuationPoint.length, result->continuationPoint.data);
        printf("  References:\n");

        for (size_t j = 0; j < result->referencesSize; j++) {
            UA_ReferenceDescription *ref = &result->references[j];
            printf("    Reference Type Id: %s\n", ref->referenceTypeId.identifier.string.data);
            printf("\tIs Forward: %s\n", ref->isForward ? "true" : "false");
            printf("\tNode Id: %.*s\n", (int) ref->nodeId.nodeId.identifier.string.length, ref->nodeId.nodeId.identifier.string.data);
            printf("\tBrowse Name: %.*s\n", (int) ref->browseName.name.length, ref->browseName.name.data);
            printf("\tDisplay Name: %.*s\n", (int) ref->displayName.text.length, ref->displayName.text.data);
            printf("\tNode Class: %d\n", ref->nodeClass);
            printf("\tType Definition: %.*s\n\n", (int) ref->typeDefinition.nodeId.identifier.string.length, ref->typeDefinition.nodeId.identifier.string.data);
        }
    }
}

/*high-level function callbacks*/
void readValueAttributeCallback(UA_Client *client, void *userdata, UA_UInt32 requestId, UA_StatusCode status, UA_DataValue *var) {
                            
    UA_Variant res = var->value;

    printUAVariant(&res);
    
    write_log("ERROR", "Failed to read the value", "/log/trans");
    printf("qweqwe\n\n");
    printf("%-50s%u\n", "Read value attribute for request", requestId);
    if(UA_Variant_hasScalarType(&var->value, &UA_TYPES[UA_TYPES_INT32])) {
        UA_Int32 int_val = *(UA_Int32*) var->value.data;
        printf("---%-40s%-8i\n",
               "Reading the value of node (1, \"the.answer\"):", int_val);
    }
}

void OPC_Client::writeValueAttributeCallback(UA_Client *client, void *userdata, UA_UInt32 requestId, UA_WriteResponse *response) {
    /*assuming no data to be retrieved by writing attributes*/
    if (response->responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        printf("%-50s%u\n", "Wrote value attribute for request ", requestId);
    } else {
        printf("Write failed\n");
    }
    UA_WriteResponse_clear(response);
}


void OPC_Client::methodCalled(UA_Client *client, void *userdata, UA_UInt32 requestId, UA_CallResponse *response) {
    printf("%-50s%u\n", "Called method for request ", requestId);
    size_t outputSize;
    UA_Variant *output;
    UA_StatusCode retval = response->responseHeader.serviceResult;
    if(retval == UA_STATUSCODE_GOOD)
    {
        if(response->resultsSize == 1)
            retval = response->results[0].statusCode;
        else
            retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    if(retval != UA_STATUSCODE_GOOD)
    {
        UA_CallResponse_clear(response);
        printf("---Method call was unsuccessful, returned %x values.\n", retval);
    }
    else
    {
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

void OPC_Client::OPC_Client_Uninitialize()
{

}
Json::Value OPC_Client::recv_MQTT(void)
{
    std::string input;
    std::string topic;

    Recv_OPC.resetMessage();
    input.clear();
    topic.clear();

    while (1) {
        input = Recv_OPC.getMessage();
        if (!input.empty()) {
            break;
        }
    }

    Json::Value root;
    Json::Value combinedJson;
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream is_stream(input);
    
    if (!Json::parseFromStream(builder, is_stream, &root, &errs)) {
        std::cerr << "Error parsing JSON: " << errs << std::endl;
    }

    if (!root.isMember("id"))
        return NULL;
    
    if(root["id"].asString() != clientID)
        return NULL;
    
    if (topic == "/modbus/exit")
    {
        OPC_Client_Uninitialize();
        return NULL;
    }
    
    return root;

}


void OPC_Client::alloc_json(Json::Value* input)
{
    Json::Value root = *input;
    if (root.isMember("id"))
        id = root["id"].asString();

    if (root.isMember("networkData"))
    {
        if (root["networkData"].isMember("endpointurl")) 
            serverUrl = root["networkData"]["endpointurl"].asString();

        if (root["networkData"].isMember("securitymode")) 
        {
            std::string securityMod = root["networkData"]["securitymode"].asString();
            if (securityMod == "None") {
                securityMode = UA_MESSAGESECURITYMODE_NONE;
            } else if (securityMod == "Sign") {
                securityMode = UA_MESSAGESECURITYMODE_SIGN;
            } else if (securityMod == "SignAndEncrypt") {
                securityMode = UA_MESSAGESECURITYMODE_SIGNANDENCRYPT;
            }
        }
        if (root["networkData"].isMember("securitypolicy")) 
        {
            std::string securityPolicy = root["networkData"]["securitypolicy"].asString();
            if (securityPolicy == "None") {
                securityPolicyUri = UA_String_fromChars("http://opcfoundation.org/UA/SecurityPolicy#None");
            } else if (securityPolicy == "Basic256") {
                securityPolicyUri = UA_String_fromChars("http://opcfoundation.org/UA/SecurityPolicy#Basic256");
            } else if (securityPolicy == "Basic128Rsa15") {
                securityPolicyUri = UA_String_fromChars("http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15");
            } else if (securityPolicy == "Basic256Sha256") {
                securityPolicyUri = UA_String_fromChars("http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256");
            }
        }

        if (root["networkData"].isMember("useridentify")) 
            useridentify = root["networkData"]["useridentify"].asString();
        if (root["networkData"].isMember("applicationuri")) 
            applicationUri = root["networkData"]["applicationuri"].asString();
        if (root["networkData"].isMember("username")) 
            username = root["networkData"]["username"].asString();
        if (root["networkData"].isMember("password")) 
            pwd = root["networkData"]["password"].asString();
        if (root["networkData"].isMember("msgCount")) 
            msgCount = root["networkData"]["msgCount"].asInt();
        if (root["networkData"].isMember("certFile")) 
            certfile = root["networkData"]["certFile"].asString();
        if (root["networkData"].isMember("keyFile")) 
            keyfile = root["networkData"]["keyFile"].asString();
    }
    if (root.isMember("msgData"))
    {
        if (root["msgData"][0].isMember("type")) 
            type = root["msgData"][0]["type"].asString();

        if (root["msgData"][0].isMember("inputArguments")) 
        {
            if (root["msgData"][0]["inputArguments"][0].isMember("size")) 
                value = root["msgData"][0]["inputArguments"][0]["size"].asString();
            if (root["msgData"][0]["inputArguments"][0].isMember("dataType")) 
                valuetype = root["msgData"][0]["inputArguments"][0]["dataType"].asString();
        }
            
        if (root["msgData"][0].isMember("nodeId")) 
            nodeid = root["msgData"][0]["nodeId"].asString();
        
        
        if (root["msgData"][0].isMember("RawBuffer")) 
            raw = root["msgData"][0]["RawBuffer"].asString();
    }
    
}
void OPC_Client::Client_Process(Json::Value* root)
{
    if((*root)["id"].asString() != clientID)
        return;
    
    alloc_json(root);
    UA_UInt32 reqId = 0;
    UA_Variant myVariant;
    UA_Variant_init(&myVariant);
    
    UA_SessionState ss;
    UA_Client_getState(client, NULL, &ss, NULL);
    
    if(ss == UA_SESSIONSTATE_ACTIVATED) {
        sscanf(nodeid.c_str(), "ns=%d;s=%s", &ns, s);
        if(type == "Read NodeId Value")
        {
            UA_Client_readValueAttribute_async(client, UA_NODEID_STRING(ns, s), readValueAttributeCallback, NULL, &reqId);
        }
        else if(type == "Write NodeId Value")
        {
            setVariantValue(&myVariant, value, valuetype);
            UA_Client_writeValueAttribute_async(client, UA_NODEID_STRING(ns, s), &myVariant, writeValueAttributeCallback, NULL, &reqId);
        }
        else if(type == "method call")
        {
            UA_Client_call_async(client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_STRING(ns, s), 1, &myVariant, methodCalled, NULL, &reqId);
        }
        else if(type == "rawBuffer send")
        {
            UA_ByteString convertedValue = StringToByteString(raw);
            UA_Variant_setScalarCopy(&myVariant, &convertedValue, &UA_TYPES[UA_TYPES_BYTESTRING]);
            UA_Client_writeValueAttribute_async(client, UA_NODEID_STRING(ns, s), &myVariant, writeValueAttributeCallback, NULL, &reqId);
            
        }
        
        else if(type == "Browse")
        {
            UA_BrowseRequest bReq;
            UA_BrowseRequest_init(&bReq);
            bReq.requestedMaxReferencesPerNode = 0;
            bReq.nodesToBrowse = UA_BrowseDescription_new();
            bReq.nodesToBrowseSize = 1;
            bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
            bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
            bReq.nodesToBrowse[0].browseDirection = UA_BROWSEDIRECTION_FORWARD;
            
            sleep_ms(100);

            UA_Client_sendAsyncBrowseRequest(client, &bReq, fileBrowsed, NULL, &reqId);
        }
        else if(type == "Historical Read")
        {
            UA_NodeId node = UA_NODEID_STRING(ns, s);
            UA_Client_HistoryRead_raw(client, &node, readHist, UA_DateTime_fromUnixTime(0), UA_DateTime_now(), UA_STRING_NULL, false, 10, UA_TIMESTAMPSTORETURN_BOTH, (void *)UA_FALSE);
        }
        else if(type == "Subscriptions")
        {
            UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
            UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client, request,
                                                                                    NULL, NULL, NULL);

            UA_UInt32 subId = response.subscriptionId;
            if(response.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
                printf("Create subscription succeeded, id %u\n", subId);

            UA_MonitoredItemCreateRequest monRequest =
                UA_MonitoredItemCreateRequest_default(UA_NODEID_STRING(ns, s));

            UA_MonitoredItemCreateResult monResponse =
            UA_Client_MonitoredItems_createDataChange(client, response.subscriptionId,
                                                    UA_TIMESTAMPSTORETURN_BOTH,
                                                    monRequest, NULL, DataChangeNotificationCallback, NULL);
                        
            if(monResponse.statusCode == UA_STATUSCODE_GOOD)
                printf("Monitoring 'the.answer', id %u\n", monResponse.monitoredItemId);
            
            
            pthread_t threadId;
            pthread_create(&threadId, NULL, &run_iterate_thread, client);
        }
    }

        UA_Client_run_iterate(client, 1000);
        UA_Variant_clear(&myVariant);
        return;

}
int OPC_Client::start(Json::Value* root)
{
    
    alloc_json(root);

    clientID = id;
    mqttid = 0;

    Send_OPC_Log._pub_client_id = "OPCUA_Client_Pub_ID:" + clientID;
    Send_OPC_Log.Publish_Initialize();
    Recv_OPC._sub_client_id = "OPCUA_Client_Sub_ID:" + clientID;
    Recv_OPC.Subscribe_Initialize();

    
    int start_time = time(NULL);
    while(!Send_OPC_Log.isConnected)
	{
        sleep(1);
		if(time(NULL) - start_time > 5)
		{
			printf("MQTT Publish Connection Timeout : 5 Seconds\n");
			return 0;
		}
	}
    
    client = UA_Client_new();
    UA_ClientConfig *cc = UA_Client_getConfig(client);
    
    cc->securityMode = securityMode;
    cc->securityPolicyUri = securityPolicyUri;
    cc->stateCallback = onConnect;
    cc->subscriptionInactivityCallback = subscriptionInactivityCallback;

    UA_ApplicationDescription_clear(&cc->clientDescription);
    cc->clientDescription.applicationUri = UA_STRING_ALLOC(applicationUri.c_str());
    cc->clientDescription.applicationType = UA_APPLICATIONTYPE_CLIENT;

    UA_Logger logger;
    logger.log = write_log_callback;
    cc->logger = logger;

    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    if(useridentify == "Anonymous")
    {
        UA_ClientConfig_setDefault(UA_Client_getConfig(client));
        retval = UA_Client_connect(client, serverUrl.c_str());
    }
    else if(useridentify == "UserName")
    {
        if(!username.empty()) {
            UA_ClientConfig_setAuthenticationUsername(cc, username.c_str(), pwd.c_str());
            retval = UA_Client_connect(client, serverUrl.c_str());
            // retval = UA_Client_connectUsername(client, serverUrl, username, pwd);
        }
        else if(!certfile.empty()) {
            // certificate = loadFile(certfile.c_str());
            // privateKey  = loadFile(keyfile.c_str());

            if(!trustList.empty())
            {
                size_t trustListSize = 1;
                UA_STACKARRAY(UA_ByteString, trustListAuth, trustListSize);
                // trustListAuth[0] = loadFile(trustList.c_str());
                UA_ClientConfig_setDefaultEncryption(cc, certificate, privateKey,
                                                    trustListAuth, trustListSize, NULL, 0);
                UA_ByteString_clear(&trustListAuth[0]);
            }
            else
            {
                UA_ClientConfig_setDefaultEncryption(cc, certificate, privateKey, NULL, 0, NULL, 0);
                UA_CertificateVerification_AcceptAll(&cc->certificateVerification);
            }

            UA_ClientConfig_setAuthenticationCert(cc, certificate, privateKey);
            
            retval = UA_Client_connect(client, serverUrl.c_str());
            
            UA_ByteString_clear(&certificate);
            UA_ByteString_clear(&privateKey);
        }
    }
    
        
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Could not connect");
        write_log("ERROR", "Could not connect", "/log/trans");
        UA_Client_delete(client);
        return 0;
    }
     
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Connected!");
    write_log("SYSTEM", "OPCUA CLIENT START", "/log/system");
    
    bool isfirst = true;
    Json::Value OPC_root;
    
    while(1) {   
        OPC_root.clear();
        if(isfirst) {
            OPC_root = *root;
            isfirst = false;
        }
        else {
            OPC_root = recv_MQTT();
        }        
        
        Client_Process(&OPC_root);
        
    }
    
        /* Clean up */
        // uaClientThread.join(); // Wait for uaClientThread to finish
        UA_Client_disconnect(client);
        UA_Client_delete(client);
        return 0;

}