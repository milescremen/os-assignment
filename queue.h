#ifndef QUEUE_H
#define QUEUE_H

#include "lifts.h"

extern Buffer* buffer;

/* Forward Declarations */
void createBuffer(int bufferSize);
void enqueue(FloorReq* floorReq);
FloorReq* dequeue();
bool bufferIsFull();
bool bufferIsEmpty();
#endif