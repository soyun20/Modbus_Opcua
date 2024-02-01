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


    int division_time;
    for(int i=0;;i++)
    {
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - client_start_time; // 시간 차이 계산
        if(elapsed_seconds.count() > 600.0){
            break;
        }

        for(int i=0;i<100;i++)
        {
            auto request_start_time = std::chrono::steady_clock::now();
            printf("iter: %d\n",i);
            uint16_t tab_reg[1024];
            
            rc = modbus_read_input_registers(ctx, START_ADDRESS, NUM_REGISTERS, tab_reg);
            if (rc == -1) {
                std::cerr << "Failed to read: " << modbus_strerror(errno) << std::endl;
            }
            std::cout << "Input Register " << START_ADDRESS + 0 << ": " << tab_reg[0] << std::endl;
            std::cout << "Input Register " << START_ADDRESS + 10 << ": " << tab_reg[10] << std::endl;
            std::cout << "Input Register " << START_ADDRESS + 124 << ": " << tab_reg[124] << "\n" << std::endl;
 
            auto request_end_time = std::chrono::steady_clock::now();

            std::chrono::steady_clock::duration request_elapsed_time = request_end_time - request_start_time;
            std::chrono::microseconds elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(request_elapsed_time);

            // 대기할 시간 계산 (1초 - 요청에 걸린 시간)
            std::chrono::microseconds ten_micro_second(10000);

            division_time = (ten_micro_second - elapsed_microseconds).count();

            if (division_time > 0) {
                usleep(division_time);
            }
            auto see_request_end_time = std::chrono::steady_clock::now();
            std::chrono::duration<double> see_elapsed_seconds = see_request_end_time - request_start_time;
            std::cout << "wow Elaspsed time: " << see_elapsed_seconds.count() << " sec\n";
        }
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - client_start_time;
    std::cout << "Elaspsed time: " << elapsed_seconds.count() << " sec\n";


    modbus_close(ctx);
    modbus_free(ctx);
    
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end - client_start_time;
    std::cout << "Modbus Close Elaspsed time: " << elapsed_seconds.count() << " sec\n";
    
    return 0;
}