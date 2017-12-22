
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#include "message_handler.h"
#include "declarations.h"
#include "conference_handler.h"
#include "communication_handler.h"


peer_struct server;
fd_set read_fds;
fd_set write_fds;
fd_set except_fds;
char client_name[256];
char password[256];


/* DISCALIMER: 
 * You should be careful when using this function in multithread program. 
 * Ensure that server is thread-safe. */



void shutdown_client(int code){
  delete_peer(&server);
  printf("shutdown client client.\n");
  exit(code);
}


/*MAGIC CONTROL


DESCRIPTION        

 The sigaction() system call is used to change the action taken by a
 process on receipt of a specific signal.  

 signum specifies the signal and can be any valid signal except
 SIGKILL and SIGSTOP.

 If act is non-NULL, the new action for signal signum is installed
 from act.  If oldact is non-NULL, the previous action is saved in
 oldact.

 The sigaction structure is defined as something like:

 struct sigaction {
     void     (*sa_handler)(int);
     void     (*sa_sigaction)(int, siginfo_t *, void *);
     sigset_t   sa_mask;
     int        sa_flags;
     void     (*sa_restorer)(void);
 };


*/


void handle_signal_action(int sig_number){
  if (sig_number == SIGINT) {
    shutdown_client(EXIT_SUCCESS);
  }
  else if (sig_number == SIGPIPE) {
    shutdown_client(EXIT_SUCCESS);
  }
}

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

//Function to obtain client name

int obtain_client_name(int argc, char **argv, char *client_name){
  strcpy(client_name, argv[1]);
  return 0;
}



//Function to obtain client password

int obtain_client_password(int argc, char **argv, char *password){
  strcpy(password, argv[2]);
  return 0;
}


// Function to connect client to server (Referred from Beej's guide)

int connect_server(peer_struct *server, char *client_id){
  
   FILE *client_name_temp_store_file= fopen("client_name_temp.txt", "w");
   fprintf(client_name_temp_store_file, "%s", client_id);
   fclose(client_name_temp_store_file);

  // create socket
  server->socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server->socket < 0) {
    perror("socket()");
    return -1;
  }
  
  // set up address
  struct sockaddr_in server_addr, client_addr;
  int len = sizeof(client_addr);
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(STATION_SERVER_ADDRESS_IPV4);
  server_addr.sin_port = htons(LISTENER_PORT);
  
  server->address = server_addr;
  
  if (connect(server->socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) != 0) {
    perror("connect()");
    return -1;
  }
  
  printf("Connected to %s:%d.\n", STATION_SERVER_ADDRESS_IPV4, LISTENER_PORT);

  return 0;
}


//Function to extract message provided by client

