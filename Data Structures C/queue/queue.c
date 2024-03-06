/*
 * Queue implementation.
 *
 * - Implement each of the functions to create a working circular queue.
 * - Do not change any of the structs
 * - When submitting, You should not have any 'printf' statements in your queue 
 *   functions. 
 */
#include <assert.h>
#include <stdlib.h>

#include "queue.h"

/** The main data structure for the queue. */
struct queue{
  unsigned int back;      /* The next free position in the queue
                           * (i.e. the end or tail of the line) */
  unsigned int front;     /* Current 'head' of the queue
                           * (i.e. the front or head of the line) */
  unsigned int size;      /* How many total elements we currently have enqueued. */
  unsigned int capacity;  /* Maximum number of items the queue can hold */
  long *data;             /* The data our queue holds  */
};

/** 
 * Construct a new empty queue.
 *
 * Returns a pointer to a newly created queue.
 * Return NULL on error
 */
queue_t *queue_new(unsigned int capacity) {

  /* [TODO] Complete the function */
  if (capacity == 0) {  //make sure cap /= 0
    return NULL;
  }

  queue_t *q = (struct queue *) malloc(sizeof(queue_t));  //allocate mem for queue(new)
  if (q == NULL) {  //check mem allocated
    return NULL;
  }

  //init
  q->data = (long *)malloc(sizeof(long) *capacity);  //allocate memory for data
  if (q->data == NULL) { //check if memory allocated fail
    free(q); //free mem allocated
    return NULL;
  }

  q->back = 0;
  q->front = 0;
  q->capacity =  capacity;
  q->size = 0;

  return q;
}

/**
 * Check if the given queue is empty.
 *
 * Returns a non-0 value if the queue is empty, 0 otherwise.
 */
int queue_empty(queue_t *q) {
  assert(q != NULL);

  /* [TODO] Complete the function */
  if (q->size == 0) {
    return 1; //if empty return 1
  } else {
    return 0;
  }
}

/**
 * Check if the given queue is full.
 *
 * Returns a non-0 value if the queue is full, 0 otherwise.
 */
int queue_full(queue_t *q) {
  assert(q != NULL);

  /* [TODO] Complete the function */
  if (q->capacity == q->size) {
    return 1; //if size reached max cap, return 1
  } else {
    return 0;
  }
}

/**
 * Enqueue a new item.
 *
 * Push a new item into our data structure.
 */
void queue_enqueue(queue_t *q, long item) {
  assert(q != NULL);
  assert(q->size < q->capacity);

  /* [TODO] Complete the function */
  if (queue_full(q)) {
    return;
  }

  q->data[q->back] = item;  //add item to back
  q->back = (q->back + 1) % q->capacity;  //update back
  q->size++; //update size
}

/**
 * Dequeue an item.
 *
 * Returns the item at the front of the queue and removes an item from the 
 * queue.
 *
 * Note: Removing from an empty queue is an undefined behavior (i.e., it could 
 * crash the program)
 */
long queue_dequeue(queue_t *q) {
  assert(q != NULL);
  assert(q->size > 0);

  /* [TODO] Complete the function */
  long item = q->data[q->front]; //get first item from queue

  q->front = (q->front + 1) % q->capacity;  //update first item 

  q->size--;  //update size
  return item;
}

/** 
 * Queue size.
 *
 * Queries the current size of a queue (valid size must be >= 0).
 */
unsigned int queue_size(queue_t *q) {
  assert(q != NULL);

  /* [TODO] Complete the function */
  return q->size; 
}

/** 
 * Delete queue.
 * 
 * Remove the queue and all of its elements from memory.
 *
 * Note: This should be called before the proram terminates.
 */
void queue_delete(queue_t* q) {
  assert(q != NULL);

  /* [TODO] Complete the function */
  free(q->data); //free mem allocated for data
  free(q);  //free mem allocated for queue
}

