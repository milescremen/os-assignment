
#include "linkedlist.h"
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

/* Struct definitions */
typedef struct 
{
    int origFloor;
    int destFloor;
} FloorReq;

/* Forward declarations */
void *request();
void *lift();

/**
 * bufferCount - the current amount of items in the buffer
 * bufferLimit - the max size of the buffer
 * t - the time that lifts take
 * finishedImport - TRUE if the request() method has finished importing from file to Buffer 
 */
LinkedList* buffer;
int bufferCount;
int bufferLimit; 
int t; 

bool finishedProducer = false;
bool finishedConsume = false;
bool globalFinishedConsumer = false;

void** retval;
pthread_mutex_t mutex;
pthread_cond_t cond;

int main(int argc, char* argv[])
{
    if(argc == 3)
    {
        pthread_t liftR, lift1, lift2, lift3;
        pthread_attr_t attr;

        /* Buffer Creation */
        buffer = createLinkedList();

        /* Set global variables to command line arguments */ 
        // Most likely need to change from atoi for error checking
        bufferLimit = atoi(argv[1]);
        t = atoi(argv[2]);

        /* Mutex setup - NULL for default values */
        pthread_mutex_init(&mutex, NULL);

        /* Thread creation */
        pthread_attr_init(&attr);
        pthread_create(&liftR, &attr, request, NULL);
        printf("Lift Request thread created\n");

        pthread_create(&lift1, &attr, lift, NULL);
        printf("Lift 1 thread created\n");
       
        pthread_create(&lift2, &attr, lift, NULL);
        printf("Lift 2 thread created\n");
      
        pthread_create(&lift3, &attr, lift, NULL);
        printf("Lift 3 thread created\n");

        pthread_join(liftR, retval);
        printf("FINISHED lift request\n");
        pthread_join(lift1, retval);
        printf("FINISHED lift 1\n");
        pthread_join(lift2, retval);
        printf("FINISHED lift 2\n");
        pthread_join(lift3, retval);
        printf("FINISHED lift 3\n"); 

        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }
    else
    {
        printf("ERROR: Invalid amount of arguments, exiting...\n");
    }
    return 0;
}

/**
 * Producer thread
 */
void *request()
{
    FILE* f;
    int nRead, origFloor, destFloor;
    FloorReq* floor;
    bool error = false;

    f = fopen("sim_input", "r");
    if(f != NULL)
    {
        do
        {
            /* Aquire the mutex lock */
            pthread_mutex_lock(&mutex);

            /* If buffer is full, put thread on cond wait */
            while(bufferCount >= bufferLimit)
            {
                printf("request entered wait\n");
                pthread_cond_wait(&cond, &mutex);
                printf("request exited wait\n");
            }

            /* CRITICAL SECTION */
            nRead = fscanf(f, "%d %d\n", &origFloor, &destFloor);
            if(nRead == 2)
            {
                floor = (FloorReq*)malloc(sizeof(FloorReq));
                floor -> origFloor = origFloor;
                floor -> destFloor = destFloor;

                /* Insert floor struct into the buffer */ 
                insertLast(floor, buffer);
                bufferCount++;
                printf("added: %d\n", bufferCount);

                if(feof(f))
                {
                    finishedProducer = true;
                }

                /* CRITICAL SECTION END */
                /* Broadcast to other waiting threads */
                pthread_cond_broadcast(&cond);
                /* Release the mutex lock */
                pthread_mutex_unlock(&mutex);
            }
            else
            {
                printf("ERROR: File is in wrong format\n");
                error = true;
            }
        /* Loops until file end or an error occurs */
        } while(!feof(f) && !error);
    }
    return NULL;
}

/**
 * Consumer thread
 */
void *lift()
{
    FloorReq* request;
    bool localFinished = false;

    /* Loops until the producer thread ends and the buffer is empty */
    while(!localFinished)
    {
        /* Aquire the mutex lock */
        pthread_mutex_lock(&mutex);

        /* If buffer is empty, put thread on cond wait */
        while(bufferCount == 0 && !globalFinishedConsumer)
        {
            printf("%lu entered wait\n", pthread_self());
            pthread_cond_wait(&cond, &mutex);
            printf("%lu exited wait\n", pthread_self());
            printf("BC: %d\n", bufferCount);
            printf("%s\n", localFinished ? "true" : "false");
        }

        /* CRITICAL SECTION */


        if(bufferCount != 0)
        {
            /* Removes a request from the buffer */
            request = (FloorReq*) removeFirst(buffer);
            bufferCount--;

            printf("Moving %lu from floor %d to %d --- request %d\n", pthread_self(), request -> origFloor, request -> destFloor, bufferCount);
        }


        /**
         * Checks if another lift thread(consumer) has ended (and set globalFinishedConsume flag to true)
         * If not checks if the producer thread is finished and the buffer is empty
         * 
         * If these are true, will stop local thread with localFinished = true
         * and will notify other threads to finish with globalFinished
         * 
         * The reason for the different global / local flags, is to prevent race conditions of reading the global flag and setting the global flag
         */

        if(globalFinishedConsumer || (finishedProducer && bufferCount == 0))
        {
            localFinished = true;
            globalFinishedConsumer = true;
        }


        /* CRITICAL SECTION END */
        /* Broadcast to other waiting threads */
        
        printf("%lu BROADCASTED\n", pthread_self());
        pthread_cond_broadcast(&cond);
        /* Release the mutex lock */
        pthread_mutex_unlock(&mutex);

        /* Sleeps thread to simulate the moving of the lift */
        sleep(t); 
    }
    printf("%lu broke out of loop\n", pthread_self());

    pthread_exit(NULL);
}



