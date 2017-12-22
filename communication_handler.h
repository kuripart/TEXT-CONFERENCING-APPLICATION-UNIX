#ifndef COMMUNICATION_HANDLER
#define COMMUNICATION_HANDLER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "declarations.h"
#include "message_handler.h"


size_t len_to_receive;
ssize_t received_count;
size_t received_total;

size_t len_to_send;
ssize_t send_count;
size_t send_total;

// PEER MANAGEMENT --------------------------------------------------

char *peer_get_addres_str(peer_struct *peer){
  static char ret[INET_ADDRSTRLEN + 10];
  char peer_ipv4_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &peer->address.sin_addr, peer_ipv4_str, INET_ADDRSTRLEN);
  sprintf(ret, "%s:%d", peer_ipv4_str, peer->address.sin_port);
  wait();
  return ret;
}



int peer_add_to_send(peer_struct *peer, message_data *message){
  if(enqueue_message(&peer->send_buffer, message) == 0){
    wait();
    return 0;
  }else{
    return -1;
  }
}

int delete_peer(peer_struct *peer){
  close(peer->socket);
  delete_message_queue(&peer->send_buffer);
  wait();
}

int create_peer(peer_struct *peer){
  create_message_queue(BUFFER_MAX_MSG_SIZE, &peer->send_buffer);
  
  peer->current_sending_byte = -1;
  peer->current_receiving_byte = 0;
  wait();

  return 0;
}


/* Receive message from peer and handle it with message_handler(). */
int receive_from_peer(peer_struct *peer, int (*message_handler)(message_data *)){

  if (peer->current_receiving_byte >= sizeof(peer->receiving_buffer)) {
    message_handler(&peer->receiving_buffer);
    peer->current_receiving_byte = 0;
  }
  
  // Count bytes to send.
  len_to_receive = sizeof(peer->receiving_buffer) - peer->current_receiving_byte;
  if (len_to_receive > MAX_SIZE_TO_SEND){
    len_to_receive = MAX_SIZE_TO_SEND;
  }
  
  received_count = recv(peer->socket, (char *)&peer->receiving_buffer + peer->current_receiving_byte, len_to_receive, MSG_DONTWAIT);

  while(received_count > 0){

    if (received_count < 0) {
      if (errno == EAGAIN) {
        //
      }
      else {
        return -1;
      }
    } 
    else if (received_count < 0 && errno == EAGAIN) {
      break;
    }
    else if (received_count == 0) {
      return -1;
    }
    else if (received_count > 0) {
      peer->current_receiving_byte += received_count;
      received_total += received_count;

      if (peer->current_receiving_byte >= sizeof(peer->receiving_buffer)) {
        message_handler(&peer->receiving_buffer);
        peer->current_receiving_byte = 0;
      }
      
      // Count bytes to send.
      len_to_receive = sizeof(peer->receiving_buffer) - peer->current_receiving_byte;
      if (len_to_receive > MAX_SIZE_TO_SEND)
        len_to_receive = MAX_SIZE_TO_SEND;
      
      received_count = recv(peer->socket, (char *)&peer->receiving_buffer + peer->current_receiving_byte, len_to_receive, MSG_DONTWAIT);
    }

  }

  wait();
  return 0;
}

int send_to_peer(peer_struct *peer){

  do {
    if (peer->current_sending_byte < 0 || peer->current_sending_byte >= sizeof(peer->sending_buffer)) {
      if (dequeue_message(&peer->send_buffer, &peer->sending_buffer) != 0) {
        peer->current_sending_byte = -1;
        break;
      }
      peer->current_sending_byte = 0;
    }
    
    // Count bytes to send.
    len_to_send = sizeof(peer->sending_buffer) - peer->current_sending_byte;
    if (len_to_send > MAX_SIZE_TO_SEND)
      len_to_send = MAX_SIZE_TO_SEND;

    send_count = send(peer->socket, (char *)&peer->sending_buffer + peer->current_sending_byte, len_to_send, 0);
    if (send_count < 0) {
      if (errno == EAGAIN) {
        //
      }
      else {
        return -1;
      }
    }

    // we have read as many as possible
    else if (send_count < 0 && errno == EAGAIN) {
      break;
    }
    else if (send_count == 0) {
      break;
    }
    else if (send_count > 0) {
      peer->current_sending_byte += send_count;
      send_total += send_count;
    }
  }
  while (send_count > 0);

  wait();
  return 0;
}


/* Reads from stdin and create new message. This message enqueue_messages to send queueu. */
int read_from_stdin(char *read_buffer, size_t max_len){
  memset(read_buffer, 0, max_len);
  
  ssize_t read_count = 0;
  ssize_t total_read = 0;
  
  do {
    read_count = read(STDIN_FILENO, read_buffer, max_len);
    if (read_count < 0 && errno != EAGAIN) {
      return -1;
    }
    else if (read_count < 0 && errno == EAGAIN) {
      break;
    }
    else if (read_count > 0) {
      total_read = total_read + read_count;
      if (total_read > max_len) {
        fflush(STDIN_FILENO);
        break;
      }
    }
  } while (read_count > 0);
  
  size_t len = strlen(read_buffer);
  if (len > 0 && read_buffer[len - 1] == '\n')
    read_buffer[len - 1] = '\0';


  if(strcmp(read_buffer,"logout") == 0){
    printf("%s\n","LOGGING OUT\n");
    exit(1);
  }

  wait();
  return 0;
}


#endif /* COMMUNICATION_HANDLER */