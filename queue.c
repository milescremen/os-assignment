#include "queue.h"

/* Creates the buffer (Implemented as a circular queue with an array) */
void createBuffer(int bufferSize)
{
    buffer = (Buffer*)malloc(sizeof(Buffer));
    buffer->queue = (FloorReq*)malloc(sizeof(FloorReq) * bufferSize);
    buffer->size = bufferSize;
    buffer->head = -1;
    buffer->tail = -1;
    buffer->count = 0;
}

void enqueue(FloorReq* floorReq)
{
    if(bufferIsFull())
    {
        printf("ERROR\n");
    }
    else
    {
        if(buffer->head == -1)
        {
            buffer->head = 0;
            buffer->tail = 0;
        }
        else
        
        {
            if(buffer->tail == buffer->size - 1) 
            {
                buffer->tail = 0;
            }
            else
            {
                buffer->tail++;
            }
        }

        buffer->queue[buffer->tail] = *floorReq;
        buffer->count++;
        printf("Count: %d Head %d Tail: %d\n", buffer->count, buffer->head, buffer->tail);
    }
}


FloorReq* dequeue()
{
    FloorReq* value;
    value = NULL;

    if(bufferIsEmpty() || buffer->head == -1)
    {
        printf("QUEUE IS EMPTY\n");
    }
    else
    {
        value = &buffer->queue[buffer->head];
        printf("Count: %d Head %d Tail: %d\n", buffer->count, buffer->head, buffer->tail);
        if(buffer->head == buffer->tail)
        {
            buffer->head = -1;
            buffer->tail = -1;
        }
        else
        {
            buffer->head = (buffer->head + 1) % buffer->count;
        }
    }


    
    return value;
}

bool bufferIsFull()
{
    return (buffer->count >= buffer->size);
}

bool bufferIsEmpty()
{
    return (buffer->count == 0);
}