// #include <open62541/client_highlevel.h>
// #include <open62541/plugin/pki_default.h>
// #include <open62541/client_config_default.h>
// #include <open62541/client_highlevel_async.h>
// #include <open62541/client_subscriptions.h>
// #include <open62541/plugin/log_stdout.h>
// #include <open62541/server_config_default.h>
// #include <open62541/client_subscriptions.h>
// #include <stdlib.h>
// #include <iostream>
// // Subscription callback 함수
// static void DataChangeNotificationCallback(UA_Client *client, UA_UInt32 subId, void *subContext, UA_UInt32 monId, void *monContext, UA_DataValue *value) {
//     if (value->hasValue && UA_Variant_hasScalarType(&value->value, &UA_TYPES[UA_TYPES_STRING])) {
//         UA_String data = *(UA_String *)value->value.data;
//         printf("Value changed: %.*s\n", (int)data.length, data.data);
//     }
// }

// int main(void) {
//     UA_Client *client = UA_Client_new();
//     UA_ClientConfig_setDefault(UA_Client_getConfig(client));

//     UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://DESKTOP-L89F0IQ:4880/OPCUA/SimulationServer"); // OPC UA 서버에 연결

//     if (retval != UA_STATUSCODE_GOOD) {
//         UA_Client_delete(client);
//         return (int)retval;
//     }

//     // 구독 생성
//     UA_UInt32 subscriptionId = 1;
//     UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
//     UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client, request, NULL, NULL, NULL);

//     // 모니터링 파라미터 설정
//     UA_MonitoredItemCreateRequest monRequest = UA_MonitoredItemCreateRequest_default(UA_NODEID_STRING(3, "test"));
//     UA_MonitoredItemCreateResult monResponse = UA_Client_MonitoredItems_createDataChange(client, response.subscriptionId, UA_TIMESTAMPSTORETURN_BOTH, monRequest, DataChangeNotificationCallback, DataChangeNotificationCallback);

//     if (monResponse.statusCode != UA_STATUSCODE_GOOD) {
//         UA_Client_disconnect(client);
//         UA_Client_delete(client);
//         return (int)monResponse.statusCode;
//     }

//     // 이벤트 루프
//     UA_Client_run_iterate(client, 10000); // 예제에서는 10초 동안 이벤트 루프 실행

//     // 클라이언트 정리
//     UA_Client_disconnect(client);
//     UA_Client_delete(client);

//     return 0;
// }




// #include <iostream>
// #include <pthread.h>
// #include <chrono>
// const int num_threads = 60; // 60개의 스레드 생성
// // 스레드에서 실행될 함수
// void* performSummation(void* thread_id) {
//     long tid = (long)thread_id;
//     // 여기에 원하는 덧셈 연산을 수행하는 코드를 넣어주세요.
//     // 예시: 1부터 100까지 더하는 작업을 수행한다고 가정
//     int sum = 0;
//     for (int i = 1; i <= 100; ++i) {
//         sum += i;
//     }
//     std::cout << "Thread " << tid << "의 결과: " << sum << std::endl;
//     pthread_exit(NULL);
// }
// int main() {
//     pthread_t threads[num_threads];
//     auto start = std::chrono::steady_clock::now(); // 시작 시간 기록
//     // 60번의 스레드 생성
//     for (long i = 0; i < num_threads; ++i) {
//         pthread_create(&threads[i], NULL, performSummation, (void*)i);
//     }
//     // 모든 스레드가 종료될 때까지 대기
//     for (int i = 0; i < num_threads; ++i) {
//         pthread_join(threads[i], NULL);
//     }
//     auto end = std::chrono::steady_clock::now(); // 끝 시간 기록
//     std::chrono::duration<double> elapsed_seconds = end - start; // 전체 걸린 시간 계산
//     std::cout << "전체 시간: " << elapsed_seconds.count() << " 초\n";
//     return 0;
// }

// #include <iostream>
// #include <modbus/modbus.h>
// #include <chrono>

// #define SERVER_IP "192.168.1.34"
// #define SERVER_PORT 502
// #define START_ADDRESS 0
// #define NUM_REGISTERS 125

// int main() {
//     auto client_start_time = std::chrono::steady_clock::now();
//     modbus_t *ctx;
//     int rc;

//     ctx = modbus_new_tcp(SERVER_IP, SERVER_PORT);
//     if (ctx == nullptr) {
//         std::cerr << "Unable to create the libmodbus context" << std::endl;
//         return 1;
//     }

