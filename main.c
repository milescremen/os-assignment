


#include <stdbool.h>
#include <pthread.h>
#include "linkedlist.h"
#include "structs.h"

void *request(LinkedList* buffer);
int bufferCount = 0;
int bufferLimit;

int main(int argc, char* argv[])
{
    LinkedList* buffer;
    bool error; 
    if(argc == 2)
    {
        buffer = createLinkedList();
        bufferLimit = argv[1];
        pthread_t liftR, lift1, lift2, lift3;
        pthread_attr_t attr;

        /* get the default attributes */
        pthread_attr_init(&attr);
        /* create the thread */ 
        pthread_create(&liftR, &attr, request, buffer);
        pthread_create(&lift1, &attr, , buffer)
        pthread_create(&lift2, &attr, , buffer)
        pthread_create(&lift3, &attr, , buffer)


    }  
    else
    {
        printf("ERROR: Invalid amount of arguments, exiting..\n");
    }   
    return error;
}


void *request(LinkedList* buffer)
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
            nRead = fscanf(f, "%d %d\n", &origFloor, &destFloor);
            if(nRead == 2)
            {
                //Testing 
                printf("%d %d", origFloor, destFloor);
                floor = (FloorReq*)malloc(sizeof(FloorReq));
                floor -> origFloor = origFloor;
                floor -> destFloor = destFloor;
                if(bufferCount <= bufferLimit)
                {
                    insertLast(floor, buffer);
                    bufferCount++;
                }
            }
            else
            {
                printf("%d\n", nRead);
                printf("ERROR: File is in wrong format\n");
                error = true;
            }
        } while(!feof(f) && !error);
    }

    pthread_exit(NULL);
}