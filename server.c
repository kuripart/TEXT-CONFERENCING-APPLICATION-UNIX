                
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

#include "message_handler.h"
#include "declarations.h"
#include "conference_handler.h"
#include "communication_handler.h"


fd_set read_fds;
fd_set write_fds;
fd_set exception_fds;

int listen_sock;
peer_struct connection_list[MAX_CLIENTS];
char read_buffer[1024]; // buffer for stdin
int high_sock;

int clients_connected;


// Function to hangle system calls due to CTRL + C
void handle_signal_action(int sig_number){
  if (sig_number == SIGINT) {
    remove_all_files();
    remove("session_file.txt");
  }
  else if (sig_number == SIGPIPE) {
    //not needed now
  }
}


// Function to signal the exception created from CTRL + C
int setup_signals(){
  struct sigaction sa;
  sa.sa_handler = handle_signal_action;
  if (sigaction(SIGPIPE, &sa, 0) != 0) {
    perror("sigaction()");
    return -1;
  }
  else if (sigaction(SIGINT, &sa, 0) != 0) {
    perror("sigaction()");
    return -1;
  }
  return 0;
}

// Function to handle incoming connections from the clients
/*
The function accepts the client connections and stores in client socket, 
address and id in a structure for individual clients, which is later used for 
sending/receiving messages from multiple clients

On reaching a MAX_CLIENT threshold, it sends out a message telling that
no more clients can be connected and closes the socket that currently tried 
to connect
*/

int handle_new_connection() {
  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(client_addr));
  socklen_t client_len = sizeof(client_addr);
  int client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_len);
  if (client_sock < 0) {
    perror("accept()");
    return -1;
  }
  
  char client_ipv4_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &client_addr.sin_addr, client_ipv4_str, INET_ADDRSTRLEN);
  
  printf("Incoming connection from %s:%d.\n", client_ipv4_str, client_addr.sin_port);
  

  // CHECK client_port_data's number of available clients ... 
  // if already 0 ... say TOO MANY connections and close socket 
  // otherwise, iterate through a loop to find the first available empty_connection
  // and add the connection data to that point. Also update client_port_data!

  if (clients_connected == MAX_CLIENTS){
      printf("Max Clients Exceeded: %s:%d.\n", client_ipv4_str, client_addr.sin_port);
      close(client_sock);
      return -1;
  }

  else{

      for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (connection_list[i].socket == NO_SOCKET) {
          char *line;
          size_t length = 0;
          FILE *client_name_temp_store_file= fopen("client_name_temp.txt", "r");
          getline(&line, &length, client_name_temp_store_file);
          char* client_name = line;
          fclose(client_name_temp_store_file);
          //Add info to the connection_list
          connection_list[i].socket = client_sock;
          connection_list[i].name = client_name;
          connection_list[i].address = client_addr;
          connection_list[i].current_sending_byte   = -1;
          connection_list[i].current_receiving_byte = 0;
          clients_connected++;
          printf("Client Added: %s\n", connection_list[i].name);
          return client_addr.sin_port;
        }
      }
  }
  return -1;
}

//Function to close client connection

int close_client_connection(peer_struct *client){
  printf("Close client socket for %s.\n", peer_get_addres_str(client));
  
  close(client->socket);
  client->socket = NO_SOCKET;
  dequeue_message_all(&client->send_buffer);
  client->current_sending_byte   = -1;
  client->current_receiving_byte = 0;
}

/* Reads from stdin and create new message. This message enqueues to send queue. */
int handle_read_from_stdin(){
  char read_buffer[DATA_SIZE_MAX]; // buffer for stdin
  if (read_from_stdin(read_buffer, DATA_SIZE_MAX) != 0)
    return -1;
  
  // Create new message and enqueue it.
  message_data new_message;
  prepare_message(SERVER_NAME, read_buffer, &new_message);
  print_message(&new_message);
  
  /* enqueue message for all clients */
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (connection_list[i].socket != NO_SOCKET) {
      if (peer_add_to_send(&connection_list[i], &new_message) != 0) {
        continue;
      }
    }
  }
  return 0;
}



// Function to deliver message to clients form other clients

