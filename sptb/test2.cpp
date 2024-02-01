// #include <iostream>
// #include <modbus/modbus.h>
// #include <chrono>
// #include <fstream>
// #include <string>
// #include <string.h>

// int main()
// {
//     char SERVER_IP[256];
//     int SERVER_PORT;
//     int START_ADDRESS;
//     int NUM_REGISTERS;

//     FILE *fp;
//     char setting_arr[8][32];
//     char tmp[32];
//     char *ptr;

//     fp = fopen("setting.config", "r");
//     for(int i=0;i<4;i++)
//     {
//         fgets(tmp, sizeof(tmp), fp);
//         strtok(tmp, "=");
//         ptr = strtok(NULL, "\n");
//         strcpy(setting_arr[i], ptr);
//     }

//     strcpy(SERVER_IP, setting_arr[0]);
//     SERVER_PORT = atoi(setting_arr[1]);
//     START_ADDRESS = atoi(setting_arr[2]);
//     NUM_REGISTERS = atoi(setting_arr[3]);

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

//     for(int i=0;;i++)
//     {
//         auto end = std::chrono::steady_clock::now();
//         std::chrono::duration<double> elapsed_seconds = end - client_start_time; // 시간 차이 계산
//         if(elapsed_seconds.count() > 1.0){
//             break;
//         }
//         printf("iter: %d\n",i+1);
//         uint16_t tab_reg[4*1024];
//         for(int j=0;j<3;j++)
//         {
//             rc = modbus_read_input_registers(ctx, START_ADDRESS+(j*NUM_REGISTERS), NUM_REGISTERS, tab_reg+(j*NUM_REGISTERS));
//             if (rc == -1) {
//                 std::cerr << "Failed to read: " << modbus_strerror(errno) << std::endl;
//             }
//         }
//         std::cout << "Input Register " << START_ADDRESS + 0 << ": " << tab_reg[0] << std::endl;
//         std::cout << "Input Register " << START_ADDRESS + 10 << ": " << tab_reg[10] << std::endl;
//         std::cout << "Input Register " << START_ADDRESS + 256 << ": " << tab_reg[256] << "\n" << std::endl;

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

    int division_time = 12000;
    for(int i=0;;i++)
    {
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - client_start_time; // 시간 차이 계산
        if(elapsed_seconds.count() > 5.0){
            break;
        }

        for(int i=0;i<60;i++)
        {
            printf("iter: %d\n",i);
            uint16_t tab_reg[4*1024];
            for(int j=0;j<3;j++)
            {
                rc = modbus_read_input_registers(ctx, START_ADDRESS+(j*NUM_REGISTERS), NUM_REGISTERS, tab_reg+(j*NUM_REGISTERS));
                if (rc == -1) {
                    std::cerr << "Failed to read: " << modbus_strerror(errno) << std::endl;
                }
            }
            std::cout << "Input Register " << START_ADDRESS + 0 << ": " << tab_reg[0] << std::endl;
            std::cout << "Input Register " << START_ADDRESS + 10 << ": " << tab_reg[10] << std::endl;
            std::cout << "Input Register " << START_ADDRESS + 256 << ": " << tab_reg[256] << "\n" << std::endl;
            usleep(division_time);
        }
    }

    modbus_close(ctx);
    modbus_free(ctx);
    
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - client_start_time; // ?쒓컙 李⑥씠 怨꾩궛
    std::cout << "Elaspsed time: " << elapsed_seconds.count() << " sec\n";
    
    return 0;
}