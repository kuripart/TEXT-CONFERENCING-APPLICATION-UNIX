#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "declarations.h"
#include "communication_handler.h"


void wait(){
  int i = 0;
  while(i<10){
    i++;
  }
}



// MESSAGE MANAGEMENT ------------------------------------------------

void delete_message_queue(message_queue *queue){
  free(queue->data);
  queue->data = NULL;
  wait();
}

int prepare_message(char *sender, char *data, message_data *message){
  sprintf(message->sender, "%s", sender);
  sprintf(message->data, "%s", data);
  wait();
  return 0;
}


int prepare_message_complete(char *sender, char *session_name, char *data, message_data *message){
  sprintf(message->sender, "%s", sender);
  sprintf(message->session_name, "%s", session_name);
  sprintf(message->data, "%s", data);
  wait();
  return 0;
}

 
int print_message(message_data *message){
  printf("Message: \"%s: %s\"\n", message->sender, message->data);
  wait();
  return 0;
}

// queue message
int create_message_queue(int queue_size, message_queue *queue){
  queue->data = calloc(queue_size, sizeof(message_data));
  queue->size = queue_size;
  queue->current = 0;
  wait();
  return 0;
}

int enqueue_message(message_queue *queue, message_data *message){
  if (queue->current == queue->size)
    return -1;
  else{
    memcpy(&queue->data[queue->current], message, sizeof(message_data));
    queue->current++;
    wait();
    return 0;
  }
}

int dequeue_message(message_queue *queue, message_data *message){
  if (queue->current == 0)
    return -1;
  else{
    memcpy(message, &queue->data[queue->current - 1], sizeof(message_data));
    queue->current--;
    wait();
  return 0;
  }
}

int dequeue_message_all(message_queue *queue){
  queue->current = 0;
  wait();
  return 0;
}


// -----------------------------------------------------------------------


#endif /* MESSAGE_HANDLER */


 
