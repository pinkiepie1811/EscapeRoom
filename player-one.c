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
// for introduction sequence
bool received_message = false;
bool sent_message = false;

bool door_done = false;

/**
 * Thread for pacing the narrative of the game and controlling the game play
 */
void* narrate(void* args) {
  // Introduction sequence
  ui_display("Narrator","You wake up.");
  sleep(2);
  ui_display("Narrator","Taking a look around, you see you are trapped in a stone chamber.");
  sleep(2);
  ui_display("Narrator","You hear the faint trickle of water, and a strange light seems to glow from the cracks in the wall.");
  sleep(2);
  ui_display("Narrator","Even as you look, these cracks grow wider: the room is vibrating, and every so often, the sound of earth collapsing and rocks crashing into themselves echoes from beyond.");
  sleep(2);
  ui_display("Narrator","You need to escape before it is too late!");
  sleep(2);
  ui_display("Narrator","Your phone starts to buzz in your pocket, but when you check it out, it has no signal.");
  sleep(2);
  ui_display("Narrator","Instead, it seems a strange app has taken over your whole screen! It looks like... a text editor?");
  sleep(2);
  ui_display("Narrator","You try typing something in. What's this?");
  sleep(2);
  ui_display("Narrator","It seems someone else is on the other end of this line- maybe they are stuck too.");
  sleep(2);
  ui_display("Narrator","Perhaps you can use this strange app to communicate, and maybe even help each other escape!");
  sleep(2);
  ui_display("Narrator","Try sending a message to each other now!");

  // Wait for players to try the chat
  while(1) {
    if (received_message && sent_message) {
      break;
    }
  }

  // Start maze sequence
  ui_display("Narrator", "As you consider your situation, the cracks in the wall in front of you start to glow brighter, before they abruptly split apart into a pathway.");
  sleep(2);
  ui_display("Narrator", "You poke your head in, and realize you there looks to be a set of tunnels ahead.");
  sleep(2);
  ui_display("Narrator", "However, the glow in the walls is limited to your room- if you step in you will be walking in the dark."); 
  sleep(2);
  ui_display("Narrator", "Still, you have little other choice.");
  sleep(2);
  ui_display("Narrator", "[Type :enter to enter the darkness. Use your arrow keys to navigate.]");
  
  // Wait for maze to start
  while(1) {
   if(maze_running_check()) break;
  }
  // Wait for maze to finish
  while(1) {
   if(!maze_running_check()) break;
  }
  ui_display("Narrator", "Congrats! You made it through the maze! You step out of the darkness to a large cavern.");
  sleep(1);

  message_info_t info = {"Data", "escaped"};
  send_message(fd, info);

  ui_display("Narrator", "You see a piece of paper. [Type :view to look at the paper");
  sleep(1);

  while (1) {
    if (door_done) {
      break;
    }
  }

   //Anagram game
  ui_display("Narrator", "Another emtpy room!");
  ui_display("Narrator", "But wait!! There is a small box in the corner. Let's see what's inside. [type :open]");

  while(1){
    if(box_running_check() == 0 || box_running_check() == 2) break;
  }

  ui_display("Narrator", "These words do not make much sense, but it seems like the letters can be moved around.");
  sleep(2);
  ui_display("Narrator", "Enter '[correct sequence]' to rearrange these words");


  while(1){
    if (box_running_check() == 1){
      ui_display("Narrator", "It seems like your partner is struggling. Communicate and help them.");
    }
    if (box_running_check() == 3) break;
  }

  ui_display("Narrator", "Congratulations! Both of you have cracked the code!!.. (room vibrating, werid noise,...) Now.. you Computer Scientists should prepare yourself for SEGFAULT blah blah blah.");

  
  ui_display("Narrator", "MONSTOR");

  return NULL;
} // narrate

/**
 * This function is run whenever Player One hits enter after typing a message.
 * 
 * \param message  The string holding the message that Player One wants to send to Player Two
 */
