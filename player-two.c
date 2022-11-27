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
    ui_display("Player Two", message);
  }
}

// Make two threads: one for sending messages and one for receiving messages
// Thread for receiving messages from Player One
void* player_one_receive(void* receive_socket_fd) {
  // File descriptor to receive messages
  int fd = *((int*)receive_socket_fd);
  // String to hold the received message
  char* message = malloc(MAX_MESSAGE_LENGTH);

  // Continuously receive messages
  while(1) {
      // Read a message from Player One
      message = receive_message(fd);
      if (message == NULL) {
        perror("Failed to read message from client");
        exit(EXIT_FAILURE);
      }

      // Print the message otherwise
      // printf("Player One: %s", message);
      ui_display("Player One", message);
  }

  return NULL;
} // player_one_receive

// Thread for sending messages to Player One
void* player_one_send(void* send_socket_fd) {
  // File descriptor to receive messages
  int fd = *((int*)send_socket_fd);
  // String to hold the message Player Two wishes to send to Player One
  char * input = malloc(MAX_MESSAGE_LENGTH);

  // Get the line of input
  fgets(input, MAX_MESSAGE_LENGTH, stdin);

  // Continuously send messages
  while (input != NULL) {
    // Send a message to Player One
    int rc = send_message(fd, input);
    if (rc == -1) {
      perror("Failed to send message to server");
      exit(EXIT_FAILURE);
    }

    // Get the next line of input
    fgets(input, MAX_MESSAGE_LENGTH, stdin);
  }

  return NULL;
} // player_one_send

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
  pthread_t receive_thread;
  if (pthread_create(&receive_thread, NULL, player_one_receive, (void*)&socket_fd) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Create a separate thread to handle communication between players
  pthread_t send_thread;
  if (pthread_create(&send_thread, NULL, player_one_send, (void*)&socket_fd) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Set up the user interface. The input_callback function will be called
  // each time the user hits enter to send a message.
  ui_init(input_callback);

  // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
  ui_run();

  // Close socket EDIT
  // close(socket_fd);

  return 0;
}