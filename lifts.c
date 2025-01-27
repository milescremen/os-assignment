#include "lifts.h"

/**
 * bufferCount - the current amount of items in the buffer
 * bufferLimit - the max size of the buffer
 * t - the time that lifts take
 * finishedImport - TRUE if the request() method has finished importing from file to Buffer 
 */
Buffer* buffer;
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

        int bufferLimit;
        pthread_t liftR, lift1, lift2, lift3;
        pthread_attr_t attr;

        Lift *liftPtr1, *liftPtr2, *liftPtr3;
        liftPtr1 = createLiftStruct("Lift 1");
        liftPtr2 = createLiftStruct("Lift 2");
        liftPtr3 = createLiftStruct("Lift 3");
        
        bufferLimit = atoi(argv[1]);
        /* Buffer Creation */
        createBuffer(bufferLimit);

        /* Set global variables to command line arguments */ 
        // Most likely need to change from atoi for error checking
        t = atoi(argv[2]);

        /* Mutex setup - NULL for default values */
        pthread_mutex_init(&mutex, NULL);

        /* Thread creation */
        pthread_attr_init(&attr);
        pthread_create(&liftR, &attr, request, NULL);
        printf("Lift Request thread created\n");

        pthread_create(&lift1, &attr, lift, liftPtr1);
        printf("Lift 1 thread created\n");
       
        pthread_create(&lift2, &attr, lift, liftPtr2);
        printf("Lift 2 thread created\n");
      
        pthread_create(&lift3, &attr, lift, liftPtr3);
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
    int nRead, origFloor, destFloor, requestCount;
    FloorReq* floorReq;
    bool error = false;
    requestCount = 0;

    f = fopen("sim_input", "r");
    if(f != NULL)
    {
        do
        {
            /* Aquire the mutex lock */
            pthread_mutex_lock(&mutex);

            /* If buffer is full, put thread on cond wait */
            while(buffer->count >= buffer->size)
            {
                printf("request entered wait\n");
                pthread_cond_wait(&cond, &mutex);
                printf("request exited wait\n");
            }

            /* CRITICAL SECTION */
            nRead = fscanf(f, "%d %d\n", &origFloor, &destFloor);
            if(nRead == 2)
            {
                floorReq = (FloorReq*)malloc(sizeof(FloorReq));
                floorReq -> origFloor = origFloor;
                floorReq -> destFloor = destFloor;

                /* Insert floor struct into the buffer */ 
                enqueue(floorReq);
                requestCount++;
                printf("added: %d\n", buffer->count);
                outputRequestLogs(floorReq, requestCount);

                if(feof(f))
                {
                    finishedProducer = true;
                }
                printf("%d\n", requestCount);

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
void *lift(Lift* liftPtr)
{
    FloorReq* reqPtr;
    bool localFinishedConsumer = false;

    /* Loops until the producer thread ends and the buffer is empty */
    while(!localFinishedConsumer)
    {
        /* Aquire the mutex lock */
        pthread_mutex_lock(&mutex);

        /* If buffer is empty, put thread on cond wait */
        while(buffer->count == 0 && !globalFinishedConsumer)
        {
            //--printf("%lu entered wait\n", pthread_self());
            pthread_cond_wait(&cond, &mutex);
            //--printf("%lu exited wait\n", pthread_self());
            //--printf("BC: %d\n", bufferCount);
            //--printf("%s\n", localFinishedConsumer ? "true" : "false");
        }

        /* CRITICAL SECTION */

        /* Protects from segfaults when exiting */
        printf("buffercount: %d\n", buffer->count);
        if(buffer->count != 0)
        {
            /* Removes a request from the buffer */
            reqPtr = dequeue();
            buffer->count--;

            updateLiftValues(liftPtr, reqPtr);
            outputLiftLogs(liftPtr, reqPtr);
            printf("Moving %s from floor %d to %d --- request %d\n", liftPtr->name, reqPtr -> origFloor, reqPtr -> destFloor, buffer->count);
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

        if(globalFinishedConsumer || (finishedProducer && buffer->count == 0))
        {
            localFinishedConsumer = true;
            globalFinishedConsumer = true;
        }

        /* 
        if(globalFinishedConsumer)
        {
            localFinishedConsumer = true;
        }
        else if(finishedProducer && bufferCount == 0)
        {
            localFinishedConsumer = true;
            globalFinishedConsumer = true;
        }
        */

        /* CRITICAL SECTION END */
        /* Broadcast to other waiting threads */
        
        //--printf("%lu BROADCASTED\n", pthread_self());
        pthread_cond_broadcast(&cond);
        /* Release the mutex lock */
        pthread_mutex_unlock(&mutex);

        /* Sleeps thread to simulate the moving of the lift */
        sleep(t); 
    }
    printf("%s broke out of loop\n", liftPtr->name);

    pthread_exit(NULL);
}

void outputLiftLogs(Lift* lift, FloorReq* req)
{
    FILE* f; 

    f = fopen("sim_out", "a");

    if(f != NULL)
    {
        fprintf(f, 
        "%s Operation\n"
        "Previous position: Floor %d\n"
        "Request: Floor %d to Floor %d\n"
        "Detail operations:\n"
        "\tGo from Floor %d to Floor %d\n"
        "\tGo from Floor %d to Floor %d\n"
        "\t#movement for this request: %d\n"
        "\t#request: %d\n"
        "\tTotal #movement: %d\n"
        "Current position: Floor %d\n\n",
        lift->name, 
        lift->prevFloor,
        req->origFloor, req->destFloor,
        lift->prevFloor, req->origFloor,
        req->origFloor, req->destFloor,
        lift->reqMovement,
        lift->reqNum,
        lift->totalMovement,
        lift->currFloor
        );

        fclose(f);
    }
}

void outputRequestLogs(FloorReq* req, int requestNum)
{
    FILE* f;

    f = fopen("sim_out", "a");
    
    if(f != NULL)
    {
        fprintf(f,
        "------------------------------------------\n"
        "New Lift Request From Floor %d to Floor %d\n"
        "Request No: %d\n"
        "------------------------------------------\n",
        req->origFloor, req->destFloor,
        requestNum);

        fclose(f);
    }
}


/**
 * createLift(char* name)
 * Imports a name, creates a new Lift struct 
 * Initalizes it and then returns a pointer to it.
 */
Lift* createLiftStruct(char* name)
{
    Lift* liftPtr;

    liftPtr = (Lift*)malloc(sizeof(Lift));

    liftPtr->name = name;

    liftPtr->prevFloor = 0;
    liftPtr->reqMovement = 0;
    liftPtr->reqNum = 0;
    liftPtr->totalMovement = 0;
    liftPtr->currFloor = 0;

    return liftPtr;
}

/**
 * Updates the lift values ready for output for logging 
 * 
 * 
 */
void updateLiftValues(Lift* lift, FloorReq* req)
{
    lift->prevFloor = lift->currFloor;
    lift->reqMovement = abs(lift->prevFloor - req->origFloor) + abs(req->origFloor - req->destFloor);
    lift->reqNum += 1;
    lift->totalMovement += lift->reqMovement;
    lift->currFloor = req->destFloor;
}





