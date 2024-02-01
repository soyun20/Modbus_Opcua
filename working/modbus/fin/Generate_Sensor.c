#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

unsigned char Generate_Data(int average, int stdev, int seed);
void Start_Sensor(int mem);
void Close_Sensor(int signo);
void help();
void my_getopt(int argc, char* argv[]);

typedef struct {
    pthread_mutex_t mutex;
    unsigned char data[10];
} shared_data;

shared_data* ShmSendBuffer;
void (*sig)(int);
int mem = 1234, average = 50, stdev = 5, seed = 1234;
int keep_going = 1;
int shm_id;

int main(int argc, char *argv[])
{
	my_getopt(argc, argv);
    Start_Sensor(mem);
    return 0;
}

void Start_Sensor(int mem)
{
    sig = signal(SIGINT, Close_Sensor);

    shm_id = shmget((key_t)mem, sizeof(shared_data), IPC_CREAT | 0666);
    ShmSendBuffer = (shared_data*)shmat(shm_id, NULL, 0);
	printf("\nStarted Sensor ID: %d\nSensor Started\n", shm_id);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&ShmSendBuffer->mutex, &attr);

	while(keep_going == 1)
	{
		pthread_mutex_lock(&ShmSendBuffer->mutex);
		for(int i = 0; i < 10 && keep_going == 1; i++)
			ShmSendBuffer->data[i] = Generate_Data(average, stdev, seed);
		pthread_mutex_unlock(&ShmSendBuffer->mutex);

		sleep(1);
	}
	return;
}

// ctrl+c -> shared memory remove -> exit
void Close_Sensor(int signo)
{
    shmctl(shm_id, IPC_RMID, NULL);
    keep_going = 0;
	printf("\nStopped Sensor ID: %d\nSensor Stopped\n", shm_id);
	return;
}

void my_getopt(int argc, char* argv[])
{
	int c;
	while (1)
	{
		static struct option long_options[] =
		{
		  {"help",	no_argument,     	  0, 'h'},
		  {"mem",  	required_argument,	  0, 'm'},
		  {"average",   required_argument,       0, 'a'},
		  {"stdev",  	required_argument,       0, 'd'},
		  {"seed",  	required_argument,       0, 's'},
		  {0, 0, 0, 0}
		};
		
		int option_index = 0;

		c = getopt_long(argc, argv, "hm:a:d:s:", long_options, &option_index);

		if (c == -1)
			break;

		switch (c)
		{
		case 'h':
			help();
			exit(1);
		case 'm':
			mem = atoi(optarg);
			break;
		case 'a':
			average = atoi(optarg);
			break;
		case 'd':
			stdev = atoi(optarg);
			break;
		case 's':
			seed = atoi(optarg);
			break;
		default:
			break;
		}
	}
}

unsigned char Generate_Data(int average, int stdev, int seed)
{
	double v1, v2, s;
	do {
		v1 =  2 * ((double) rand() / RAND_MAX) - 1;
		v2 =  2 * ((double) rand() / RAND_MAX) - 1;
		s = v1 * v1 + v2 * v2;
	} while (s >= 1 || s == 0);
	s = sqrt((-2 * log(s)) / s);
    
	return (unsigned char)((stdev * v1 * (int)s) + average);
}
// getopt --help or -h
void help()
{
	printf("Usage : options\n");
	printf("--help / -h\tOption Guide\n--mem / -m\tSet memory address\n--average / -a\tSet Gaussian average\n--stdev / -d\tSet Gaussian standard deviation\n--seed / -s\tSet random seed\n");
}