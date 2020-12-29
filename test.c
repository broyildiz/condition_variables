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

static void rsleep (int t);			// already implemented (see below)
static ITEM get_next_item (void);	// already implemented (see below)

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

	ITEM val = get_next_item();
	

	pthread_mutex_lock(&mx);
	printf("Producer Thread ID: %d, tel++ = %d\t\t val = %d\n", tid, ++tel, val);
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

/* To Do
* implement cicular buffer: use struct
* producers just add random items to the buffer, with mutex and stuff, not when full
* consumer consumes items from the buffer, not when empty
* only threds that have a number larger than the previous one in the buffer may continue
* only the thread that has the next value may produce
* producres whos number is more than one increment away dont look at the general call?
*/

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

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void 
rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time(NULL));
        first_call = false;
    }
    usleep (random () % t);
}


/* 
 * get_next_item()
 *
 * description:
 *		thread-safe function to get a next job to be executed
 *		subsequent calls of get_next_item() yields the values 0..NROF_ITEMS-1 
 *		in arbitrary order 
 *		return value NROF_ITEMS indicates that all jobs have already been given
 * 
 * parameters:
 *		none
 *
 * return value:
 *		0..NROF_ITEMS-1: job number to be executed
 *		NROF_ITEMS:		 ready
 */
static ITEM
get_next_item(void)
{
    static pthread_mutex_t	job_mutex	= PTHREAD_MUTEX_INITIALIZER;
	static bool 			jobs[NROF_ITEMS+1] = { false };	// keep track of issued jobs
	static int              counter = 0;    // seq.nr. of job to be handled
    ITEM 					found;          // item to be returned
	
	/* avoid deadlock: when all producers are busy but none has the next expected item for the consumer 
	 * so requirement for get_next_item: when giving the (i+n)'th item, make sure that item (i) is going to be handled (with n=nrof-producers)
	 */
	pthread_mutex_lock (&job_mutex);

    counter++;
	if (counter > NROF_ITEMS)
	{
	    // we're ready
	    found = NROF_ITEMS;
	}
	else
	{
	    if (counter < NROF_PRODUCERS)
	    {
	        // for the first n-1 items: any job can be given
	        // e.g. "random() % NROF_ITEMS", but here we bias the lower items
	        found = (random() % (2*NROF_PRODUCERS)) % NROF_ITEMS;
	    }
	    else
	    {
	        // deadlock-avoidance: item 'counter - NROF_PRODUCERS' must be given now
	        found = counter - NROF_PRODUCERS;
	        if (jobs[found] == true)
	        {
	            // already handled, find a random one, with a bias for lower items
	            found = (counter + (random() % NROF_PRODUCERS)) % NROF_ITEMS;
	        }    
	    }
	    
	    // check if 'found' is really an unhandled item; 
	    // if not: find another one
	    if (jobs[found] == true)
	    {
	        // already handled, do linear search for the oldest
	        found = 0;
	        while (jobs[found] == true)
            {
                found++;
            }
	    }
	}
    jobs[found] = true;
			
	pthread_mutex_unlock (&job_mutex);
	return (found);
}