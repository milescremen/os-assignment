
#ifndef LINKED_LIST_H
#define LINKED_LIST_H
/* Includes */
#include <stdlib.h>
#include <stdio.h>

/* Typedef's */
typedef struct ListNode {
    void* data;
    struct ListNode* next;
    struct ListNode* prev;
} Node;

typedef struct LinkedList {
    struct ListNode* head;
    struct ListNode* tail;
    int size;
} LinkedList;

/* Forward declaration */

LinkedList* createLinkedList();
void insertFirst(void* inData, LinkedList* list);
void insertLast(void* inData, LinkedList* list);
void* removeFirst(LinkedList* list);
void* removeLast(LinkedList* list);
void printLinkedList(LinkedList* list);
void freeLinkedList(LinkedList* list);

#endif