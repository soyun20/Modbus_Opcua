#include <stdio.h>
#include <stdlib.h>
#include <modbus/modbus.h>
#include <errno.h>

int main() {
    modbus_t *ctx;
    //uint8_t raw_req[] = {0x01, 0x10, 0x00, 0x06, 0x00, 0x02, 0x04, 0x00, 0x01, 0x00, 0x02}; // 예시로 사용할 원시 요청 패킷
    uint8_t raw_req[] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x01}; // 예시로 사용할 원시 요청 패킷
    uint8_t rsp[MODBUS_RTU_MAX_ADU_LENGTH];
    printf("raw_request: \n");
    for(int i=0;i<sizeof(raw_req);i++){
        printf("%02x ",raw_req[i]);
    }
    printf("\n");

    // Modbus 컨텍스트 초기화
    ctx = modbus_new_tcp("10.7.12.94", 1502); // TCP 연결 설정

    if (ctx == NULL) {
        fprintf(stderr, "Unable to create Modbus context\n");
        return 1;
    }

    // 연결 설정 및 연결 시도
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Modbus connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return 1;
    }

    // 원시 요청 패킷 전송
    int ret = modbus_send_raw_request(ctx, raw_req, sizeof(raw_req));
    //int ret = modbus_write_register(ctx, 0x00, 18);
    if (ret == -1) {
        fprintf(stderr, "Error sending raw request: %s\n", modbus_strerror(errno));
    } else {
        printf("Raw request sent successfully\n");
    }
    int rc = modbus_receive_confirmation(ctx, rsp);
    if (rc == -1) {
        fprintf(stderr, "Failed to receive confirmation: %s\n", modbus_strerror(errno));
    }
    else{
        printf("Raw request sent successfully2\n");
    }
    

    // 연결 종료 및 메모리 해제
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
