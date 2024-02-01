#include <open62541/server.h>

using namespace std;

int main() {
    UA_Server *server = UA_Server_new(); //서버 객체 생성

    // adress space에 변수 노드 추가
    UA_VariableAttributes attr = UA_VariableAttributes_default; //변수 노드의 속성 초기화
    UA_Int32 myInteger = 42;
    UA_Variant_setScalarCopy(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]); //변수 값 설정
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US","the answer"); //속성을 영문으로 설정
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US","the answer");
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING_ALLOC(1, "the.answer"); //Node ID 설정
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME_ALLOC(1, "the answer"); //변수 노드 이름 설정
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); 
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES); //부모 노드 지정
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
                              parentReferenceNodeId, myIntegerName,
                              UA_NODEID_NULL, attr, NULL, NULL); // 서버에 변수 노드 추가

    //할당된 메모리 해제
    UA_VariableAttributes_clear(&attr);
    UA_NodeId_clear(&myIntegerNodeId);
    UA_QualifiedName_clear(&myIntegerName);

    UA_StatusCode retval = UA_Server_runUntilInterrupt(server);

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}

