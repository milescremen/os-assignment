#ifndef LIFTS_H
#define LIFTS_H


/* Includes */
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#include "linkedlist.h"

/* Struct definitions */
typedef struct 
{
    int origFloor;
    int destFloor;
} FloorReq;


typedef struct
{
    char* name;
    int prevFloor;
    int reqMovement; 
    int reqNum;
    int totalMovement;
    int currFloor;
} Lift;

/* Forward declarations */
void *request();
void *lift();
void outputLiftLogs(Lift* lift, FloorReq* req);
Lift* createLiftStruct(char* name);
void outputRequestLogs(FloorReq* req, int requestNum);
void updateLiftValues(Lift* lift, FloorReq* req);
#endif