int handle_msg_to_deliver_to_clients(char* message_from_client, char* client_name){

  message_data new_message;
  //prepare_message(client_name, message_from_client, &new_message);


  char client_current_session[BUF_SIZE];

  //Peek into that client's current session file
  find_current_session(client_name, client_current_session);

  char other_clients[MAX_CLIENTS][BUF_SIZE];

  init_clients_list(other_clients);

  //Given a session and the exclude-client, get a list of all other clients in that session

  int num_other_clients = acquire_fellow_clients(client_name,client_current_session,other_clients);


  // prepare the message to be sent with the client name and session name
  prepare_message_complete(client_name, client_current_session, message_from_client, &new_message);
  print_message(&new_message);

  /* enqueue message for clients */
  int i;

  //OUTER LOOP: ITERATE THROUGH EVERY LOGGED IN CLIENT ....
  for (i = 0; i < MAX_CLIENTS; ++i) {
    if (connection_list[i].socket != NO_SOCKET) {

      //CHECK IF LOGGED IN CLIENT EXISTS IN ONE OF THE OTHER CLIENTS ...
      for (int j = 0; j < num_other_clients; j++){
        //If some element in "other_clients" matches with the name associated the current "connection_list[i]""
        if (strcmp(connection_list[i].name,other_clients[j]) == 0){

          if (peer_add_to_send(&connection_list[i], &new_message) != 0) {
            continue;
          }
        }
      }
    }
  }
  return 0;
}


// Helper function to extract message from a request packet

void get_msg_field(char *message, int field_no, char *return_str){

    //field_no 1: message_type UNLESS default message??
    //field_no 2: session_id (for CREATE/JOIN/LEAVE sessions)

    int str_length = strlen(message);
    int semi_counter = 0;
    int buff_char_count = 0;
    char curr_char;

    for(int i = 0; i < str_length; i++){
        curr_char = message[i];

        if(curr_char == ':'){
            semi_counter++; 
            if(semi_counter ==  field_no){
                return;
            }else{
                memset(return_str, 0, strlen(return_str));
                buff_char_count = 0;
            }

        }else{
            return_str[buff_char_count] = message[i];
            buff_char_count++;
        }
    }
    memset(return_str, 0, sizeof(return_str));
}

int handle_received_message(message_data *message){

  char *client_name = message->sender;

  char msg_type[BUF_SIZE];
  memset(msg_type, 0, sizeof(msg_type)); //THIS IS IMPORTANT!!!
  get_msg_field(message->data,1,msg_type);

  if (strcmp(msg_type,"/create") == 0){

    char session_name[BUF_SIZE];
    memset(session_name,0,sizeof(session_name));

    get_msg_field(message->data,2,session_name);
    printf("Session name <create>: %s\n", session_name);
    create_new_conf_sess(session_name, client_name);
  }

  else if (strcmp(msg_type,"/join") == 0){

    char session_name[BUF_SIZE];
    memset(session_name,0,sizeof(session_name));
    get_msg_field(message->data,2,session_name);
    printf("Session name <join>: %s\n", session_name);
    join_session(session_name, client_name);
    //makes this session the current session
    transition_to_session(session_name, client_name);
  }

  else if (strcmp(msg_type, "/leave") == 0){
    
    char session_name[BUF_SIZE];
    memset(session_name,0,sizeof(session_name));
    get_msg_field(message->data,2,session_name);
    printf("Session name <leave>: %s\n", session_name);

    leave_session(session_name, client_name);
  }

  else if (strcmp(msg_type, "/swap") == 0){

    char session_name[BUF_SIZE];
    memset(session_name,0,sizeof(session_name));
    get_msg_field(message->data,2,session_name);
    printf("Session name <swap>: %s\n", session_name);

    swap_session(session_name, client_name);
  }

  else if(strcmp(msg_type, "/current") == 0){
    char current_session[BUF_SIZE];
    memset(current_session,0,sizeof(current_session));
    find_current_session(client_name, current_session);

    printf("Current session name <current>: %s\n", current_session);

  }

    else if(strcmp(msg_type, "/exit") == 0){
      for (int i = 0; i < MAX_CLIENTS; ++i){
        if(strcmp(connection_list[i].name,"client_name") == 0){
          printf("%s%s\n", "Closing connection of client: ", client_name);
          close(connection_list[i].socket);
          break;
        }
      }
  }


  //THIS REQUIRES BULLETPROOFING
  else{
    //Handle default messages (FOR NOW)
    // - HANDLE WHICH SESSION TO SEND MESSAGE TO
    /*printf("Received message from client.\n");

    //Check if file exists
    char *txt = "txt";
    char *temp_str_client = malloc(strlen(client_name)+strlen(txt)+1);//+1 for the null-terminator
    strcpy(temp_str_client, client_name);
    strcat(temp_str_client, txt);
    char out_line[BUF_SIZE];
    memset(out_line, 0, sizeof(out_line));
    FILE *client_file = fopen(temp_str_client, "r");*/


    //If file does exist
    //if(client_file!=NULL){
      //read_line_from_file(temp_str_client, out_line);
      //If file not empty....
      //if(strlen(out_line) != 0){
        print_message(message);
        // once a message is received from a client send it to all the avaialable clients.
        handle_msg_to_deliver_to_clients(message->data, client_name);
      //}
    //}else{
      //printf("You are not included in any conference sessions. Thus, message not delivered!");
    //}
  }

  return 0;
}

