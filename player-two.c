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

// Boolean that tells us if Player One has made it through the maze
bool maze_done;

/**
 * Thread for pacing the narrative of the game and controlling the game play
 */
void* narrate(void* args) {
  // Introduction sequence
  ui_display("Narrator","You wake up.");
  sleep(2);
  ui_display("Narrator","Taking a look around, you see you are trapped in a stone chamber with a large padlocked door to the side.");
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
  while(1){
    if (received_message && sent_message) {
      break;
    }
  }

  // Start maze sequence
  ui_display("Narrator", "Other than the cracks and the door, the room you are in is empty, save for a strange lever almost directly in front of where you woke up.");
  sleep(2);
  ui_display("Narrator", "Pull the lever [type :pull]");

  while (1) {
    if (maze_done) {
      break;
    }
  }

  ui_display("Narrator", "A keypad pops open on the wall near the door. [Type :door]");

  // Wait for door to start
  while(1) {
   if(door_running_check()) break;
  }
  // Wait for door to finish
  while(1) {
   if(!door_running_check()) break;
  }
  ui_display("Narrator", "The door opens!");
  // TODO: add stuff about a door w/ a number lock on it here too. they can look at either.

  message_info_t info = {"Data", "opened"};
  send_message(fd, info);

  //Anagram game
  ui_display("Narrator", "Another emtpy room!");
  ui_display("Narrator", "But wait!! There is a small box in the corner. Let's see what's inside. [type :open]");

  while(1){
    if(box_running_check() == 0 || box_running_check() == 1) break;
  }

  ui_display("Narrator", "These words do not make much sense, but it seems like the letters can be moved around.");
  sleep(2);
  ui_display("Narrator", "Enter '[correct sequence]' to rearrange these words");

  while(1){
    if (box_running_check() == 2){
      ui_display("Narrator", "It seems like your partner is struggling. Communicate and help them.");
    }
    if (box_running_check() == 3) break;
  }

  ui_display("Narrator", "Congratulations! Both of you have cracked the code!!.. (room vibrating, werid noise,...) Now.. you Computer Scientists should prepare yourself for SEGFAULT blah blah blah.");

  
  return NULL;
} // narrate

/**
 * This function is run whenever Player Two hits enter after typing a message.
 * 
 * \param message  The string holding the message that Player Two wants to send to Player One
 */
void input_callback(const char* message) {
  // Quitting mechanismƒ
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  }
  // Message ':pull' calls the maze game 
  else if (strcmp(message, ":pull") == 0) {
    if (!maze_running_check()) {
      ui_maze(2);
    }
    else {
      ui_display("Narrator", "You have already pulled the lever.");
    }
  }
  // Message ':door' calls the door (display door with lock)
  else if (strcmp(message, ":door") == 0 || strcmp(message, ":d") == 0) {
    if (!door_running_check()) {
      ui_door();
    }
    else {
      ui_display("Narrator", "You are already at the door.");
    }
  }

    // Message ':open' calls the box 
  else if (strcmp(message, ":open") == 0 || strcmp(message, ":o") == 0) {
    if (box_running_check() == 2 || box_running_check() == 3) {
      ui_box(2);
    }
    else {
      ui_display("Narrator", "You have already opened this box");
    }
  }
  
  
  // Otherwise, display the message in the chat
  else { 
    ui_display("Player Two", message); 
  }
  // Send the message to Player One if they are connected
  if (fd != -1) {
    message_info_t info = {"Player Two", (char*)message};
    if (send_message(fd, info) == -1) {
      perror("send_message to Player One has failed");
      exit(EXIT_FAILURE);
    }
  }
  // We have now sent a message, so set this bool to true
  sent_message = true;
} // input_callback


/**
 * Thread function for receiving messages from Player One.
 * 
 * No arguments are passed in.
 */
void* player_one_receive(void* args) {
  // Continuously receive messages
  while(1) {
      // Read a message from Player One
      message_info_t info = receive_message(fd);
      // Error checks
      if (info.username == NULL) {
        perror("receive_message from Player One has failed");
        exit(EXIT_FAILURE);
      }
      if (info.message == NULL) {
        perror("receive_message from Player One has failed");
        exit(EXIT_FAILURE);
      }
      // Break out of the receive loop if Player One quit
      if ((strcmp(info.message, ":q") == 0) || (strcmp(info.message, ":quit") == 0)) {
        ui_display("WARNING", "PLAYER 1 HAS QUIT");
        break;
      }
      // Don't display the message if Player One is trying to show the maze
      else if ((strcmp(info.message, ":enter") == 0)) {
        continue;
      }
      else if (strcmp(info.message, ":open") == 0){
        continue;
      }
      else if ((strcmp(info.message, ":view") == 0) || (strcmp(info.message, ":v") == 0)) {
        continue;
      }
      // We received data from Player One
      else if (strcmp(info.username, "Data") == 0) {
        if (strcmp(info.message, "escaped") == 0) maze_done = true;
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
} // player_one_receive

// Set up connection and communication to Player One. 
// Set up UI
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

  // Create a separate thread to handle communication between players (receive from Player One)
  pthread_t receive_thread;
  if (pthread_create(&receive_thread, NULL, player_one_receive, NULL) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Set up the user interface. The input_callback function will be called
  // each time the user hits enter to send a message.
  ui_init(input_callback);

  // Create a separate thread for the narration
  pthread_t narration;
  if (pthread_create(&narration, NULL, narrate, NULL) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
  ui_run();

  // TODO: CHECK THIS
  // TODO: CLOSE ALL SOCKETS, FREE ALL THINGS
  close(fd);

  return 0;
} // main