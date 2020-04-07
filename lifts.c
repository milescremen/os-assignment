#include "lifts.h"

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

        Lift *liftPtr1, *liftPtr2, *liftPtr3;
        liftPtr1 = createLiftStruct("Lift 1");
        liftPtr2 = createLiftStruct("Lift 2");
        liftPtr3 = createLiftStruct("Lift 3");



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
    FloorReq* floor;
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
                requestCount++;
                printf("added: %d\n", bufferCount);
                outputRequestLogs(floor, requestCount);

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
void *lift(Lift* liftPtr)
{
    FloorReq* req;
    bool localFinishedConsumer = false;

    /* Loops until the producer thread ends and the buffer is empty */
    while(!localFinishedConsumer)
    {
        /* Aquire the mutex lock */
        pthread_mutex_lock(&mutex);

        /* If buffer is empty, put thread on cond wait */
        while(bufferCount == 0 && !globalFinishedConsumer)
        {
            //--printf("%lu entered wait\n", pthread_self());
            pthread_cond_wait(&cond, &mutex);
            //--printf("%lu exited wait\n", pthread_self());
            //--printf("BC: %d\n", bufferCount);
            //--printf("%s\n", localFinishedConsumer ? "true" : "false");
        }

        /* CRITICAL SECTION */

        /* Protects from segfaults when exiting */
        if(bufferCount != 0)
        {
            /* Removes a request from the buffer */
            req = (FloorReq*) removeFirst(buffer);
            bufferCount--;

            outputLiftLogs(liftPtr, req);

            printf("Moving %lu from floor %d to %d --- request %d\n", pthread_self(), req -> origFloor, req -> destFloor, bufferCount);
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
    printf("%lu broke out of loop\n", pthread_self());


    printf("%s broke out of loop\n", liftPtr->name);

    pthread_exit(NULL);
}

void outputLiftLogs(Lift* lift, FloorReq* req)
{
    FILE* f; 
    int prevFloor, reqMovement;
    prevFloor = lift->currFloor;
    lift->currFloor = req->destFloor;
    reqMovement = abs(prevFloor - req->origFloor) + abs(req->origFloor - req->destFloor);
    lift->totalMovement += reqMovement;
    lift->reqNum += 1;

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
        "\trequest: %d\n"
        "\tTotal #movement: %d\n"
        "Current position: Floor %d\n\n",
        lift->name, 
        prevFloor,
        req->origFloor, req->destFloor,
        prevFloor, req->origFloor,
        req->origFloor, req->destFloor,
        reqMovement,
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
    liftPtr->currFloor = 0;
    liftPtr->reqNum = 0;
    liftPtr->totalMovement = 0;

    return liftPtr;
}





