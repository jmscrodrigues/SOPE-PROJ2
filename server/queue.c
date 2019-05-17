#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "types.h"

// A structure to represent a queue
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    struct tlv_request* array;
};

// function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;  // This is important, see the enqueue
    queue->array = malloc(queue->capacity * sizeof(struct tlv_request));
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue* queue)
{  return (queue->size == queue->capacity);  }

// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{  return (queue->size == 0); }

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, struct tlv_request item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)%queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
  //  printf("%d enqueued to queue\n", item);
}

// Function to remove an item from queue.
// It changes front and size
struct tlv_request dequeue(struct Queue* queue)
{
    struct tlv_request item = queue->array[queue->front];
    queue->front = (queue->front + 1)%queue->capacity;
    queue->size = queue->size - 1;

    return item;
}

// Function to get front of queue
struct tlv_request  front(struct Queue* queue)
{
      struct tlv_request  item_ret;
      item_ret = queue->array[queue->front];
      return item_ret;
    
}

// Function to get rear of queue
struct tlv_request rear(struct Queue* queue)
{

      struct tlv_request  item_ret;
      item_ret = queue->array[queue->rear];
      return item_ret;


}
