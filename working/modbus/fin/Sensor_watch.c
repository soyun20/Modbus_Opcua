#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    pthread_mutex_t mutex;
    unsigned char data[10];
} shared_data;

int main() {
    int shm_id = shmget((key_t)1234, sizeof(shared_data), IPC_CREAT | 0666);
    shared_data* ShmSendBuffer = (shared_data*)shmat(shm_id, NULL, 0);
	while(1)
	{
		for(int i=0;i<10;i++)
		{
			pthread_mutex_lock(&ShmSendBuffer->mutex);
			printf("%x ", ShmSendBuffer->data[i]);  
			pthread_mutex_unlock(&ShmSendBuffer->mutex);
		}
		printf("\n\n");
		sleep(1);
	}
    
    shmctl(shm_id, IPC_RMID, NULL);
    
    return 0;
}
