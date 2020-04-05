
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
int done = 0;
bool end = false;

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

        pthread_t liftR, lift1, lift2, lift3;
        pthread_attr_t attr;

        /* get the default attributes */
        pthread_attr_init(&attr);

        /* creates the Lift thread */ 
        pthread_create(&liftR, &attr, request, NULL);
        printf("Lift Request thread created\n");

        pthread_create(&lift1, &attr, lift, NULL);
        printf("Lift 1 thread created\n");

        pthread_create(&lift2, &attr, lift, NULL);
        printf("Lift 2 thread created\n");
        pthread_create(&lift3, &attr, lift, NULL);
        printf("Lift 3 thread created\n");
       
        /* Have to check if these work correctly and print out errors */
        pthread_join(liftR, retval);
        printf("FINISHED FINISHE DFINISHEHJAKFGHJAKEFLADJFALFHA\n");
        pthread_join(lift1, retval);
        pthread_join(lift2, retval);
        pthread_join(lift3, retval);
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

    f = fopen("sim_input", "r");
    if(f != NULL)
    {
        do
        {
            /* aquires mutex lock so it can enter critical section (the buffer) */
            pthread_mutex_lock(&mutex);

            while(bufferCount >= bufferLimit)
            {
                pthread_cond_wait(&cond, &mutex);
            }

            nRead = fscanf(f, "%d %d\n", &origFloor, &destFloor);
            if(nRead == 2)
            {
                floor = (FloorReq*)malloc(sizeof(FloorReq));
                floor -> origFloor = origFloor;
                floor -> destFloor = destFloor;

                /* If buffer is full, wait until its free */
                while(bufferCount + 1 > bufferLimit)
                {
                    printf("request waiting...\n");
                    pthread_cond_wait(&cond, &mutex);
                    printf("request resumed\n");
                }

                insertLast(floor, buffer);
                bufferCount++;
                printf("added: %d\n", bufferCount);
            }
            else
            {
                printf("%d\n", nRead);
                printf("ERROR: File is in wrong format\n");
                error = true;
            }

            done++;
            printf("Imported (%d/%d)\n", done, bufferLimit);

            /* Tells lifts that they can check their wait condition */ 
            pthread_cond_broadcast(&cond);
            /* releases the mutex lock */
            pthread_mutex_unlock(&mutex);

        } while(!feof(f) && !error);
        fclose(f);
        pthread_mutex_lock(&mutex);
        end = true;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *lift(pthread_t r)
{
    FloorReq* req;
    bool finished;
    finished = false;

    while(!finished)
    {
        /* aquires the mutext lock */
        pthread_mutex_lock(&mutex);

        if(end)
        {   
            break;
            finished = true;
        }
        /* Waits if the buffer is empty */
        while(bufferCount - 1 < 0)
        {
            pthread_cond_wait(&cond, &mutex);
        }


        printf("removing %d\n", bufferCount);
        bufferCount--;
        req = (FloorReq*)removeFirst(buffer);

        printf("Moving %lu from floor %d to %d --- request %d\n", pthread_self(), req -> origFloor, req -> destFloor, bufferCount);

        printf("Lift waiting : %lu\n", pthread_self());


        printf("%s\n", finished ? "true" : "false");

        /* Signals the Wait in request() (probably can use signal because all threads are the same*/
        pthread_cond_broadcast(&cond);
        /* releases the mutex lock to allow request() to use it again */
        pthread_mutex_unlock(&mutex); 

        sleep(t);
    }
    printf("THREAD EXITED \n");

    pthread_exit(NULL);
}

