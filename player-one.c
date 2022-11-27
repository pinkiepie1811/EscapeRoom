#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"

// Make two threads: one for sending messages and one for receiving messages
// Thread for receiving messages from Player Two
void* player_two_receive(void* receive_socket_fd) {
  // File descriptor to receive messages
  int fd = *((int*)receive_socket_fd);
  // String to hold the received message
  char* message = malloc(MAX_MESSAGE_LENGTH);

  // Continuously receive messages
  while(1) {
      // Read a message from Player Two
      message = receive_message(fd);
      if (message == NULL) {
        perror("Failed to read message from client");
        exit(EXIT_FAILURE);
      }

      // Print the message otherwise
      printf("Player Two: %s", message);
  }

  return NULL;
} // player_two_receive

// Thread for sending messages to Player Two
void* player_two_send(void* send_socket_fd) {
  // File descriptor to receive messages
  int fd = *((int*)send_socket_fd);
  // String to hold the message Player One wishes to send to Player Two
  char * input = malloc(MAX_MESSAGE_LENGTH);

  // Get the line of input
  fgets(input, MAX_MESSAGE_LENGTH, stdin);

  // Continuously send messages
  while (input != NULL) {
    // Send a message to Player Two
    int rc = send_message(fd, input);
    if (rc == -1) {
      perror("Failed to send message to server");
      exit(EXIT_FAILURE);
    }

    // Get the next line of input
    fgets(input, MAX_MESSAGE_LENGTH, stdin);
  }

  // Free input string
  free(input);

  // Close the socket
  close(fd);

  return NULL;
} // player_two_send

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
  int socket_fd = server_socket_accept(server_socket_fd);
  if (socket_fd == -1) {
    perror("accept failed");
    exit(EXIT_FAILURE);
  }

  printf("Client connected!\n");

  // Create a separate thread to handle communication between players
  pthread_t receive_thread;
  if (pthread_create(&receive_thread, NULL, player_two_receive, (void*)&socket_fd) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Create a separate thread to handle communication between players
  pthread_t send_thread;
  if (pthread_create(&send_thread, NULL, player_two_send, (void*)&socket_fd) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  while(1);
  
  // Close socket EDIT
  close(server_socket_fd);
} // main