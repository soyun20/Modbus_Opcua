#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <stdio.h>

int main() {
    UA_Client *client = UA_Client_new(); // 클라이언트 객체 생성
    UA_ClientConfig *config = UA_Client_getConfig(client); //클라이언트 설정 가져오기
    config->timeout = 5000; //연결 시도 타임아웃 설정

    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); //서버와 연결
    if (retval != UA_STATUSCODE_GOOD) {
        printf("Failed to connect to the server\n");
        UA_Client_delete(client);
        return retval;
    }

    //"the.answer" 노드의 값 읽기
    UA_Variant value;
    UA_Variant_init(&value);
    retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(1, "the.answer"), &value);

    if (retval == UA_STATUSCODE_GOOD) {
        UA_Int32 answer = *(UA_Int32 *)value.data;
        printf("Read Value: the.answer = %d\n", answer);
    } else {
        printf("Failed to read the.answer attribute\n");
    }

    //할당된 메모리 해제
    UA_Variant_deleteMembers(&value);
    UA_Client_disconnect(client); // 서버와 연결 해제
    UA_Client_delete(client);

    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}


