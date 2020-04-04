
/* Includes */
#include <stdbool.h>
#include <pthread.h>
#include "linkedlist.h"
#include "structs.h"
#include <unistd.h>

/* Forward declarations */
void *request();
void *lift();

/* Global variables */
int bufferCount = 0;
int bufferLimit;
/* t = time take to travel in lift */
int t; 
LinkedList* buffer;
pthread_mutex_t mutex; 
pthread_cond_t cond;

void** retval;

int main(int argc, char* argv[])
{
    bool error = false;

    if(argc == 3)
    {
        buffer = createLinkedList();
        bufferLimit = atoi(argv[1]);
        t = atoi(argv[2]);

        /* create the mutex lock */
        pthread_mutex_init(&mutex, NULL);

        pthread_t liftR, lift1;
        pthread_attr_t attr;

        /* get the default attributes */
        pthread_attr_init(&attr);

        /* creates the Lift thread */ 
        pthread_create(&liftR, &attr, request, NULL);
        printf("Lift Request thread created\n");

        pthread_create(&lift1, &attr, lift, NULL);
        printf("Lift 1 thread created\n");

        /* Have to check if these work correctly and print out errors */
        pthread_join(liftR, retval);
        pthread_join(lift1, retval);
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }  
    else
    {
        printf("ERROR: Invalid amount of arguments, exiting..\n");
    }   
    return error;
}

void *request()
{

    FILE* f;
    int nRead, origFloor, destFloor;
    bool error;
    FloorReq* floor;
    
    error = false; 
    
    /* aquires mutex lock so it can enter critical section (the buffer) */
    pthread_mutex_lock(&mutex);

    while(bufferCount > bufferLimit)
    {
        pthread_cond_wait(&cond, &mutex);
    }

    f = fopen("sim_input", "r");
    if(f != NULL)
    {
        do
        {
            nRead = fscanf(f, "%d %d\n", &origFloor, &destFloor);
            if(nRead == 2)
            {
                floor = (FloorReq*)malloc(sizeof(FloorReq));
                floor -> origFloor = origFloor;
                floor -> destFloor = destFloor;

                /* If buffer is full, wait until its free */
                while(bufferCount >= bufferLimit)
                {
                    printf("request waiting...\n");
                    pthread_cond_wait(&cond, &mutex);
                    printf("request resumed\n");
                }

                insertLast(floor, buffer);
                bufferCount++;
                printf("%d\n", bufferCount);
            }
            else
            {
                printf("%d\n", nRead);
                printf("ERROR: File is in wrong format\n");
                error = true;
            }
        } while(!feof(f) && !error);
    }


    /* releases the mutex lock */
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void *lift()
{
    FloorReq* req;
    while(true)
    {
        /* aquires the mutext lock */
        pthread_mutex_lock(&mutex);

        /* Waits if the buffer is empty */
        while(bufferCount <= 0)
        {
            pthread_cond_wait(&cond, &mutex);
        }

        bufferCount--;
        printf("%d\n", bufferCount);

        req = (FloorReq*)removeFirst(buffer);

        printf("Moving lift from floor %d to %d\n", req -> origFloor, req -> destFloor);
        sleep(t);


        /* Signals the Wait in request() (probably can use signal because all threads are the same*/
        pthread_cond_broadcast(&cond);
        /* releases the mutex lock to allow request() to use it again */
        pthread_mutex_unlock(&mutex); 
    }
    return NULL;
}

