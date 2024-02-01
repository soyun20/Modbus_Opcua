#include <iostream>
#include <modbus/modbus.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1" // 서버 IP 주소
#define SERVER_PORT 1502        // 서버 포트 번호

int main() {
    modbus_t *ctx;
    int server_socket;
    int rc;
    uint16_t tab_reg[32];

    ctx = modbus_new_tcp(SERVER_IP, SERVER_PORT);
    if (ctx == nullptr) {
        std::cerr << "Unable to create the libmodbus context" << std::endl;
        return 1;
    }

    if (modbus_set_slave(ctx, 1) == -1) {
        fprintf(stderr, "Failed to set slave id: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return 0;
    }

    static modbus_mapping_t *mb_mapping;
    int coils = 1024;
    int distreteInputs = 1024;
    int inputRegisters = 1024;
    int holdingRegisters = 1024;
    mb_mapping = modbus_mapping_new_start_address( 0,  coils,
                                                   0,  distreteInputs,
                                                   0,  inputRegisters,
                                                   0,  holdingRegisters);

    // Slave 메모리 값 설정
    const uint8_t UT_BITS_TAB[] = { 0xCD, 0x6B, 0xB2, 0x0E, 0x1B };
    const uint8_t UT_INPUT_BITS_TAB[] = { 0xAC, 0xDB, 0x35 };
    const uint16_t UT_INPUT_REGISTERS_TAB[] = { 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A };
    const uint16_t UT_REGISTERS_TAB[] = { 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A};

    modbus_set_bits_from_bytes(mb_mapping->tab_bits, 0, coils, UT_BITS_TAB);
    modbus_set_bits_from_bytes(mb_mapping->tab_input_bits, 0, distreteInputs, UT_INPUT_BITS_TAB);

    server_socket = modbus_tcp_listen(ctx, 1);
    if (server_socket == -1) {
        fprintf(stderr, "Unable to listen connection\n");
        modbus_free(ctx);
        return -1;
    }

    rc = modbus_connect(ctx);
    if (rc == -1) {
        fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }



    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
