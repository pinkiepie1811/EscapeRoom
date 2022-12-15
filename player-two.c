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

bool box1_done = false;

/**
 * @brief TODO
 * 
 * @param args 
 * @return void* 
 */
void* timer(void* args) {
  while(ui_time() != -1) {   
    sleep(1);
  }
  message_info_t info = {"Data", "time"};
  send_message(fd, info);
  ui_exit();
  printf("\nYou're too late! Your time has run out. The ceiling has collapsed on you and your friend. The end.\n\n");
  return NULL;
}

void* boss_attack_func(void* args) {
  while(boss_running_check()) {
    message_info_t info;
    char mess[10];

    info.username = "dam";
    sprintf(mess, "%d", change_damage());
    info.message = mess;
    send_message(fd, info);

    info.username = "posx";
    sprintf(mess, "%d", get_pos_x());
    info.message = mess;
    send_message(fd, info);

    info.username = "posy";
    sprintf(mess, "%d", get_pos_y());
    info.message = mess;
    send_message(fd, info);

    boss_attack();
    ui_boss();

    usleep(1000 * 500);
  }
  message_info_t last;
    char mess[10];

    last.username = "dam";
    sprintf(mess, "%d", change_damage());
    last.message = mess;
    send_message(fd, last);
  return NULL;
}

/**
 * Thread for pacing the narrative of the game and controlling the game play
 */
void* narrate(void* args) {
  
  // Introduction sequence
  narrate_display("You wake up.");
  narrate_display("Taking a look around, you see you are trapped in a stone chamber with a large padlocked door to the side.");
  narrate_display("You hear the faint trickle of water, and a strange light seems to glow from the cracks in the wall.");
  narrate_display("Even as you look, these cracks grow wider: the room is vibrating, and every so often, the sound of earth collapsing and rocks crashing into themselves echoes from beyond.");
  narrate_display("You need to escape before it is too late!");
  narrate_display("Your phone starts to buzz in your pocket, but when you check it out, it has no signal.");
  narrate_display("Instead, it seems a strange app has taken over your whole screen! It looks like... a text editor?");
  narrate_display("You try typing something in. What's this?");
  narrate_display("It seems someone else is on the other end of this line- maybe they are stuck too.");
  narrate_display("Perhaps you can use this strange app to communicate, and maybe even help each other escape!");
  narrate_display("Try sending a message to each other now!");

  // Wait for players to try the chat
  while(1){
    if (received_message && sent_message) {
      break;
    }
  }
  pthread_t timer_thread;
  if (pthread_create(&timer_thread, NULL, timer, NULL) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Start maze sequence
  narrate_display("Other than the cracks and the door, the room you are in is empty, save for a strange lever almost directly in front of where you woke up.");
  narrate_display("You wonder if there is any way to help your friend.");
  narrate_display("Your option is to pull the lever. Pull the lever [Type ':pull']");

  while (1) {
    if (maze_done) {
      break;
    }
  }

  narrate_display( "A keypad pops open on the door. [Type ':door']");

  // Wait for door to start
  while(1) {
   if(door_running_check()) break;
  }

  narrate_display( "[Use the arrow keys to adjust the numbers on the lock]");
  // Wait for door to finish

  while(1) {
   if(!door_running_check()) break;
  }
  // TODO: add stuff about a door w/ a number lock on it here too. they can look at either.

  message_info_t info = {"Data", "opened"};
  send_message(fd, info);

  narrate_display( "The door opens into a giant cavern!");
  
  
    //Anagram game

  narrate_display( "But wait! There is a small box in the corner. Let's see what's inside. [Type ':open']");

  while(1){
    if(box_running_check()) break;
  }

  narrate_display( "These words do not make much sense, but it seems like the letters can be moved around.");
  narrate_display( "Enter '[correct sequence]' to rearrange these words into a name (without spaces).");

  // Wait for box to finish
  while(1) {
   if(!box_running_check()) break;
  }

  message_info_t box_info = {"Data", "solved_two"};
  send_message(fd, box_info);

  if (box1_done == false){
      narrate_display( "Your friend seems to be struggling. Communicate and help them!");
  }

  while(1){
    if(box1_done) break;
  }

  narrate_display( "Both of you have cracked the code! You hesitantly say the code out loud, only to hear another voice say 'PM Osera' at the other end of the cavern.");
  narrate_display( "It's your friend! As you reunite, you realize...");
  narrate_display( "It looks like theres a.... giant octopus? in the cavern with you! Its shooting lasers!");
  narrate_display( "[Type ':fight' to start combat]");

  while (1) {
    if (boss_running_check()) {
      break;
    }
  }
  pthread_t boss_attack_thread;
  if (pthread_create(&boss_attack_thread, NULL, boss_attack_func, NULL) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }
  narrate_display( "[Use arrow keys to move and avoid attacks]");
  narrate_display( "[Touch the monster to do damage]");

  // Wait for door to finish
  while(1) {
   if(!boss_running_check()) break;
  }

  narrate_display( "The octopus dissolves into the floor!");
  narrate_display( "The open sky lies beyond. [Enter ':exit' to escape to freedom!]");

  return NULL;
} // narrate

/**
 * This function is run whenever Player Two hits enter after typing a message.
 * 
 * \param message  The string holding the message that Player Two wants to send to Player One
 */
void input_callback(const char* message) {
  // Quitting mechanismƒ
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0 || strcmp(message, ":exit") == 0) {
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
  else if (strcmp(message, ":fight") == 0 || strcmp(message, ":f") == 0) {
    ui_boss(10, 18);
  }

    // Message ':open' calls the box 
  else if (strcmp(message, ":open") == 0 || strcmp(message, ":o") == 0) {
    if (!box_running_check()) {
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
      else if (strcmp(info.message, ":exit") == 0) {
        break;
      }
      // Don't display the message if Player One is trying to show the maze
      else if ((strcmp(info.message, ":enter") == 0)) {
        continue;
      }
      else if (strcmp(info.message, ":open") == 0){
        continue;
      }
      else if (strcmp(info.message, ":view") == 0) {
        continue;
      }
      else if (strcmp(info.message, ":fight") == 0) {
        continue;
      }
      // We received data from Player One
      else if (strcmp(info.username, "Data") == 0) {
        if (strcmp(info.message, "escaped") == 0) maze_done = true;
        else if (strcmp(info.message, "solved_one") == 0) box1_done = true;
        else if (strcmp(info.message, "time") == 0)  break;
        continue;
      }

       // We received data from Player One
      else if (strcmp(info.username, "dam") == 0) {
        int damage = atoi(info.message);
        do_damage(damage);
        if (damage != 0) ui_display("Narrator", "Your friend attacked the octopus!");
        continue;
      }
      else if (strcmp(info.username, "posx") == 0) {
        int x = atoi(info.message);
        change_p2_posx(x);
        continue;
      }
      else if (strcmp(info.username, "posy") == 0) {
        int y = atoi(info.message);
        change_p2_posy(y);
        continue;
      }


      // We have now received a message; set bool to to true
      received_message = true;

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