#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"
#include "ui.h"

int fd = -1;

// This function is run whenever the user hits enter after typing a message
void input_callback(const char* message) {
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  }
  else { 
    ui_display("Player One", message); 
  }
  if (fd != -1) {
    if (send_message(fd, (char*)message) == -1) {
      perror("send_message to Player Two has failed");
      exit(EXIT_FAILURE);
    }
  }
}

// Make two threads: one for sending messages and one for receiving messages
// Thread for receiving messages from Player Two
void* player_two_receive(void* arg) {
  // String to hold the received message
  char* message = malloc(MAX_MESSAGE_LENGTH);

  // Continuously receive messages
  while(1) {
      // Read a message from Player Two
      message = receive_message(fd);
      if (message == NULL) {
        perror("receive_message from Player Two has failed");
        exit(EXIT_FAILURE);
      }
      if ((strcmp(message, ":q") == 0) || (strcmp(message, ":quit") == 0)) {
        ui_display("WARNING", "Player Two has quit");
        break;
      }

      // Print the message otherwise
      ui_display("Player Two", message);
  }

  free(message);

  return NULL;
} // player_two_receive

void* connect_players(void* server_socket_fd) {

  int server_fd = *((int*)server_socket_fd);

  // Wait for a client to connect
  fd = server_socket_accept(server_fd);
  if (fd == -1) {
    perror("accept failed");
    exit(EXIT_FAILURE);
  }
  ui_display("","connected");
  
  // Create a separate thread to handle communication between players
  pthread_t receive_thread;
  if (pthread_create(&receive_thread, NULL, player_two_receive, NULL) != 0) {
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