//     if (modbus_connect(ctx) == -1) {
//         std::cerr << "Connection failed: " << modbus_strerror(errno) << std::endl;
//         modbus_free(ctx);
//         return 1;
//     }

//     int starting_address = START_ADDRESS;
//     int num_registers = NUM_REGISTERS;
//     for(int i=0;i<60;i++)
//     {
//         uint16_t tab_reg[4*1024];
//         int cnt = 0;
//         rc = modbus_read_input_registers(ctx, starting_address, num_registers, tab_reg);
//         if (rc == -1) {
//             std::cerr << "Failed to read: " << modbus_strerror(errno) << std::endl;
//         } else {
//             for (int i = 0; i < num_registers; ++i) {
//                 std::cout << "Input Register " << starting_address + cnt << ": " << tab_reg[cnt] << std::endl;
//                 cnt++;
//             }
//         }
//         rc = modbus_read_input_registers(ctx, starting_address, num_registers, tab_reg);
//         if (rc == -1) {
//             std::cerr << "Failed to read: " << modbus_strerror(errno) << std::endl;
//         } else {
//             for (int i = 0; i < num_registers; ++i) {
//                 std::cout << "Input Register " << starting_address + cnt << ": " << tab_reg[cnt] << std::endl;
//                 cnt++;
//             }
//         }
//         rc = modbus_read_input_registers(ctx, starting_address, num_registers, tab_reg);
//         if (rc == -1) {
//             std::cerr << "Failed to read: " << modbus_strerror(errno) << std::endl;
//         } else {
//             for (int i = 0; i < 256-(2*NUM_REGISTERS); ++i) {
//                 std::cout << "Input Register " << starting_address + cnt << ": " << tab_reg[cnt] << std::endl;
//                 cnt++;
//             }
//         }

//         printf("\n");
//     }

//     modbus_close(ctx);
//     modbus_free(ctx);

//     auto end = std::chrono::steady_clock::now();
//     std::chrono::duration<double> elapsed_seconds = end - client_start_time; // 시간 차이 계산
//     std::cout << "Elaspsed time: " << elapsed_seconds.count() << " sec\n";

//     return 0;
// }



#include <iostream>
#include <modbus/modbus.h>
#include <chrono>
#include <fstream>
#include <string>
#include <string.h>

int main()
{
    char SERVER_IP[256];
    int SERVER_PORT;
    int START_ADDRESS;
    int NUM_REGISTERS;

    FILE *fp;
    char setting_arr[8][32];
    char tmp[32];
    char *ptr;

    fp = fopen("setting.config", "r");
    for(int i=0;i<4;i++)
    {
        fgets(tmp, sizeof(tmp), fp);
        strtok(tmp, "=");
        ptr = strtok(NULL, "\n");
        strcpy(setting_arr[i], ptr);
    }

    strcpy(SERVER_IP, setting_arr[0]);
    SERVER_PORT = atoi(setting_arr[1]);
    START_ADDRESS = atoi(setting_arr[2]);
    NUM_REGISTERS = atoi(setting_arr[3]);

    auto client_start_time = std::chrono::steady_clock::now();
    modbus_t *ctx;
    int rc;

    ctx = modbus_new_tcp(SERVER_IP, SERVER_PORT);
    if (ctx == nullptr) {
        std::cerr << "Unable to create the libmodbus context" << std::endl;
        return 1;
    }
    
    if (modbus_connect(ctx) == -1) {
        std::cerr << "Connection failed: " << modbus_strerror(errno) << std::endl;
        modbus_free(ctx);
        return 1;
    }

    for(int i=0;;i++)
    {
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - client_start_time; // 시간 차이 계산
        if(elapsed_seconds.count() > 1.0){
            break;
        }
        printf("iter: %d\n",i+1);
        uint16_t tab_reg[256];
        for(int j=0;j<3;j++)
        {
            rc = modbus_read_input_registers(ctx, START_ADDRESS+(j*NUM_REGISTERS), NUM_REGISTERS, tab_reg+(j*NUM_REGISTERS));
            if (rc == -1) {
                std::cerr << "Failed to read: " << modbus_strerror(errno) << std::endl;
            }
        }
        for (int i = 0; i < 256; ++i) {
            std::cout << "Input Register " << i << ": " << tab_reg[i] << std::endl;
        }

    }
    

    modbus_close(ctx);
    modbus_free(ctx);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - client_start_time; // 시간 차이 계산
    std::cout << "Elaspsed time: " << elapsed_seconds.count() << " sec\n";

    return 0;
}

