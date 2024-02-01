#include "Modbus_TCP.h"
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

int SUPPORTED_FUNCTION[] = {0x03};
int SUPPORTED_ADDRESS[] = {0, 10};

int _2_HEX_TO_1_HEX(unsigned char HEX1, unsigned char HEX2){
	return(HEX1 << 8) | HEX2;
}


void Modbus_TCP_Class::Set_Input_Output(unsigned char* input)
{
	INPUT_TID_1 = input[0];
	INPUT_TID_2 = input[1];
	INPUT_PID_1 = input[2];
	INPUT_PID_2 = input[3];
	INPUT_LENGTH_1 = input[4];
	INPUT_LENGTH_2 = input[5];
	INPUT_UID = input[6];
	INPUT_FUNCTION_CODE = input[7];
	INPUT_STARTING_ADDRESS_1 = input[8];
	INPUT_STARTING_ADDRESS_2 = input[9];
	INPUT_DATA_SIZE_1 = input[10];
	INPUT_DATA_SIZE_2 = input[11];

	DATA_SIZE = _2_HEX_TO_1_HEX(INPUT_DATA_SIZE_1, INPUT_DATA_SIZE_2);
	START_ADDRESS = _2_HEX_TO_1_HEX(INPUT_STARTING_ADDRESS_1, INPUT_STARTING_ADDRESS_2);

	OUTPUT_TID_1 = INPUT_TID_1;
	OUTPUT_TID_2 = INPUT_TID_2;
	OUTPUT_PID_1 = INPUT_PID_1;
	OUTPUT_PID_2 = INPUT_PID_2;
	OUTPUT_LENGTH_1 = ((DATA_SIZE + 5) >> 8) & 0xFF;
	OUTPUT_LENGTH_2 = (DATA_SIZE + 5) & 0xFF;
	OUTPUT_UID = INPUT_UID;
	OUTPUT_FUNCTION_CODE = INPUT_FUNCTION_CODE;
	OUTPUT_BYTE_COUNT = DATA_SIZE * 2;

	unsigned char OUTPUT_DATA[DATA_SIZE * 2];
	ShmId = shmget((key_t)mem, sizeof(shared_data), IPC_CREAT | 0666);
    ShmSendBuffer = (shared_data*)shmat(ShmId, NULL, 0);

	pthread_mutex_lock(&ShmSendBuffer->mutex);
	for (int i = 0; i < DATA_SIZE; i++) 
	{	
		OUTPUT_DATA[i*2] = (ShmSendBuffer->data[i + START_ADDRESS] >> 8) & 0xFF;	
		OUTPUT_DATA[i*2+1] = ShmSendBuffer->data[i + START_ADDRESS] & 0xFF;
	}
	pthread_mutex_unlock(&ShmSendBuffer->mutex);
	
	if(FindError() == 1)
	{
		s_buff[0] = OUTPUT_TID_1;
		s_buff[1] = OUTPUT_TID_2;
		s_buff[2] = OUTPUT_PID_1;
		s_buff[3] = OUTPUT_PID_2;
		s_buff[4] = 0x00;
		s_buff[5] = 0x03;
		s_buff[6] = OUTPUT_UID;
		s_buff[7] = EXCEPTION_CODE;
		s_buff[8] = EXCEPTION_CODE | 0x80;
		return;
	}
	else
	{
		s_buff[0] = OUTPUT_TID_1;
		s_buff[1] = OUTPUT_TID_2;
		s_buff[2] = OUTPUT_PID_1;
		s_buff[3] = OUTPUT_PID_2;
		s_buff[4] = OUTPUT_LENGTH_1;
		s_buff[5] = OUTPUT_LENGTH_2;
		s_buff[6] = OUTPUT_UID;
		s_buff[7] = OUTPUT_FUNCTION_CODE;
		s_buff[8] = OUTPUT_BYTE_COUNT;

		for(int i=0; i<DATA_SIZE*2; i++)
			s_buff[i+9] = OUTPUT_DATA[i];

		return;
	}
}
int Modbus_TCP_Class::isModbus(unsigned char* input)
{
	if (input[2] == 0x00 && input[3] == 0x00) {
		if (_2_HEX_TO_1_HEX(input[4], input[5]) == (READ_SIZE - 6) && input[7] <= 0x17) {
			return 0;
		} 
		else {
			return 1;
		}
	} 
	else {
		return 1;
	}
}
int Modbus_TCP_Class::FindError()
{
	int ERROR_OCCURED = 1;
	for(int i = 0; i < sizeof(SUPPORTED_FUNCTION)/sizeof(unsigned char); i++)
	{
		if(INPUT_FUNCTION_CODE == SUPPORTED_FUNCTION[i])
			ERROR_OCCURED = 0;
			break;
	}
	if(ERROR_OCCURED == 1)
	{
		EXCEPTION_CODE = 0x01;
		return ERROR_OCCURED;
	}

	else if(DATA_SIZE < 0x0001 || DATA_SIZE > 0x000A)
	{
		ERROR_OCCURED = 1;
		EXCEPTION_CODE = 0x03;
		return ERROR_OCCURED;
	}
	else if(START_ADDRESS < SUPPORTED_ADDRESS[0] || START_ADDRESS+DATA_SIZE > SUPPORTED_ADDRESS[1])
	{
		ERROR_OCCURED = 1;
		EXCEPTION_CODE = 0x02;
		return ERROR_OCCURED;
	}
	else
	{
		ERROR_OCCURED = 0;
		return ERROR_OCCURED;
	}
}
