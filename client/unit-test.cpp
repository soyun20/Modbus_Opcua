/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <modbus/modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
// clang-format off
#ifdef _WIN32
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif

/* For MinGW */
#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0

#endif
// clang-format on

#include "unit-test.h.in"
modbus_mapping_t *mb_mapping;

enum {
    TCP,
    TCP_PI,
    RTU
};

void signal_handler(int sig){
    if(sig == SIGINT){
        printf("\nSIGINT received...\nShutdown\n");
        exit(0);
    }
    else if(sig == SIGPIPE){
        printf("\nSIGPIPE received...\nIgnored\n");
    }
}

void* client_handler(void *arg){
    modbus_t *ctx = (modbus_t *)arg;
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int rc;
    int header_length = modbus_get_header_length(ctx);

    while(0) {
        do {
            rc = modbus_receive(ctx, query);
        } while (rc == 0);

        if (rc == -1 && errno != EMBBADCRC) {
            break;
        }

        rc = modbus_reply(ctx, query, rc, mb_mapping);

        if (rc == -1) {
            break;
        }
    }

    printf("Exit client handler: %s\n", modbus_strerror(errno));

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int s = -1;
    modbus_t *ctx;
    
    int rc;
    int i;
    int use_backend;
    uint8_t *query;
    int header_length;
    char *ip_or_device;

    signal(SIGINT, signal_handler);
    signal(SIGPIPE, signal_handler);


    if (argc > 1) {
        if (strcmp(argv[1], "tcp") == 0) {
            use_backend = TCP;
        } else if (strcmp(argv[1], "tcppi") == 0) {
            use_backend = TCP_PI;
        } else if (strcmp(argv[1], "rtu") == 0) {
            use_backend = RTU;
        } else {
            printf("Modbus server for unit testing.\n");
            printf("Usage:\n  %s [tcp|tcppi|rtu] [<ip or device>]\n", argv[0]);
            printf("Eg. tcp 127.0.0.1 or rtu /dev/ttyUSB0\n\n");
            return -1;
        }
    } else {
        use_backend = TCP;
    }

    if (argc > 2) {
        ip_or_device = argv[2];
    } else {
        switch (use_backend) {
        case TCP:
            ip_or_device = "0.0.0.0";
            break;
        case TCP_PI:
            ip_or_device = "::1";
            break;
        case RTU:
            ip_or_device = "/dev/ttyUSB0";
            break;
        default:
            break;
        }
    }
    int modbus_write_register(modbus_t *ctx, int addr, const uint16_t value);
    if (use_backend == TCP) {
        ctx = modbus_new_tcp(ip_or_device, 1502);
        query = (uint8_t *)malloc(MODBUS_TCP_MAX_ADU_LENGTH);
    } else if (use_backend == TCP_PI) {
        ctx = modbus_new_tcp_pi(ip_or_device, "502");
        query = (uint8_t *)malloc(MODBUS_TCP_MAX_ADU_LENGTH);
    } else {
        ctx = modbus_new_rtu(ip_or_device, 115200, 'N', 8, 1);
        modbus_set_slave(ctx, SERVER_ID);
        query = (uint8_t *)malloc(MODBUS_RTU_MAX_ADU_LENGTH);
    }

    header_length = modbus_get_header_length(ctx);
    
    modbus_set_debug(ctx, TRUE);

    mb_mapping = modbus_mapping_new_start_address(UT_BITS_ADDRESS,
                                                  UT_BITS_NB,
                                                  UT_INPUT_BITS_ADDRESS,
                                                  UT_INPUT_BITS_NB,
                                                  UT_REGISTERS_ADDRESS,
                                                  UT_REGISTERS_NB_MAX,
                                                  UT_INPUT_REGISTERS_ADDRESS,
                                                  UT_INPUT_REGISTERS_NB);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Examples from PI_MODBUS_300.pdf.
       Only the read-only input values are assigned. */

    /* Initialize input values that's can be only done server side. */
    modbus_set_bits_from_bytes(
        mb_mapping->tab_input_bits, 0, UT_INPUT_BITS_NB, UT_INPUT_BITS_TAB);

    /* Initialize values of INPUT REGISTERS */
    for (i = 0; i < UT_REGISTERS_NB; i++) {
        mb_mapping->tab_registers[i] = UT_REGISTERS_TAB[i];
    }

    if (use_backend == TCP) {
        s = modbus_tcp_listen(ctx, 1);
    } else if (use_backend == TCP_PI) {
        s = modbus_tcp_pi_listen(ctx, 1);
    } 

    while (1) {
        if (use_backend == TCP) {
            modbus_tcp_accept(ctx, &s);
        } else if (use_backend == TCP_PI) {
            modbus_tcp_pi_accept(ctx, &s);
        } 
        printf("asd");
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, client_handler, (void *)ctx);
        pthread_detach(client_thread);
    }

    printf("Shutting down server...\n");

        

    printf("Quit the loop: %s\n", modbus_strerror(errno));

    if (use_backend == TCP) {
        if (s != -1) {
            close(s);
        }
    }
    modbus_mapping_free(mb_mapping);
    free(query);
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
