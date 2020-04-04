
#include "linkedlist.h"

/* Linked List */
LinkedList* createLinkedList()
{
    LinkedList* newList = (LinkedList*)malloc(sizeof(LinkedList));
    newList -> head = NULL;
    newList -> tail = NULL;
    newList -> size = 0;
    return newList;
}

void insertFirst(void* inData, LinkedList* list)
{
    /* Create a new node with imported data */
    Node* newNode = (Node*)malloc(sizeof(Node)); 
    Node* prevHead;
    newNode -> data = inData;

    if(list -> head == NULL) /* If list is empty */
    {
        list -> head = newNode;
        list -> tail = newNode;
        newNode -> next = NULL;
        newNode -> prev = NULL;
    }
    else
    {
        /* Sets the node's next value to the list's head value */
        newNode -> next = list -> head; 
        newNode -> prev = NULL;
        /* Stores the old head in prevHead (which we stored in newNode's next)*/
        prevHead = newNode -> next;
        prevHead -> prev = newNode;
        list -> head = newNode;
    }
    list -> size += 1;
}

void insertLast(void* inData, LinkedList* list)
{
    /* Create a new node with imported data */
    Node* newNode = (Node*)malloc(sizeof(Node));
    Node* prevTail;
    newNode -> data = inData;

    if(list -> head == NULL) /* If list is empty */
    {
        /* Sets lists head and tail to the created node */
        list -> head = newNode;
        list -> tail = newNode;
        newNode -> next = NULL;
        newNode -> prev = NULL;
    }
    else
    {
        /* Sets the node's prev value to the list's tail value */
        newNode -> prev = list -> tail;
        newNode -> next = NULL;
        /* Stores the old tail in prevTail (which we stored in newNode's prev)*/
        prevTail = newNode -> prev;
        prevTail -> next = newNode;
        list -> tail = newNode;
    }
    list -> size += 1;
}

void* removeFirst(LinkedList* list)
{
    Node* removedNode;
    Node* newHead;
    void* removedData;
    /* If list is empty then we return NULL */
    if(list -> head == NULL)
    {
        removedNode = NULL;
    }
    else
    {
        /* Stores the current head in removedNode*/
        removedNode = list -> head;
        /* Creates a new head variable and stores the removed heads next */
        newHead = removedNode -> next; 
        /* Updates the list's head */
        list -> head = newHead;
        /* Sets the prev value in our new head to NULL
            Unlinking the removed node */
        list -> head -> prev = NULL;
    }
    removedData = removedNode -> data;
    free(removedNode);
    return removedData;
}

void* removeLast(LinkedList* list)
{
    Node* removedNode;
    Node* newTail;
    void* removedData;
    /* If list is empty then we return NULL*/
    if(list -> head == NULL)
    {
        removedNode = NULL;
    }
    else
    {
        /* Stores the current tail in removedNode */ 
        removedNode = list -> tail;
        /* Creates a new tail variable and stores the removed tail's prev */
        newTail = removedNode -> prev;
        /* Updates the list's tail */
        list -> tail = newTail;
        /* Sets the next value in our new tail to NULL
            Unlinking the removed node */ 
        list -> tail -> next = NULL;
    }
    removedData = removedNode -> data;
    free(removedNode);
    return removedData;
}

void printLinkedList(LinkedList* list)
{   
    Node* currentNode;
    currentNode = list -> head;

    while(currentNode != NULL)
    {
        /* printf("%s\n", (char*)(currentNode -> data)); */
        /* Updates the currentNode to its next, traversing the list */
        (*list -> printFunc)(currentNode -> data);
        currentNode = currentNode -> next;
    }
}


void freeLinkedList(LinkedList* list)
{
    Node* currentNode;
    Node* nextNode;

    /* Starts the freeing process at the head of the list 
        Then grabs the next node in the list, storing it in nextNode 
        Then frees the current node, and sets the nextNode to the currentNode */
    currentNode = list -> head;
    while(currentNode != NULL)
    {
        nextNode = currentNode -> next;
        /*free(currentNode); */
    
        (*list -> freeFunc)(currentNode -> data);
        free(currentNode);
        currentNode = nextNode;
    }
    free(list);
}
