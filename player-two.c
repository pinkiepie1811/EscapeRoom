#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"

// Thread for communication 
void* player_one_send(void* socket_fd) {
  int fd = *((int*)socket_fd);
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

void* player_one_receive(void* client_socket_fd) {
  int fd = *((int*)client_socket_fd);
  char* message = malloc(MAX_MESSAGE_LENGTH);
  while(1) {
      // Read a message from the client
      message = receive_message(fd);
      if (message == NULL) {
        perror("Failed to read message from client");
        exit(EXIT_FAILURE);
      }

      printf("Player One: %s", message);
  }

  // Free the message string
  free(message);

  // Close sockets
  close(fd);

  return NULL;
}

int main(int argc, char** argv) {
  // Check user input
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <server name> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Read command line arguments
  char* server_name = argv[1];
  unsigned short port = atoi(argv[2]);

  // Connect to the server
  int socket_fd = socket_connect(server_name, port);
  if (socket_fd == -1) {
    perror("Failed to connect");
    exit(EXIT_FAILURE);
  }

  // Create a separate thread to handle communication between players
  pthread_t thread;
  if (pthread_create(&thread, NULL, player_one_receive, (void*)&socket_fd) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Create a separate thread to handle communication between players
  pthread_t thread2;
  if (pthread_create(&thread2, NULL, player_one_send, (void*)&socket_fd) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }
  
  while(1) {}

  // Close socket EDIT
  close(socket_fd);
}