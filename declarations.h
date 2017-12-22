#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Maximum bytes that can be send() or recv() via net by one call.
 * It's a good idea to test sending one byte by one.
 */
#define MAX_SIZE_TO_SEND 100

/* Size of send queue (messages). */
#define BUFFER_MAX_MSG_SIZE 10

#define MAX_SENDER_PEER_SIZE 128
#define DATA_SIZE_MAX 512

#define STATION_SERVER_ADDRESS_IPV4 "127.0.0.1"
#define LISTENER_PORT 33235

#define MAX_FILES 1000
#define FILE_NAME_SIZE 256

/* Server side constants */

#define MAX_CLIENTS 5

#define SESSION_NAME_SIZE 256

#define NO_SOCKET -1 //default when no socket provided

#define SERVER_NAME "PRIME SERVER"

 #define BUF_SIZE 256

typedef struct {
  char sender[MAX_SENDER_PEER_SIZE];
  char data[DATA_SIZE_MAX];
  char session_name[SESSION_NAME_SIZE];
}  message_data;


typedef struct {

  int client_port;
  char *client_name;

} client_port_mapping;

typedef struct {

  client_port_mapping* client_port_mapping_list[MAX_CLIENTS];
  int mappings_used;

} client_port_data_struct;


char list_of_filenames[MAX_FILES][FILE_NAME_SIZE];



typedef struct {
  int size;
  message_data *data;
  int current;
} message_queue;


typedef struct {
  int socket;
  struct sockaddr_in address;


  char *name;

  
  /* The same for the receiving message. */
  message_data receiving_buffer;
  size_t current_receiving_byte;

  /* Messages that waiting for send. */
  message_queue send_buffer;
  
  /* Buffered sending message.
   */
  message_data sending_buffer;
  size_t current_sending_byte;
} peer_struct;


void init_conf_files(){

  for(int i = 0; i<MAX_FILES; i++){
    memset(list_of_filenames[i],0,FILE_NAME_SIZE);
  }
}


/**********  FILE MANAGEMENT *********/

//add specific file during create and join sessions
int add_file_name(char* file_name){
  int arr_index = -1;

  for (int i = 0; i < MAX_FILES; i++){
    if(strlen(list_of_filenames[i]) == 0){
      arr_index = i;
      break;
    }
  }
  
  if (arr_index != -1){
    strcpy(list_of_filenames[arr_index],file_name);
    return 0;
  }
  else{
    return -1;
  }

}

//remove specific file during leave session
int remove_file_name(char* file_name){
  int arr_index = -1;

  for (int i = 0; i < MAX_FILES; i++){
    if(strcmp(list_of_filenames[i], file_name) == 0){
      arr_index = i;
      break;
    }
  }
  
  if (arr_index != -1){
    memset(list_of_filenames[arr_index], 0, sizeof(list_of_filenames[arr_index]));
    return 0;
  }
  else{
    return -1;
  }

}


//remove all files when CTRL + C
void remove_all_files(){

  int ret;

  for (int i = 0; i < MAX_FILES; i++){
    if(strlen(list_of_filenames[i]) != 0){
      ret = remove(list_of_filenames[i]);
      if(ret == 0){
          printf("Session File %s deleted sucessfully\n", list_of_filenames[i]);
      }else{
          printf("Session File %s NOT deleted\n", list_of_filenames[i]);
      }
    }
  }
}

#endif /* DECLARATIONS_H */







