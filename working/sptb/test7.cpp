#include <iostream>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>

char SERVER_IP[256];
int SERVER_PORT;
int START_ADDRESS;
int DATA_SIZE = 125;
int SEND_SIZE = 12;
int READ_SIZE = 9 + DATA_SIZE * 2;
int sock;

void Print_Hexa_Buff(char *buff, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++)
    {
        printf("%02X ", (unsigned char)buff[i]);
    }
    printf("\n");
}

void modbusReadRequest() {
    auto client_start_time = std::chrono::steady_clock::now();
    char s_buff[SEND_SIZE];
    int ret;

    memset(s_buff, 0, SEND_SIZE);

    for (int i = 0; i < 5; i++){
        s_buff[i] = 0x00; // TID, PID, Length_1
    }
    
    s_buff[5] = 0x06;     // Length_2
    s_buff[6] = 0x01;     // Unit ID
    s_buff[7] = 0x04;     // Function Code

    s_buff[8] = (START_ADDRESS >> 8) & 0xFF;          // Data Start Adress_1
    s_buff[9] = START_ADDRESS & 0xFF;          // Data Start Adress_2

    s_buff[10] = (DATA_SIZE >> 8) & 0xFF;
    s_buff[11] = DATA_SIZE & 0xFF;

    int count = 0;
    int division_time;
    while (1)
    {
        auto client_end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = client_end_time - client_start_time; // 시간 차이 계산
        if(elapsed_seconds.count() > 5.0){
            break;
        }
        
        for(int i=0;i<50;i++)
        {
            auto request_start_time = std::chrono::steady_clock::now();
            s_buff[1] = 0x00 + count;
            count++;

            ret = send(sock, s_buff, SEND_SIZE, 0);
            // if (ret >= 0)
            // {
            //     printf("Sended Byte\t:");
            //     Print_Hexa_Buff(s_buff, SEND_SIZE);
            // }
            // else
            // {
            //     printf("Send Failed \n");
            //     return;
            // }
            // sleep(1);

            auto request_end_time = std::chrono::steady_clock::now();

            std::chrono::steady_clock::duration request_elapsed_time = request_end_time - request_start_time;
            std::chrono::microseconds elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(request_elapsed_time);

            // 대기할 시간 계산 (1초 - 요청에 걸린 시간)
            std::chrono::microseconds term_micro_second(20000);

            division_time = (term_micro_second - elapsed_microseconds).count();

            if (division_time > 0) {
                usleep(division_time);
            }
        }
    }
    auto client_end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = client_end_time - client_start_time;
    std::cout << "Elaspsed time: " << elapsed_seconds.count() << " sec\n";
    
}

void modbusReadResponse() {
    char r_buff[READ_SIZE];
    int ret;

    while (1)
    {
        memset(r_buff, 0, READ_SIZE);
        ret = recv(sock, r_buff, READ_SIZE, 0);

        if (ret >= 0)
        {
            printf("Received Byte\t:");
            Print_Hexa_Buff(r_buff, READ_SIZE);
            printf("Received Data\t:");
            for (int i = 0; i < DATA_SIZE*2; i += 2)
                printf("%d ", (r_buff[i + 9] << 8) | r_buff[i + 10]);
            printf("\n\n");
        }
        else
        {
            printf("Receive Failed");
            break;
        }
    }
    
}

int main() {
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
    DATA_SIZE = atoi(setting_arr[3]);

    READ_SIZE = 9 + DATA_SIZE * 2;


    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("errno is %d", errno);
        return -1;
    }
    // connect
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serveraddr.sin_port = htons(SERVER_PORT);

    int ret = connect(sock, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr));
    if (ret != 0)
    {
        printf("errno is %d", errno);
        return -1;
    }

    std::thread requestThread(modbusReadRequest);
    std::thread responseThread(modbusReadResponse);

    requestThread.join();
    // responseThread.join();

    return 0;
}

