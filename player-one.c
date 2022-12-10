#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "message.h"
#include "socket.h"
#include "ui.h"

// The socket to send and receive messages across to and from Player Two
// Initialize to -1 so we know not to send messages until we are connected to Player Two
int fd = -1;

// Booleans that tell us whether the Player One has received the first message from
// Player Two and whether Player One has sent the first message to Player Two
// For introduction sequence
bool received_message = false;
bool sent_message = false;

// This function is run whenever the user hits enter after typing a message
void input_callback(const char* message) {
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  }
  else if (strcmp(message, ":enter") == 0 || strcmp(message, ":m") == 0) {
    ui_maze(1);
  }
  else { 
    ui_display("Player One", message); 
  }
  if (fd != -1) {
    message_info_t info = {"Player One", (char*)message};
    if (send_message(fd, info) == -1) {
      perror("send_message to Player One has failed");
      exit(EXIT_FAILURE);
    }
  }
  sent_message = true;
}

// Make two threads: one for sending messages and one for receiving messages
// Thread for receiving messages from Player Two
void* player_two_receive(void* arg) {
  // Continuously receive messages
  while(1) {
      // Read a message from Player Two
      message_info_t info = receive_message(fd);
      if (info.username == NULL) {
        perror("receive_message from Player Two has failed");
        exit(EXIT_FAILURE);
      }
      if (info.message == NULL) {
        perror("receive_message from Player Two has failed");
        exit(EXIT_FAILURE);
      }
      if ((strcmp(info.message, ":q") == 0) || (strcmp(info.message, ":quit") == 0)) {
        ui_display("WARNING", "PLAYER 2 HAS QUIT");
        break;
      }
      if ((strcmp(info.message, ":pull") == 0) || (strcmp(info.message, ":m") == 0)) {
        continue;
      }

      received_message = true;
      // Print the message otherwise

      /**
       * if (username = username) uidisplay
       * if username = damage) total_damage+=atoi(message) damage
       * if username = maze_solved if message = "true" maze
       */
      ui_display(info.username, info.message);
      free(info.username);
      free(info.message);
  }
  fd = -1;
  return NULL;
} // player_two_receive

void* narrate(void* args) {
  ui_display("Narrator","You wake up.");
  sleep(1);
  ui_display("Narrator","Taking a look around, you see you are trapped in a stone chamber.");
  sleep(1);
  ui_display("Narrator","You hear the faint trickle of water, and a strange light seems to glow from the cracks in the wall.");
  sleep(1);
  ui_display("Narrator","Even as you look, these cracks grow wider: the room is vibrating, and every so often, the sound of earth collapsing and rocks crashing into themselves echoes from beyond.");
  sleep(1);
  ui_display("Narrator","You need to escape before it is too late!");
  sleep(1);
  ui_display("Narrator","Your phone starts to buzz in your pocket, but when you check it out, it has no signal.");
  sleep(1);
  ui_display("Narrator","Instead, it seems a strange app has taken over your whole screen! It looks like... a text editor?");
  sleep(1);
  ui_display("Narrator","You try typing something in. What's this?");
  sleep(1);
  ui_display("Narrator","It seems someone else is on the other end of this line- maybe they are stuck too.");
  sleep(1);
  ui_display("Narrator","Perhaps you can use this strange app to communicate, and maybe even help each other escape!");
  sleep(1);
  ui_display("Narrator","Try sending a message to each other now!");
  while(1) {
    if (received_message && sent_message) {
      break;
    }
  }
  ui_display("Narrator", "As you consider your situation, the cracks in the wall in front of you start to glow brighter, before they abruptly split apart into a pathway.");
  sleep(1);
  ui_display("Narrator", "You poke your head in, and realize you there looks to be a set of tunnels ahead.");
  sleep(1);
  ui_display("Narrator", "However, the glow in the walls is limited to your room- if you step in you will be walking in the dark."); 
  sleep(1);
  ui_display("Narrator", "Still, you have little other choice.");
  sleep(1);
  ui_display("Narrator", "[Type :enter to enter the darkness. Use your arrow keys to navigate.]");
  // Wait for maze to start
  while(1) {
   if(maze_running_check()) break;
  }
  // Wait for maze to finish
  while(1) {
   if(!maze_running_check()) break;
  }
  ui_display("Narrator", "Congrats! You made it through the maze!");
  //math time
return NULL;
} // narrate

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