//MAIN STARTS//
 
int main(int argc, char **argv){
  /* Start listening socket listen_sock. */

  if (setup_signals() > 0)
    exit(EXIT_FAILURE);


  //Initialize number of clients connected to 0
  clients_connected = 0;
  //initialize_mapping_list();
  init_conf_files();


  // Obtain a file descriptor for our "listening" socket.
  listen_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sock < 0) {
    perror("socket");
    return -1;
  }
 
  int reuse = 1;
  if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0) {
    perror("setsockopt");
    return -1;
  }
  
  struct sockaddr_in my_addr;
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = inet_addr(STATION_SERVER_ADDRESS_IPV4);
  my_addr.sin_port = htons(LISTENER_PORT);
 
  if (bind(listen_sock, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) != 0) {
    perror("bind");
    return -1;
  }
 
  // start accept client connections
  if (listen(listen_sock, 10) != 0) {
    perror("listen");
    return -1;
  }
  printf("Accepting connections on port %d.\n", (int)LISTENER_PORT);
  
  /* Set nonblock for stdin. */
  int flag = fcntl(STDIN_FILENO, F_GETFL, 0); //manipulate file descriptor
  flag |= O_NONBLOCK;
  fcntl(STDIN_FILENO, F_SETFL, flag);
  
  //create the peers here --> the connected clients
  int i;
  for (i = 0; i < MAX_CLIENTS; ++i) {
    connection_list[i].socket = NO_SOCKET;
    create_peer(&connection_list[i]);
  }
  printf("Waiting for incoming connections.\n");

  /* Start waiting for client*/
  for(;;) {


      // intitialise by setting the file descriptors to zero
      FD_ZERO(&read_fds);
      FD_ZERO(&write_fds);
      FD_ZERO(&exception_fds);

      FD_SET(STDIN_FILENO, &read_fds);
      FD_SET(listen_sock, &read_fds);

      for (i = 0; i < MAX_CLIENTS; i++){
        if (connection_list[i].socket != NO_SOCKET){
          FD_SET(connection_list[i].socket, &read_fds);
        }
      }

      //NOTE: EXCEPTION FDs are set similar to READ
      for (int i = 0; i < MAX_CLIENTS; i++){
        if (connection_list[i].socket != NO_SOCKET && connection_list[i].send_buffer.current > 0){
          FD_SET(connection_list[i].socket, &write_fds);
        }
      }
      
      high_sock = listen_sock;
      for (i = 0; i < MAX_CLIENTS; ++i) {
        if (connection_list[i].socket > high_sock)
          high_sock = connection_list[i].socket;
      }

      //SELECT HAPPENS HERE ***
      
      int activity = select(high_sock + 1, &read_fds, &write_fds, &exception_fds, NULL);

      if(activity == -1){
          perror("select()");
          close(listen_sock);
          for (int i = 0; i < MAX_CLIENTS; i++){
            if (connection_list[i].socket != NO_SOCKET){
              close(connection_list[i].socket);
            }
          }
          printf("Shutdown server.\n");
          exit(EXIT_FAILURE);
      }else if(activity == 0){
          // you should never get here
          printf("select() returns 0.\n");
          close(listen_sock);
          for (int i = 0; i < MAX_CLIENTS; ++i){
            if (connection_list[i].socket != NO_SOCKET){
              close(connection_list[i].socket);
            }
          }
          printf("Shutdown server.\n");
          exit(EXIT_FAILURE);
      }else{
          /* All set fds should be checked. */
          if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (handle_read_from_stdin() != 0){
                close(listen_sock);
                for (int i = 0; i < MAX_CLIENTS; i++){
                  if (connection_list[i].socket != NO_SOCKET){
                    close(connection_list[i].socket);
                  }
                }
                printf("Shutdown server.\n");
                exit(EXIT_FAILURE);
              }
          }

          if (FD_ISSET(listen_sock, &read_fds)) {
            int port_new = handle_new_connection();

          }
          
          i = 0;
          while (i < MAX_CLIENTS) {
            if (connection_list[i].socket != NO_SOCKET && FD_ISSET(connection_list[i].socket, &read_fds)) {
              if (receive_from_peer(&connection_list[i], &handle_received_message) != 0) {
                //close_client_connection(&connection_list[i]); // works without closing the client
                continue;
              }

            }
    
            if (connection_list[i].socket != NO_SOCKET && FD_ISSET(connection_list[i].socket, &write_fds)) {
              if (send_to_peer(&connection_list[i]) != 0) {
                close_client_connection(&connection_list[i]);
                continue;
              }
            }
            i++;

          }
      
       }
   
    
  }

  return 0;
}


