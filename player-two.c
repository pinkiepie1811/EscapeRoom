#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "message.h"
#include "socket.h"
#include "ui.h"

// The socket to send and receive messages across to and from Player One
// Initialize to -1 so we know not to send messages until we are connected to Player One
int fd = -1;

// Booleans that tell us whether the Player Two has received the first message from
// Player One and whether Player Two has sent the first message to Player One (in beginning of introduction sequence)
bool received_message = false;
bool sent_message = false;

// Boolean that tells us if Player One has made it through the maze
bool maze_done;

// Boolean that tells us if Player One has solved their anagram
bool box1_done = false;

void* timer();            // Thread function that runs the timer
void* boss_attack_func(); // Thread function that controls the boss attacks and sends data to Player Two (for boss fight)

/**
 * Thread function for pacing the narrative of the game and controlling the game play.
 * This function creates and calls other thread functions to run concurrently. 
 */
void* narrate(void* args) {
  // -- INTRODUCTION -- //
  narrate_display("You wake up.");
  narrate_display("Taking a look around, you see you are trapped in a stone chamber with a large locked door to the side.");
  narrate_display("You hear the faint trickle of water, and a strange light seems to glow from the cracks in the wall.");
  narrate_display("But as you look, these cracks grow wider: the room is vibrating, and every so often, the sound of earth collapsing and rocks crashing into themselves echoes from beyond.");
  narrate_display("You need to escape before it is too late!");
  narrate_display("Your phone starts to buzz in your pocket, but when you check it out, it has no signal.");
  narrate_display("Instead, it seems a strange app has taken over your whole screen! It looks like... a text editor?");
  narrate_display("You try typing something in. What's this?");
  narrate_display("It seems someone else is on the other end of this line; they might be stuck too.");
  narrate_display("Perhaps you can use this strange app to communicate, and maybe even help each other escape!");
  narrate_display("Try sending a message to each other now!");
  narrate_display("[Type in a message using your keyboard and press the 'Enter' button to send it.]");

  // Wait for players to send and receive messages from each other
  while(1){
    if (received_message && sent_message) break;
  }

  // Create the timer thread to start the countdown
  pthread_t timer_thread;
  if (pthread_create(&timer_thread, NULL, timer, NULL) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // -- MAZE -- //
  narrate_display("Other than the cracks and the door, the room you are in is empty, save for a strange lever almost directly in front of where you woke up.");
  narrate_display("You wonder if there is any way to help your friend.");
  narrate_display("It seems that your only option for now is to pull the lever. [Type ':pull' to pull the lever]");

  // Wait until Player One has gone through the maze
  while (1) {
    if (maze_done) break;
  }

  // -- ANAGRAM -- //
  narrate_display("It seems that your friend has made it through the dark tunnels!");
  narrate_display("A keypad suddenly pops open on the door. [Type ':door' to try to unlock the door to get out!]");

  // Wait for door to start
  while(1) {
   if (door_running_check()) break;
  }

  // Explain the door/math game
  narrate_display("[Use the arrow keys to adjust the numbers on the lock. Use left and right to choose which number to change; use up and down to increase or decrease the number you are on]");
  // Wait for door to finish

  // Wait for door to finish
  while(1) {
   if (!door_running_check()) break;
  }

  // Player Two unlocked the door, so send that information to Player One
  message_info_t door_info = {"data", "opened"};
  if (send_message(fd, door_info) == -1) {
    perror("send_message of door information to Player One has failed");
    exit(EXIT_FAILURE);
  }

  // -- ANAGRAM -- //
  narrate_display("The door opens into a giant cavern!");
  narrate_display("But wait! You suddenly notice a small box in the corner of the cavern. Let's see what's inside. [Type ':open' to open the box]");

  // Wait for anagram/box to start
  while(1) {
    if (box_running_check()) break;
  }

  // Explain the anagrams
  narrate_display( "The box has some words engraved in it, but they do not make much sense. It seems like the letters can be moved around.");
  narrate_display( "Enter '[correct letter sequence]' to rearrange these letters into a name (without spaces).");

  // Wait for anagram/box to finish
  while(1) {
   if (!box_running_check()) break;
  }

  // Once Player Two has solved their anagram, send that it is solved to Player One
  message_info_t box_info = {"data", "solved_two"};
  if (send_message(fd, box_info) == -1) {
    perror("send_message of box information to Player One has failed");
    exit(EXIT_FAILURE);
  }

  // If Player One has not solved their anagram yet, let Player Two know
  if (!box1_done) {
    narrate_display("Your friend seems to be struggling to solve their puzzle. Communicate and help them!");
  }
  // Wait until Player One has sovled their anagram
  while(1) {
    if(box1_done) break;
  }

  // -- FINAL BOSS -- //
  narrate_display("Both of you have cracked the code! You hesitantly say the code out loud, only to hear another voice say 'PM Osera' at the other end of the cavern.");
  narrate_display("You peer out into the darkness and see that it's your friend! But as you reunite, you realize...");
  narrate_display("There's a... giant octopus? in the cavern with you! And it's shooting lasers at both of you!");
  narrate_display("[Type ':fight' to start combat]");

  // Wait for final boss fight to start
  while (1) {
    if (boss_running_check()) break;
  }

  // Explain the boss fight
  narrate_display("[Use your arrow keys to move around and avoid the laser attacks]");

  // Create the boss attack thread to start the boss laser attacks
  pthread_t boss_attack_thread;
  if (pthread_create(&boss_attack_thread, NULL, boss_attack_func, NULL) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  narrate_display("[Run up and touch the monster to do damage. The human touch is poisonous to him!]");

  // Wait for final boss fight to finish
  while(1) {
   if(!boss_running_check()) break;
  }

  // -- ENDING -- //
  narrate_display("Success! You defeated the monster! The octopus dissolves into the floor, its screams of agony dissolving in the air.");
  narrate_display("The cavern rumbles and opens. The open sky lies beyond. [Type ':exit' to escape to freedom!]");

  return NULL;
} // narrate

/**
 * The thread function that runs the timer to let players know how much time they
 * have left to escape. 
 */
void* timer() {
  // Sleep 1 second if time isn't up yet.
  // Once time is up, ui_time will return -1, indicating that the players have run out of time
  while(ui_time() != -1) {   
    sleep(1);
  }

  // Once the players have run out of time, we must exit the game
  // Send data that we have run out of time to Player One so they know we are exiting
  message_info_t info = {"data", "time"};
  send_message(fd, info); // Don't error check here because we are only sending just in case Player One hasn't exited yet
  // Exit
  ui_exit();
  // Print in terminal to let them know why they can't play the game anymore
  printf("\nYou're too late! Your time has run out. The ceiling has collapsed on you and your friend. The end.\n\n");

  return NULL;
} // timer

/**
 * The thread function that continuously updates the monster's laser attacks and sends position
 * and damage data to Player Two
 */
void* boss_attack_func() {
  while(boss_running_check()) {
  // Local message_info_t used to send messages over
    message_info_t info;
    char mess[10]; // Message is no more than 10 characters

    // Send damage
    info.username = "dam";
    sprintf(mess, "%d", change_damage());
    info.message = mess;
    if (send_message(fd, info) == -1) {
      perror("send_message of damage to Player One has failed");
      exit(EXIT_FAILURE);
    }

    // Send our x-position
    info.username = "posx";
    sprintf(mess, "%d", get_pos_x());
    info.message = mess;
    if (send_message(fd, info) == -1) {
      perror("send_message of x-position to Player One has failed");
      exit(EXIT_FAILURE);
    }

    // Send our y-position
    info.username = "posy";
    sprintf(mess, "%d", get_pos_y());
    info.message = mess;
    if (send_message(fd, info) == -1) {
      perror("send_message of y-position to Player One has failed");
      exit(EXIT_FAILURE);
    }

    // Call boss_attack to update laser attacks
    boss_attack();
    // Call ui_boss to update the game board
    ui_boss();

    // Sleep for 0.5 seconds to give players time to dodge the attacks
    usleep(1000 * 500);
  }

  // Once the final boss fight is over, send the outstanding damage over to Player One
  message_info_t last;
  char mess[10]; // Message is no more than 10 characters

  // Send the outstanding damage
  last.username = "dam";
  sprintf(mess, "%d", change_damage());
  last.message = mess;
  if (send_message(fd, last) == -1) {
      perror("send_message of last damage to Player One has failed");
      exit(EXIT_FAILURE);
    }

  return NULL;
} // boss_attack_func

/**
 * This function is run whenever Player Two hits enter after typing a message.
 * 
 * \param message  The string holding the message that Player Two wants to send to Player One or the command
 *                 for the game.
 */
void input_callback(const char* message) {
  // Quitting/exiting mechanism
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0 || strcmp(message, ":exit") == 0) {
    ui_exit();
  }
  // Message ':pull' calls the maze game (shows map)
  else if (strcmp(message, ":pull") == 0) {
    ui_maze(2);
    // Assume that players won't try to ':pull' twice
  }
  // Message ':door' calls the door (display door with lock)
  else if (strcmp(message, ":door") == 0) {
    if (!door_running_check()) {
      ui_door();
    }
    // If they try to type ':door' twice, tell them they already did that
    else {
      ui_display("Narrator", "You are already at the door.");
    }
  }
  // Message ':open' shows the anagram box 
  else if (strcmp(message, ":open") == 0) {
    if (!box_running_check()) {
      ui_box(2);
    }
    // If they try to type ':open' twice, tell them they already did that
    else {
      ui_display("Narrator", "You have already opened this box.");
    }
  }
  // Message of ':fight' starts the final boss fight
  else if (strcmp(message, ":fight") == 0) {
    if (!boss_running_check()) {
      // The player starts at coordinate (10, 18)
      ui_boss(10, 18);
    }
    // If they try to type ':fight' twice, tell them they already did that
    else {
      ui_display("Narrator", "You are already fighting the octopus.");
    }
  }
  // Otherwise, the message is not a game command, so display the message in the chat
  else { 
    ui_display("Player Two", message); 
  }

  // Send the message to Player One if they are connected
  // We send all messages, including game commands
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
    // ENDING: break out of loop if they won the game and are exiting
    else if (strcmp(info.message, ":exit") == 0) {
      break;
    }

    // Don't display the message if Player One is trying to start any of the minigames
    // (Not displaying game commands that are sent)
    if ((strcmp(info.message, ":enter") == 0) || 
        (strcmp(info.message, ":view") == 0) || 
        (strcmp(info.message, ":open") == 0) || 
        (strcmp(info.message, ":fight") == 0)) {
      continue;
    }
    // If we received data from Player One...
    else if (strcmp(info.username, "data") == 0) {
      // Data for maze being done; set bool to true to continue narrative
      if (strcmp(info.message, "escaped") == 0) maze_done = true;
      // Data for anagram solved; set bool to true to continue narrative
      else if (strcmp(info.message, "solved_one") == 0) box1_done = true;
      // Data for time has run out, so we break out of receiving loop
      else if (strcmp(info.message, "time") == 0)  break;

      // In all cases besides 'time', we continue
      continue;
    }

    // We received damage from Player One
    else if (strcmp(info.username, "dam") == 0) {
      // Get and update damage to our global
      int damage = atoi(info.message);
      // Damage > 0 means actual damage was done
      if (damage != 0) {
        do_damage(damage);
        ui_display("Narrator", "Your friend attacked the octopus!");
      }
      continue;
    }
    // We received position information from Player One
    else if (strcmp(info.username, "posx") == 0) {
      // Get and change Player One's x-position on our board
      int x = atoi(info.message); // We can error check this but if x is zero (indicating an error), it doesn't matter
      change_p2_posx(x);
      continue;
    }
    else if (strcmp(info.username, "posy") == 0) {
      // Get and change Player One's y-position on our board
      int y = atoi(info.message); // We can error check this but if y is zero (indicating an error), it doesn't matter
      change_p2_posy(y);
      continue;
    }

    // We have now received a message; set bool to to true
    received_message = true;

    // ELSE CASE: print the message in the chat
    ui_display(info.username, info.message);

    // Free the message information
    free(info.username);
    free(info.message);
  }

  // If we break, then the fd is invalid now
  if (close(fd) < 0) {
    perror("failed to close socket");
    exit(EXIT_FAILURE);
  }
  fd = -1;

  return NULL;
} // player_one_receive

// Set up connection and communication to Player One
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

  return 0;
} // main