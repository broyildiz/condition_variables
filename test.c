#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <string.h>

#include "prodcons.h"

ITEM buffer[BUFFER_SIZE];
static pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;

int tel;


/* Print the contents of the buffer*/
static void printBuf(void) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");
}



static void *producer(void *arg)
{
	pid_t tid = syscall(SYS_gettid);
	

	pthread_mutex_lock(&mx);
	printf("Producer Thread ID: %d, tel++ = %d\n", tid, ++tel);
	// tel++;

	pthread_mutex_unlock(&mx);

	return arg;
}



static void *consumer(void *arg)
{
	pid_t tid = syscall(SYS_gettid);
	printf("Consumer Thread ID: %d\n", tid);

	pthread_mutex_lock(&mx);

	printf("tel = %d\n", tel);

	pthread_mutex_unlock(&mx);

	return arg;
}


int main(void)
{
	memset(buffer, -1, sizeof(buffer));
	printBuf();

	tel = 0;

	pthread_mutex_lock(&mx);
	
	pthread_t threads[NROF_PRODUCERS];

	int i; 
	for(i = 0; i < NROF_PRODUCERS; i++)
	{
		pthread_create(&threads[i], NULL, producer, NULL);
	}

	pthread_mutex_unlock(&mx);

	for(i = 0; i < NROF_PRODUCERS; i++)
	{
		pthread_join(threads[i], NULL);
	}

		
	pthread_t consumer_thread;
	pthread_create(&consumer_thread, NULL, consumer, NULL);

	pthread_join(consumer_thread, NULL);

	return 0;
}


