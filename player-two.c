#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"
#include "ui.h"

int fd; 

// This function is run whenever the user hits enter after typing a message
void input_callback(const char* message) {
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  }
  else { 
    ui_display("Player Two", message); 
  }
  if (send_message(fd, (char*)message) == -1) {
    perror("send_message to Player One has failed");
    exit(EXIT_FAILURE);
  }
}

// Make two threads: one for sending messages and one for receiving messages
// Thread for receiving messages from Player One
void* player_one_receive(void* args) {
  // String to hold the received message
  char* message = malloc(MAX_MESSAGE_LENGTH);

  // Continuously receive messages
  while(1) {
      // Read a message from Player One
      message = receive_message(fd);
      if (message == NULL) {
        perror("receive_message from Player One has failed");
        exit(EXIT_FAILURE);
      }
      if ((strcmp(message, ":q") == 0) || (strcmp(message, ":quit") == 0)) {
        ui_display("WARNING", "Player One has quit");
        break;
      }

      // Print the message otherwise
      ui_display("Player One", message);
  }

  return NULL;
} // player_one_receive

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
  fd = socket_connect(server_name, port);
  if (fd == -1) {
    perror("Failed to connect");
    exit(EXIT_FAILURE);
  }

  // Create a separate thread to handle communication between players
  pthread_t receive_thread;
  if (pthread_create(&receive_thread, NULL, player_one_receive, NULL) != 0) {
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