void input_callback(const char* message) {
  // Quitting mechanism
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  }
  // Message of ':enter' calls the maze game 
  else if (strcmp(message, ":enter") == 0){
    if (!maze_running_check()) {
      ui_maze(1);
    }
    else {
      ui_display("Narrator", "You are already in the maze.");
    }
  }
  else if (strcmp(message, ":view") == 0 || strcmp(message, ":v") == 0) {
    ui_paper();
  }

     // Message ':open' calls the box 
  else if (strcmp(message, ":open") == 0 || strcmp(message, ":o") == 0) {
    if (box_running_check() == 1 || box_running_check() == 3) {
      ui_box(1);
    }
    else {
      ui_display("Narrator", "You have already opened this box");
    }
  }  

  // Otherwise, display the message in the chat
  else { 
    ui_display("Player One", message); 
  }
  // Send the message to Player Two if they are connected
  if (fd != -1) {
    message_info_t info = {"Player One", (char*)message};
    if (send_message(fd, info) == -1) {
      perror("send_message to Player One has failed");
      exit(EXIT_FAILURE);
    }
  }
  // We have now sent a message, so set this bool to true
  sent_message = true;
} // input_callback

/**
 * Thread function for receiving messages from Player Two.
 * 
 * No arguments are passed in.
 */
void* player_two_receive(void* arg) {
  // Continuously receive messages
  while (1) {
      // Read a message from Player Two
      message_info_t info = receive_message(fd);
      // Error checks
      if (info.username == NULL) {
        perror("receive_message from Player Two has failed");
        exit(EXIT_FAILURE);
      }
      if (info.message == NULL) {
        perror("receive_message from Player Two has failed");
        exit(EXIT_FAILURE);
      }
      // Break out of the receive loop if Player Two quit
      if ((strcmp(info.message, ":q") == 0) || (strcmp(info.message, ":quit") == 0)) {
        ui_display("WARNING", "PLAYER 2 HAS QUIT");
        break;
      }
      // Don't display the message if Player Two is trying to start the maze
      if ((strcmp(info.message, ":pull") == 0)) {
        continue;
      }
      else if (strcmp(info.message, ":door") == 0) {
        continue;
      }

      else if (strcmp(info.message, ":open") == 0){
        continue;
      }
      // We received data from Player Two
      else if (strcmp(info.username, "Data") == 0) {
        if (strcmp(info.message, "opened") == 0) door_done = true;
        continue;
      }

      // We have now received a message; set bool to to true
      received_message = true;
      
      /**
       * if (username = username) uidisplay
       * if username = damage) total_damage+=atoi(message) damage
       * if username = maze_solved if message = "true" maze
       */
      // Print the message otherwise
      ui_display(info.username, info.message);
      // Free the message information
      free(info.username);
      free(info.message);
  }
  // fd is invalid now
  fd = -1;
  return NULL;
} // player_two_receive

/**
 * Thread function to seek connection from Player Two.
 * 
 * \param server_socket_fd  The server fd to allow for connection to Player Two 
 */
void* connect_players(void* server_socket_fd) {
  // Get the server fd from the arguments
  int server_fd = *((int*)server_socket_fd);

  // Wait for Player Two to connect
  fd = server_socket_accept(server_fd);
  if (fd == -1) {
    perror("accept failed");
    exit(EXIT_FAILURE);
  }
  
  // Create a separate thread to handle communication between players (receive from Player Two)
  pthread_t receive_thread;
  if (pthread_create(&receive_thread, NULL, player_two_receive, NULL) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Create a separate thread for the narration
  pthread_t narration;
  if (pthread_create(&narration, NULL, narrate, NULL) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }
  
  return NULL;
} // connect_players

// Set up connection and communication to Player Two. 
// Set up UI
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

  // Start another thread to connect the players
  pthread_t accept_connection;
  if (pthread_create(&accept_connection, NULL, connect_players, (void*)&server_socket_fd) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Set up the user interface. The input_callback function will be called
  // each time the user hits enter to send a message.
  ui_init(input_callback);

  // Display the port number for Player Two to connect to
  char buffer[50];
  sprintf(buffer, "Connect Player Two to port %u\n", port);
  ui_display("INFO", buffer);

  // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
  ui_run();
  
  // TODO: CHECK THIS
  // TODO: CLOSE ALL SOCKETS, FREE ALL THINGS
  close(server_socket_fd);

  return 0;
} // main