#ifndef MODBUS_TCP_H
#define MODBUS_TCP_H

#include <pthread.h>

class Modbus_TCP_Class
{
public:
	int DATA_SIZE;
	int START_ADDRESS;
	int READ_SIZE = 12;
    int mem = 1234;
	unsigned char s_buff[0x007D] = {0x00, };

	int isModbus(unsigned char* input);
	void Set_Input_Output(unsigned char* input);
	int FindError();
    
private:
	unsigned char INPUT_TID_1;
	unsigned char INPUT_TID_2;
	unsigned char INPUT_PID_1;
	unsigned char INPUT_PID_2;
	unsigned char INPUT_LENGTH_1;
	unsigned char INPUT_LENGTH_2;
	unsigned char INPUT_UID;
	unsigned char INPUT_FUNCTION_CODE;
	unsigned char INPUT_STARTING_ADDRESS_1;
	unsigned char INPUT_STARTING_ADDRESS_2;
	unsigned char INPUT_DATA_SIZE_1;
	unsigned char INPUT_DATA_SIZE_2;

	unsigned char OUTPUT_TID_1;
	unsigned char OUTPUT_TID_2;
	unsigned char OUTPUT_PID_1;
	unsigned char OUTPUT_PID_2;
	unsigned char OUTPUT_LENGTH_1;
	unsigned char OUTPUT_LENGTH_2;
	unsigned char OUTPUT_UID;
	unsigned char OUTPUT_FUNCTION_CODE;
	unsigned char OUTPUT_BYTE_COUNT;

	unsigned char ERROR_CODE;
	unsigned char EXCEPTION_CODE;

    typedef struct {
        pthread_mutex_t mutex;
        unsigned char data[10];
    } shared_data;

    shared_data* ShmSendBuffer;
	int ShmId;
};

#endif