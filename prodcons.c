/* 
 * Operating Systems <2INCO> Practical Assignment
 * Condition Variables Application
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 *
 * Grading:
 * Students who hand in clean code that fully satisfies the minimum requirements will get an 8. 
 * Extra steps can lead to higher marks because we want students to take the initiative.
 */
 
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "prodcons.h"

static ITEM buffer[BUFFER_SIZE];

static void rsleep (int t);			// already implemented (see below)
static ITEM get_next_item (void);	// already implemented (see below)

static pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv  = PTHREAD_COND_INITIALIZER;

// helper
int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

/*
 * stuff for circular buffer
 */
static int read_idx = 0;
static int write_idx = 0;
static int nrof_items_buf = 0;

static bool isEmpty(void) {
    return nrof_items_buf == 0;
}

static bool isFull(void) {
    return nrof_items_buf == BUFFER_SIZE;
}

static ITEM pop(void) {
    if (isEmpty()) {
        printf("pop from empty buffer is illegal!\n");
        return -1;
    }

    nrof_items_buf--;
    return buffer[read_idx++ % BUFFER_SIZE];
}

static ITEM peek(void) {
    return buffer[mod(write_idx - 1, BUFFER_SIZE)];
    /* return buffer[(write_idx - 1) % BUFFER_SIZE]; */
}

static void push(ITEM item) {
    if (isFull()) {
        printf("push to full buffer is illegal!\n");
        return;
    }

    nrof_items_buf++;
    buffer[write_idx++ % BUFFER_SIZE] = item;
}

static void printBuf(void) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");
}

/* producer thread */
static void *
producer (void * arg)
{
    while (true /* TODO: not all items produced */)
    {
        // TODO:
        // * get the new item
        /* ITEM new_item = get_next_item(); */
        static int new_item_ctr = 0;
        ITEM new_item = new_item_ctr++;

        rsleep (100);	// simulating all kind of activities...

		// TODO:
		// * put the item into buffer[]
		//
        // follow this pseudocode (according to the ConditionSynchronization lecture):
        //      mutex-lock;
        //      while not condition-for-this-producer
        //          wait-cv;
        //      critical-section;
        //      possible-cv-signals;
        //      mutex-unlock;
        //
        // (see condition_test() in condition_basics.c how to use condition variables)
        //
        pthread_mutex_lock(&mx);
        printf("new_item %d\n", new_item);
        printf("peek %d\n", peek());
        while (isFull() || new_item != peek() + 1) {
            printf("producer waiting\n");
            pthread_cond_wait(&cv, &mx);
        }
        printf("push %d\n", new_item);
        push(new_item);
        // TODO: handle error?
        pthread_cond_signal(&cv);
        pthread_mutex_unlock(&mx);


        if (new_item == NROF_ITEMS) break;
    }

    return (NULL);
}

/* consumer thread */
static void *
consumer (void * arg)
{
    while (true /* TODO: not all items retrieved from buffer[] */)
    {
        // TODO: 
		// * get the next item from buffer[]
		// * print the number to stdout
        //
        // follow this pseudocode (according to the ConditionSynchronization lecture):
        //      mutex-lock;
        //      while not condition-for-this-consumer
        //          wait-cv;
        //      critical-section;
        //      possible-cv-signals;
        //      mutex-unlock;
        pthread_mutex_lock(&mx);
        while (isEmpty()) {
            printf("consumer waiting\n");
            pthread_cond_wait(&cv, &mx);
        }
        ITEM item = pop();
        printf("pop %d\n", item);
        // TODO: handle error?
        pthread_cond_signal(&cv);
        pthread_mutex_unlock(&mx);


        if (item == NROF_ITEMS) break;

        rsleep (100);		// simulating all kind of activities...
    }

    return (NULL);
}

int main (void)
{
    // don't delete, this is important for the program to run successfully
    memset(buffer, -1, sizeof(buffer));
    printBuf();

    /* printBuf(); */
    /* push(1); */
    /* push(2); */
    /* push(3); */
    /* push(4); */
    /* push(5); */
    /* push(6); */
    /* pop(); */
    /* pop(); */
    /* pop(); */
    /* pop(); */
    /* pop(); */
    /* int p = pop(); */
    /* printf("%d\n", p); */
    /* printBuf(); */


    // TODO: 
    // * startup the producer threads and the consumer thread
    // * wait until all threads are finished  
    pthread_t consumer_thread;
    pthread_t producer_thread;

    pthread_create(&consumer_thread, NULL, consumer, NULL);
    pthread_create(&producer_thread, NULL, producer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    /* int i; // Generic counter */
    /*  */
    /* int buffer_length = 0;	// Keep track of the number of items in the buffer */
    /* int write_index = 0;	// Keep track of where we are in the buffer */
    /* int read_index = 0;		// Keep track of where to read */
    /*  */
    /* // Loop through the cirvular array, a bunch of times  */
    /* // This way we check if the circularity works */
    /* for(i=0; i < BUFFER_SIZE+20; i++) */
    /* { */
    /*     if(buffer_length == BUFFER_SIZE-1) // Buffer lenght starts at 0, so -1 is added */
    /*     { */
    /*         buffer_length = 0; // dont remember the last round essentially */
    /*         write_index = 0;   // Restart at the beginning of the buffer */
    /*     } */
    /*  */
    /*     buffer[write_index] = i; 	// Write something to the buffer */
    /*     write_index++;				// Keep track of where we need to be */
    /*     buffer_length++;			// A new item has been added, keep track of it */
    /* } */
    /*  */
    /* for(i = 0; i < BUFFER_SIZE; i++) // Print the contents of the buffer */
    /* { */
    /*     printf("%d\n", buffer[i]); */
    /* } */
    /*  */
    /* printf("\n"); */
    /*  */
    /* for(i = 0; i < NROF_ITEMS; i++) // Print the output of the get_next_item() function */
    /* { */
    /*     printf("%d\n", get_next_item()); */
    /* } */
    /*  */
    /* return (0); */
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
