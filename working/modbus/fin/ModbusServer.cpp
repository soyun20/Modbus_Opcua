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
#include "Modbus_TCP.h"

#define MBAP_SIZE 7	
#define READ_SIZE 12

void (*sig)(int);
int port_num = 3000;
int mem = 1234;
int ret;
int sock;
int keep_going = 1;

int start_server();
void *server_thread(void *sock);
void cleanup_handler(void *arg);
void close_server(int signo);
void my_getopt(int argc, char* argv[]);

void help();
void Print_Hexa_Buff(unsigned char* s_buff, int len);
int _2_HEX_TO_INT(unsigned char HEX1, unsigned char HEX2);

int main(int argc, char** argv)
{
	my_getopt(argc, argv);
	start_server();
	return 0;
}

// server start
int start_server()
{
	// input SIGINT -> close server, SIGPIPE -> ignore
    sig = signal(SIGINT, close_server);
    signal(SIGPIPE, SIG_IGN);

	// socket create
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		printf("errno is %d", errno);
		return -1;
	}

	// set socket address
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_num);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// socket bind, listen
	if (-1 == bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)))
	{
		close(sock);
		return -1;
	}

	if (-1 == listen(sock, 5))
	{
		printf("listen errno is %d", errno);
		return -1;
	}

	printf("accept Stand by...\n");

	// socket accept and create thread
	while (keep_going == 1)
	{
		struct sockaddr_in client_addr = { 0, };
		socklen_t client_addr_size = sizeof(client_addr);
		int client_socket = accept(sock, (struct sockaddr*)&client_addr, &client_addr_size);
		if (client_socket < 0)
		{
			printf("accept errno is %d", errno);
			return -1;
		}
		
		pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, server_thread, (void*)&client_socket) < 0) {
            perror("pthread_create");
        }
    }
	return 0;
}

// ctrl+c -> socket close -> exit
void close_server(int signo)
{
    //sig = signal(signo, SIG_IGN);
	close(sock);

	printf("\nSocket Closed...\n");
	keep_going = 0;
}


void my_getopt(int argc, char* argv[])
{
	int c;
	while (1)
	{
		static struct option long_options[] =
		{
		  {"help",   no_argument,       0, 'h'},
		  {"port",  required_argument,  0, 'p'},
		  {"mem",  required_argument,  0, 'm'},
		  {0, 0, 0, 0}
		};
		
		int option_index = 0;

		c = getopt_long(argc, argv, "hp:m:",
			long_options, &option_index);

		if (c == -1)
			break;

		switch (c)
		{
		case 0:
			break;
		case 'h':
			help();
			break;
		case 'p':
			port_num = atoi(optarg);
			break;
		case 'm':
			mem = atoi(optarg);
			break;
		default:
			break;
		}
	}
}

void cleanup_handler(void *arg)
{			
	int client_sock = *(int *)arg;
	printf("Socket number %d has been Closed...\n", client_sock);
	close(client_sock);
}

// thread
void *server_thread(void *sock)
{
	int sock_thread = *(int*)sock;
	printf("New Client Input Complete, Socket Number: %d\n", sock_thread);
	
	Modbus_TCP_Class Modbus_TCP = Modbus_TCP_Class();
	Modbus_TCP.mem = mem;
	unsigned char r_buff[] = {0,};
	int SEND_SIZE;

	pthread_cleanup_push(cleanup_handler, &sock_thread);
	while(keep_going == 1)
	{
		ret = recv(sock_thread, r_buff, READ_SIZE, 0);
		if(ret < 1 || Modbus_TCP.isModbus(r_buff) == 1)
			break;

		//Print_Hexa_Buff(r_buff, READ_SIZE);
		Modbus_TCP.Set_Input_Output(r_buff);
		if(Modbus_TCP.FindError() == 1)
			SEND_SIZE = 9;
		else
			SEND_SIZE = Modbus_TCP.DATA_SIZE * 2 + 9;
			
		unsigned char s_buff[SEND_SIZE] = {0, };
		memcpy(s_buff, Modbus_TCP.s_buff, SEND_SIZE);

		ret = send(sock_thread, s_buff, SEND_SIZE, 0);
		//Print_Hexa_Buff(s_buff, SEND_SIZE);
	}
	pthread_cleanup_pop(1);
	pthread_exit(NULL);
}

// getopt --help or -h
void help()
{
	printf("--port / -p\t:input port number\n--mem / -m\t:input shared memory address\n");
}

// print hexadecimal array
void Print_Hexa_Buff(unsigned char* s_buff, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		printf("%02X ", s_buff[i]);
	}
	printf("\n");
}

