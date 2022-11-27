#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"
#include "ui.h"

// This function is run whenever the user hits enter after typing a message
void input_callback(const char* message) {
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  } else {
    ui_display("Player One", message);
  }
}

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
      // printf("Player Two: %s", message);

      ui_display("Player Two", message);
  }

  free(message);

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
  // close(fd);

  return NULL;
} // player_two_send

void* connect_players(void* server_socket_fd) {

  int fd = *((int*)server_socket_fd);

  // Wait for a client to connect
  int socket_fd = server_socket_accept(fd);
  if (socket_fd == -1) {
    perror("accept failed");
    exit(EXIT_FAILURE);
  }

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

  pthread_t accept_connection;
  if (pthread_create(&accept_connection, NULL, connect_players, (void*)&server_socket_fd) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Set up the user interface. The input_callback function will be called
  // each time the user hits enter to send a message.
  ui_init(input_callback);

  // Display title screen with port number on it
  // Replace the printf statement below with title screen
  // printf("Server listening on port %u\n", port);
  char buffer[50];
  sprintf(buffer, "Connect Player Two to port %u\n", port);
  ui_display("INFO", buffer);

  // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
  ui_run();
  
  // Close socket EDIT
  // close(server_socket_fd);

  return 0;
} // main