void get_msg_field_by_delimiter(char *message, int field_no, char *return_str, char delimiter){

    //field_no 1: message_type UNLESS default message??
    //field_no 2: session_id (for CREATE/JOIN/LEAVE sessions)


    int str_length = strlen(message);
    int semi_counter = 0;
    int buff_char_count = 0;
    char curr_char;

    for(int i = 0; i < str_length; i++){
        curr_char = message[i];
        if(curr_char == delimiter){
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

        if (i == str_length-1){
          return;
        }
    }

    memset(return_str, 0, sizeof(return_str));

}

// Helper function to help concatanate strings as per protocol

void new_concat_str(char *str1, char* str2, char* output_str){
        char *txt = ":";
        strcpy(output_str, str1);
        strcat(output_str, txt);
        strcat(output_str, str2);
        strcat(output_str, txt);
}


//Function to obtain message through STDIN
//Through this function we classify the packet requests into:
/*
1. /create
2. /join
3. /leave
4. /swap
5. /listsessions
6. /listclients
7. /swap
8. /current
9. /exit
*/
//NOTE: logout and login are taken care directly through server

int handle_read_from_stdin(peer_struct *server, char *client_name){
  char read_buffer[DATA_SIZE_MAX]; // buffer for stdin
  if (read_from_stdin(read_buffer, DATA_SIZE_MAX) != 0)
    return -1;
  
  // Create new message and enqueue it.
  message_data new_message;

  //
  char msg_type[DATA_SIZE_MAX];
  memset(msg_type, 0, sizeof(msg_type)); //THIS IS IMPORTANT!!!
  char delimiter;

  delimiter = ' ';

  get_msg_field_by_delimiter(read_buffer,1,msg_type,delimiter);

  char outbound_msg[DATA_SIZE_MAX];
  memset(outbound_msg, 0, sizeof(outbound_msg));

  if (strcmp(msg_type,"/create") == 0){

    delimiter = ' ';

    char session_name[DATA_SIZE_MAX];
    memset(session_name,0,sizeof(session_name));
    get_msg_field_by_delimiter(read_buffer,2,session_name,delimiter);
    new_concat_str(msg_type, session_name, outbound_msg);
  }

  else if (strcmp(msg_type,"/join") == 0){

    delimiter = ' ';

    char session_name[DATA_SIZE_MAX];
    memset(session_name,0,sizeof(session_name));
    get_msg_field_by_delimiter(read_buffer,2,session_name,delimiter);
    new_concat_str(msg_type, session_name, outbound_msg);

  }

  else if (strcmp(msg_type, "/leave") == 0){

    delimiter = ' ';

    char session_name[DATA_SIZE_MAX];
    memset(session_name,0,sizeof(session_name));
    get_msg_field_by_delimiter(read_buffer,2,session_name,delimiter);
    new_concat_str(msg_type, session_name, outbound_msg);

  }else if(strcmp(msg_type, "/listsessions") == 0){
    list_sessions();

    return 0;
  }

  else if(strcmp(msg_type, "/listclients") == 0){

    delimiter = ' ';

    char session_name[DATA_SIZE_MAX];
    memset(session_name,0,sizeof(session_name));
    get_msg_field_by_delimiter(read_buffer,2,session_name,delimiter);
    list_session_clients(session_name);

    return 0;
  }

  else if(strcmp(msg_type, "/swap") == 0){

    delimiter = ' ';

    char session_name[DATA_SIZE_MAX];
    memset(session_name,0,sizeof(session_name));
    get_msg_field_by_delimiter(read_buffer,2,session_name,delimiter);
    new_concat_str(msg_type, session_name, outbound_msg);
  }

  else if(strcmp(msg_type, "/current") == 0){

    delimiter = ' ';

    char session_name[DATA_SIZE_MAX];
    memset(session_name,0,sizeof(session_name));
    strcat(msg_type, ":");
    strcpy(outbound_msg, msg_type);
  }

  else if(strcmp(msg_type, "/exit") == 0){
    
    strcat(msg_type, ":");
    strcpy(outbound_msg, msg_type);
    prepare_message(client_name, outbound_msg, &new_message);
    peer_add_to_send(server, &new_message);
    shutdown_client(EXIT_SUCCESS);
  }

  else{

    prepare_message(client_name, read_buffer, &new_message);

    if (peer_add_to_send(server, &new_message) != 0) {
      return 0;
    }
    
    print_message(&new_message);
    return 0;

  }

  prepare_message(client_name, outbound_msg, &new_message);
  print_message(&new_message);

  if (peer_add_to_send(server, &new_message) != 0) {
    return 0;
  }
  
  return 0;
}




int handle_received_message(message_data *message){
  print_message(message);
  return 0;
}


// Function to check authentacity of the user

int check_user(char client_id[BUF_SIZE], char password[BUF_SIZE]){
    char buffer[BUF_SIZE];
    char *msg;
    int ret;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_counter = 0;

    //open the file to read
    FILE *fp = fopen("client_database.txt", "r");
    if (fp == NULL)
        exit(1);
    
    while ((read = getline(&line, &len, fp)) != -1) {

        char* packet_str = line;
        char* line_user = (char*)malloc(100 * sizeof(char));
        char* line_pwd = (char*)malloc(100 * sizeof(char));
        
        //Separate line into strings ...
        int start = 0;
        //Read value till first colon into total_frag
        int index = 0;

        while(packet_str[start + index] != ':'){
            line_user[index] = packet_str[start+index];
            index += 1;
        }
        start += index;
        start += 1; //Skip the ';' encountered when starting next read!
        printf(line_user);
        printf("\n");

        //Read value till second colon into frag_no
        index = 0;  
        while(packet_str[start + index] != ':'){
            line_pwd[index] = packet_str[start+index];
            index += 1;
        }
        start += index;
        start += 1; //Skip the ';' encountered when starting next read!

        ///////////Compare!!!!/////////
        int cmp_user = strcmp(client_id, line_user);
        int cmp_pwd = strcmp(password, line_pwd);
        
        if (cmp_user == 0){
            
            if (cmp_pwd == 0){
                return 0; //Found username and password!
            }
            else{
                return 1; //Password doesn't match :(!
            }
        }
        line_counter++;
    }

    if (line)
        free(line);

    return 2; //No matching entry in DB!
}


//MAIN starts

int main(int argc, char **argv){
  if (setup_signals() > 0)
    exit(EXIT_FAILURE);

  if(argc!=3){
    printf("%s\n", "usage: client <userid> <password>");
    exit(EXIT_FAILURE);
  }
  

  obtain_client_name(argc, argv, client_name);
  obtain_client_password(argc, argv, password);

  if(check_user(client_name, password) == 0){
    //
  }else{
    printf("%s\n", "Incorrect Credentials");
    exit(1);
  }
  
  create_peer(&server);
  if (connect_server(&server, client_name) != 0)
    shutdown_client(EXIT_FAILURE);
  
  /* Set nonblock for stdin. */
  int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
  flag |= O_NONBLOCK;
  fcntl(STDIN_FILENO, F_SETFL, flag);

  // server socket always will be greater then STDIN_FILENO
  int maxfd = server.socket;

  printf("Waiting for server message or stdin input. Please create and join a session for text conferencing\n");
  
  for(;;) {
    // Select() updates fd_set's, so we need to build fd_set's before each select()call.
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(server.socket, &read_fds);
    
    FD_ZERO(&write_fds);
    // smth to send, set up write_fd for server socket
    if (server.send_buffer.current > 0)
      FD_SET(server.socket, &write_fds);
    
    FD_ZERO(&except_fds);
    FD_SET(STDIN_FILENO, &except_fds);
    FD_SET(server.socket, &except_fds);
        
    int activity = select(maxfd + 1, &read_fds, &write_fds, &except_fds, NULL);
    

    if(activity != -1 && activity != 0){
      /* All fd_set's should be checked. */
      if (FD_ISSET(STDIN_FILENO, &read_fds)) {
        if (handle_read_from_stdin(&server, client_name) != 0)
          shutdown_client(EXIT_FAILURE);
      }

      if (FD_ISSET(STDIN_FILENO, &except_fds)) {
        shutdown_client(EXIT_FAILURE);
      }

      if (FD_ISSET(server.socket, &read_fds)) {
        if (receive_from_peer(&server, &handle_received_message) != 0)
          shutdown_client(EXIT_FAILURE);
      }

      if (FD_ISSET(server.socket, &write_fds)) {
        if (send_to_peer(&server) != 0)
          shutdown_client(EXIT_FAILURE);
      }

      if (FD_ISSET(server.socket, &except_fds)) {
        shutdown_client(EXIT_FAILURE);
      }
    }else if(activity == -1){
        perror("select()");
        shutdown_client(EXIT_FAILURE);
    }else{
        printf("select() returns 0.\n");
        shutdown_client(EXIT_FAILURE);
    }
    
  }
  
  return 0;
}
