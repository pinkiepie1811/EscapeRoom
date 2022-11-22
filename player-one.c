#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"

// I THINK WE NEED TWO THREADS, ONE TO RECEIVE AND ONE TO SEND
// Thread for communication
void* player_two_receive(void* client_socket_fd) {
  int fd = *((int*)client_socket_fd);
  char* message = malloc(MAX_MESSAGE_LENGTH);
  while(1) {
      // Read a message from the client
      message = receive_message(fd);
      if (message == NULL) {
        perror("Failed to read message from client");
        exit(EXIT_FAILURE);
      }

      printf("Player Two: %s", message);
  }

  // Free the message string
  free(message);

  // Close sockets
  close(fd);

  return NULL;
}

void* player_two_send(void* client_socket_fd) {
  int fd = *((int*)client_socket_fd);
  char * user_input = malloc(MAX_MESSAGE_LENGTH);

  // Get the line of input
  fgets(user_input, MAX_MESSAGE_LENGTH, stdin);

  while (user_input != NULL) {
    // Send a message to the server
    int rc = send_message(fd, user_input);
    if (rc == -1) {
      perror("Failed to send message to server");
      exit(EXIT_FAILURE);
    }

    // Break if user types 'quit'
    if(strcmp(user_input, "quit\n") == 0){
        break;
    }

    // Get the next line of input
    fgets(user_input, MAX_MESSAGE_LENGTH, stdin);
  }

  // Free user input
  free(user_input);

  // Close socket
  close(fd);

  return NULL;
}

int main() {
  // Open a server socket
  unsigned short port = 0;
  int server_socket_fd = server_socket_open(&port);
  if (server_socket_fd == -1) {
    perror("Server socket was not opened");
    exit(EXIT_FAILURE);
  }

  // Start listening for connections, with a maximum of one queued connection
  if (listen(server_socket_fd, 1)) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  // Display title screen with port number on it
  // Replace the printf statement below with title screen
  printf("Server listening on port %u\n", port);

  // Wait for a client to connect
  int client_socket_fd = server_socket_accept(server_socket_fd);
  if (client_socket_fd == -1) {
    perror("accept failed");
    exit(EXIT_FAILURE);
  }

  printf("Client connected!\n");

  // Create a separate thread to handle communication between players
  pthread_t thread;
  if (pthread_create(&thread, NULL, player_two_receive, (void*)&client_socket_fd) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Create a separate thread to handle communication between players
  pthread_t thread2;
  if (pthread_create(&thread2, NULL, player_two_send, (void*)&client_socket_fd) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  while(1);
  
  // Close socket EDIT
  close(server_socket_fd);
}