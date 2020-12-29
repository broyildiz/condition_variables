#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <semaphore.h> 	// Mandatory for semaphores
#include <string.h>		// memset()
#include "prodcons.h"

static void rsleep (int t);			// already implemented (see below)
static ITEM get_next_item (void);	// already implemented (see below)

// ITEM buffer[BUFFER_SIZE];

typedef struct
{
	ITEM buffer[BUFFER_SIZE];	// Bufffer that the producers store numbers in
	sem_t occupied;	// Counting sempahore that keeps track of the number of items in the buffer
	sem_t empty;	// Counting semaphore that keeps track of the empty spots in the buffer
	int nextin;		// Write counter
	int nextout;	// Read counter
	sem_t pmut;		// Binary semaphore same as Mutex Lock and unlock
	sem_t cmut;		// Binary semaphore same as Mutex Lock and unlock

} buffer_t;

// Note: the pmut and cmut semaphores could (should?) be changed to actual mutexes
// I chose to follow the example I found
// Works essentially the same
// Links to the example are above the main

buffer_t buffer;

// static pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER; // Not used

/* Print the contents of the buffer*/
static void printBuf(void) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("%d ", buffer.buffer[i]);
    }
    printf("\n");
}


static void *producer(void *arg)
{
	// Get and print thread ID
	pid_t tid = syscall(SYS_gettid);
	printf("Producer: %d\n", tid);

	while(true)
	{
		rsleep(100); // Simulate doing some stuff

		ITEM next_item = get_next_item(); // Get random item
		printf("Producer: %d\titem = %d\n", tid, next_item);
		if(next_item == NROF_ITEMS)
		{
			printf("Producer: %d\tdone\n", tid);

			/* Not sure if this is the way we need to exit, perhaps dont do sem_post(&buffer.occupied); */

			// Give semaphore+// Same as Mutex Unlock
			// sem_post(&buffer.pmut);
			// Increment the occupied semaphore. This way producers know there is one less available place in the buffer
			// sem_post(&buffer.occupied);

			return arg;
		}

		// If there is a free place in the buffer
		sem_wait(&buffer.empty);

		// same as Mutex Lock
		sem_wait(&buffer.pmut);

		// Add item
		buffer.buffer[buffer.nextin++] = next_item;

		// Adjust becasue we have a circular buffer
		buffer.nextin %= BUFFER_SIZE;

		// Give semaphore+// Same as Mutex Unlock
		sem_post(&buffer.pmut);

		// Increment the occupied semaphore. This way producers know there is one less available place in the buffer
		sem_post(&buffer.occupied);

		// break;
	}

	return arg;
}



static void *consumer(void *arg)
{
	// Get and print the thread ID
	pid_t tid = syscall(SYS_gettid);
	printf("Consumer: %d\n", tid);

	// Store the received items to print them at the end
	ITEM check[NROF_ITEMS];
	int check_cnt = 0;
	memset(check, -1, sizeof(check));

	while(true)
	{
		// Wait for items to be consumer
		sem_wait(&buffer.occupied);
		// Same as Mutex Lock
		sem_wait(&buffer.cmut);

		// Get next item
		ITEM consume = buffer.buffer[buffer.nextout];
	    printf("Consumer: %d\tconsume: %d\n", tid, consume);

	    // THIS WILL NOT HAPPEN
	    // The producers will not send the value NROF_ITEMS
	    // This should be NROF_ITEMS -1
	    // To implement this, the values send to the consumer should be aranged from smallest to largest
	    // Otherwise the consumer will quit before the producers are done
	    if(consume == NROF_ITEMS)
	    {
			printf("Consumer: %d\tdone\n", tid);

			return arg;
	    }

	    // Store to print later
	    check[check_cnt++] = consume;

	    // Increment read counter
	    buffer.nextout++;
	    // Adjust becasue we have a circular buffer
	    buffer.nextout %= BUFFER_SIZE;

	    // If we technically reached the end of the program, print what we received
	    if(check_cnt == NROF_ITEMS)
	    {
	    	printf("\n");
			int i;
			for(i = 0; i < NROF_ITEMS; i++)
			{
				printf(" %d", check[i]);
			}
			printf("\n");

			// Uncomment the next two lines if you want the program to exit after it received all producer values
			sem_post(&buffer.cmut);
			break;
	    }

	    // Same as Mutex unlock
		sem_post(&buffer.cmut);
		// Decrement the empty sempahore.
		// This way the procuders know there is a place in the buffer to produce stuff into
		sem_post(&buffer.empty);

		rsleep(100);

		// break;
	}

	return arg;
}

/*
* Great help 1: https://docs.oracle.com/cd/E26502_01/html/E35303/sync-11157.html
* Great help 2: https://docs.oracle.com/cd/E26502_01/html/E35303/sync-11157.html#sync-27385
*/

int main(void)
{

	printf("Main PID = %d\n\n\n", getpid()); // Print the pid of the main thread


	memset(&buffer.buffer, -1, sizeof(buffer.buffer));
	printBuf();

	// int sem_init(sem_t *sem, int pshared, unsigned int value);
	// if pshared is 0, then the semaphore cannot be shared between processes
	// Otherwise is can be share
	// Init the value of the samaphore to 0, i.e.: no elements in the buffer
	sem_init(&buffer.occupied, 0, 0);

	// Init to BUFFER_SIZE becasue the whole buffer is empty
	sem_init(&buffer.empty,0, BUFFER_SIZE);

	// Init the mutexes, again: should be changed to actual mutexes
	sem_init(&buffer.pmut, 0, 1);
	sem_init(&buffer.cmut, 0, 1);

	// Init read and write counters
	buffer.nextout = 0;
	buffer.nextin = 0;

	int i;

	// Create consumer thread
	pthread_t consumer_thread;
	pthread_create(&consumer_thread, NULL, consumer, NULL);


	// pthread_mutex_lock(&mx);

	// Pool of producer threads
	// Used later to join them all back
	// I dont want the main to exit before all threads are finished
	pthread_t threads[NROF_PRODUCERS];

	// Create the producer threads
	for(i = 0; i < NROF_PRODUCERS; i++)
	{
		pthread_create(&threads[i], NULL, producer, NULL);
	}

	// pthread_mutex_unlock(&mx);

	// Wait for all producers to exit
	for(i = 0; i < NROF_PRODUCERS; i++)
	{
		pthread_join(threads[i], NULL);
	}

	// Wait for the consumer to finish
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
