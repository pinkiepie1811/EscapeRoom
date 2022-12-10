#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

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
  else if (strcmp(message, ":pull") == 0 || strcmp(message, ":m") == 0) {
    ui_maze(2);
    ui_display("Narrator", "[You may have pulled the lever, but remember, when you want to look at the door, type :door]");
  }
  else { 
    ui_display("Player Two", message); 
  }
  if (fd != -1) {
    message_info_t info = {"Player Two", (char*)message};
    if (send_message(fd, info) == -1) {
      perror("send_message to Player One has failed");
      exit(EXIT_FAILURE);
    }
    sent_message = true;
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
      // Read a message from Player Two
      message_info_t info = receive_message(fd);
      if (info.username == NULL) {
        perror("receive_message from Player One has failed");
        exit(EXIT_FAILURE);
      }
      if (info.message == NULL) {
        perror("receive_message from Player One has failed");
        exit(EXIT_FAILURE);
      }
      if ((strcmp(info.message, ":q") == 0) || (strcmp(info.message, ":quit") == 0)) {
        ui_display("WARNING", "PLAYER 1 HAS QUIT");
        break;
      }
      if ((strcmp(info.message, ":enter") == 0) || (strcmp(info.message, ":m") == 0)) {
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
} // player_one_receive

void* narrate(void* args) {
  ui_display("Narrator","You wake up.");
  sleep(1);
  ui_display("Narrator","Taking a look around, you see you are trapped in a stone chamber with a large padlocked door to the side.");
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
  while(1){
    if (received_message && sent_message) {
      break;
    }
  }
  ui_display("Narrator", "Other than the cracks and the door, the room you are in is empty, save for a strange lever almost directly in front of where you woke up.");
  sleep(1);
  ui_display("Narrator", "Pull the lever [type :pull] or look at the door [type :door]?");

  // add stuff about a door w/ a number lock on it here too. they can look at either.
  
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


  pthread_t narrative;
  if (pthread_create(&narrative, NULL, narrate, NULL) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
  ui_run();

  // Close socket EDIT
  // close(socket_fd);

  return 0;
}