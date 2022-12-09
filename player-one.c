#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>s

#include "message.h"
#include "socket.h"
#include "ui.h"

int fd = -1;
bool received_message = false;
bool sent_message = false;

// This function is run whenever the user hits enter after typing a message
void input_callback(const char* message) {
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  }
  else if (strcmp(message, ":maze") == 0 || strcmp(message, ":m") == 0) {
    ui_maze(1);
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
  sent_message = true;
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
        ui_display("WARNING", "PLAYER 2 HAS QUIT");
        break;
      }

      received_message = true;
      // Print the message otherwise
      ui_display("Player Two", message);
  }
  fd = -1;
  free(message);
  return NULL;
} // player_two_receive

void* narrate(void* args) {
  ui_display("Narrator","You wake up. Taking a look around, you see you are trapped in a stone chamber. \
  You hear the faint trickle of water, and a strange light seems to glow from the cracks in the wall.  \
  Even as you look, these cracks grow wider: the room is vibrating, and every so often, \
  the sound of earth collapsing and rocks crashing into themselves echoes from beyond. \
  You need to escape before it is too late!");
  sleep(5);
  ui_display("Narrator","You wake up. Taking a look around, you see you are trapped in a stone chamber.\
  You hear the faint trickle of water, and a strange light seems to glow from the cracks in the wall.\
  Even as you look, these cracks grow wider: the room is vibrating, and every so often,\
  the sound of earth collapsing and rocks crashing into themselves echoes from beyond. You need to escape before it is too late!\
  Your phone starts to buzz in your pocket, but when you check it out, it has no signal.\
  Instead, it seems a strange app has taken over your whole screen! It looks like... a text editor? \
  You try typing something in. What's this? \
  It seems someone else is on the other end of this line- maybe they are stuck too.\
  Perhaps you can use this strange app to communicate, and maybe even help each other escape!");
  while(1){
    if (received_message && sent_message){
      break;
    }
  }
  ui_display("Narrator", "As you consider your situation, the cracks in the wall in front of you start to glow brighter, before they abruptly split apart into a pathway.\
   You poke your head in, and realize you there looks to be a set of tunnels ahead.\
   However, the glow in the walls is limited to your room- if you step in you will be walking in the dark. Still, you have little other choice.");
return NULL;
}

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

  pthread_t narrative;
  if (pthread_create(&narrative, NULL, narrate, NULL) != 0) {
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