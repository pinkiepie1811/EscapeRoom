#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "message.h"
#include "socket.h"
#include "ui.h"

// -- GLOBAL VARIABLES -- //

// The socket to send and receive messages across to and from Player Two
// Initialize to -1 so we know not to send messages until we are connected to Player Two
int fd = -1;

// Booleans that tell us whether the Player One has received the first message from
// Player Two and whether Player One has sent the first message to Player Two (in beginning of game)
bool received_message = false;
bool sent_message = false;

// Boolean that tells us if Player Two has unlocked the door
bool door_done = false;

// Boolean that tells us if Player Two has solved their anagram
bool box2_done = false;

void* timer();            // Thread function that runs the timer
void* boss_attack_func(); // Thread function that controls the boss attacks and sends data to Player Two (for boss fight)

/**
 * Thread function for pacing the narrative of the game and controlling the game play.
 * This function creates and calls other thread functions to run concurrently. 
 */
void* narrate(void* args) {
  // -- INTRODUCTION -- //
  narrate_display("You wake up.");
  narrate_display("Taking a look around, you see you are trapped in a stone chamber.");
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
  while(1) {
    if (received_message && sent_message) break;
  }

  // Create the timer thread to start the countdown
  pthread_t timer_thread;
  if (pthread_create(&timer_thread, NULL, timer, NULL) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // -- MAZE -- //
  narrate_display("As you consider your situation, the cracks in the wall in front of you start to glow even brighter, before they abruptly split apart into a pathway.");
  narrate_display("You poke your head in, and there looks to be a set of tunnels ahead. However, the glow in the walls is limited to the room you are in; if you step, in you will be walking in the dark.");
  narrate_display("Still, you have no other choice. [Type ':enter' to enter the darkness, and use your arrow keys to navigate to the end 'E'.]");
  
  // Wait for maze to start
  while(1) {
    if(maze_running_check()) break;
  }
  // Wait for maze to finish
  while(1) {
    if(!maze_running_check()) break;
  }

  // Player One got through the maze, so send that information to Player Two
  message_info_t maze_info = {"data", "escaped"};
  if (send_message(fd, maze_info) == -1) {
    perror("send_message of maze information to Player Two has failed");
    exit(EXIT_FAILURE);
  }

  // -- DOOR -- //
  narrate_display("You made it through the maze! You step out of the darkness and into a large cavern.");
  narrate_display( "A piece of paper flutters by your feet. Curious, you reach to pick it up. [Type ':view' to look at the paper]");

  // Wait until Player Two has unlocked the door
  while(1) {
    if (door_done) break;
  }

  // -- ANAGRAM -- //
  narrate_display("You've helped your friend unlock the door!");
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
  
  // Once Player One has solved their anagram, send that it is solved to Player Two
  message_info_t box_info = {"data", "solved_one"};
  if (send_message(fd, box_info) == -1) {
    perror("send_message of box information to Player Two has failed");
    exit(EXIT_FAILURE);
  }

  // If Player Two has not solved their anagram yet, let Player One know
  if (!box2_done) {
    narrate_display("Your friend seems to be struggling to solve their puzzle. Communicate and help them!");
  }
  // Wait until Player Two has sovled their anagram
  while(1) {
    if(box2_done) break;
  }

  // -- FINAL BOSS -- //
  narrate_display("Both of you have cracked the code! You hesitantly say the code out loud, only to hear another voice say 'Charlie Curtsinger' at the other end of the cavern.");
  narrate_display("You peer out into the darkness and see that it's your friend! But as you reunite, you realize...");
  narrate_display("There's a... giant octopus? in the cavern with you! And it's shooting lasers at both of you!");
  narrate_display("[Type ':fight' to start combat]");

  // Wait for final boss fight to start
  while(1) {
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
  // Send data that we have run out of time to Player Two so they know we are exiting
  message_info_t info = {"data", "time"};
  send_message(fd, info); // Don't error check here because we are only sending just in case Player Two hasn't exited yet
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
      perror("send_message of damage to Player Two has failed");
      exit(EXIT_FAILURE);
    }

    // Send our x-position
    info.username = "posx";
    sprintf(mess, "%d", get_pos_x());
    info.message = mess;
    if (send_message(fd, info) == -1) {
      perror("send_message of x-position to Player Two has failed");
      exit(EXIT_FAILURE);
    }

    // Send our y-position
    info.username = "posy";
    sprintf(mess, "%d", get_pos_y());
    info.message = mess;
    if (send_message(fd, info) == -1) {
      perror("send_message of y-position to Player Two has failed");
      exit(EXIT_FAILURE);
    }

    // Call boss_attack to update laser attacks
    boss_attack();
    // Call ui_boss to update the game board
    ui_boss();

    // Sleep for 0.5 seconds to give players time to dodge the attacks
    usleep(1000 * 500);
  }

  // Once the final boss fight is over, send the outstanding damage over to Player Two
  message_info_t last;
  char mess[10]; // Message is no more than 10 characters

  // Send the outstanding damage
  last.username = "dam";
  sprintf(mess, "%d", change_damage());
  last.message = mess;
  if (send_message(fd, last) == -1) {
    perror("send_message of last damage to Player Two has failed");
    exit(EXIT_FAILURE);
  }

  return NULL;
} // boss_attack_func

/**
 * This function is run whenever Player One hits enter after typing a message.
 * 
 * \param message  The string holding the message that Player One wants to send to Player Two or the command
 *                 for the game.
 */
void input_callback(const char* message) {
  // Quitting/exiting mechanism
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0 || strcmp(message, ":exit") == 0) {
    ui_exit();
  }
  // Message of ':enter' calls the maze game 
  else if (strcmp(message, ":enter") == 0) {
    if (!maze_running_check()) {
      ui_maze(1);
    }
    // If they try to type ':enter' twice, tell them they already did that
    else {
      ui_display("Narrator", "You are already in the maze.");
    }
  }
  // Message of ':view' shows the paper for the door (in Player Two)
  else if (strcmp(message, ":view") == 0) {
    ui_paper();
    // Assume that players won't try to ':view' twice
  }
  // Message of ':open' shows the anagram box 
  else if (strcmp(message, ":open") == 0) {
    if (!box_running_check()) {
      ui_box(1);
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
    ui_display("Player One", message); 
  }

  // Send the message to Player Two if they are connected
  // We send all messages, including game commands
  if (fd != -1) {
    message_info_t info = {"Player One", (char*)message};
    if (send_message(fd, info) == -1) {
      perror("send_message to Player Two has failed");
      exit(EXIT_FAILURE);
    }
  }

  // We have now sent a message, so set this bool to true
  sent_message = true;
} // input_callback

/**
 * Thread function for receiving messages from Player Two.
 */
void* player_two_receive() {
  // Continuously receive messages
  while(1) {
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

    // Break out of the receive loop if Player Two quit (stop receiving messages from them)
    if ((strcmp(info.message, ":q") == 0) || (strcmp(info.message, ":quit") == 0)) {
      ui_display("WARNING", "PLAYER 2 HAS QUIT");
      break;
    }
    // ENDING: break out of loop if they won the game and are exiting
    else if (strcmp(info.message, ":exit") == 0) {
      break;
    }

    // Don't display the message if Player Two is trying to start any of the minigames
    // (Not displaying game commands that are sent)
    if ((strcmp(info.message, ":pull") == 0) || 
        (strcmp(info.message, ":door") == 0) || 
        (strcmp(info.message, ":open") == 0) || 
        (strcmp(info.message, ":fight") == 0)) {
      continue;
    }

    // If we received data from Player Two...
    else if (strcmp(info.username, "data") == 0) {
      // Data for door opening; set bool to true to continue narrative
      if (strcmp(info.message, "opened") == 0) door_done = true;
      // Data for anagram solved; set bool to true to continue narrative
      else if (strcmp(info.message, "solved_two") == 0) box2_done = true;
      // Data for time has run out, so we break out of receiving loop
      else if (strcmp(info.message, "time") == 0) break;

      // In all cases besides 'time', we continue
      continue;
    }

    // We received damage from Player Two
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
    // We received position information from Player Two
    else if (strcmp(info.username, "posx") == 0) {
      // Get and change Player Two's x-position on our board
      int x = atoi(info.message); // We can error check this but if x is zero (indicating an error), it doesn't matter
      change_p2_posx(x);
      continue;
    }
    else if (strcmp(info.username, "posy") == 0) {
      // Get and change Player Two's y-position on our board
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
} // player_two_receive

/**
 * Thread function to seek connection from Player Two.
 * 
 * \param server_socket_fd  The server fd to allow for connection to Player Two.
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

// Set up connection and communication to Player Two
// Set up UI
int main() {
  // Open a server socket
  unsigned short port = 0;
  int server_socket_fd = server_socket_open(&port);
  if (server_socket_fd == -1) {
    perror("server socket was not opened");
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
  if (sprintf(buffer, "Connect Player Two to port %u\n", port) < 0) {
    perror("sprintf failed");
    exit(EXIT_FAILURE);
  }
  ui_display("INFO", buffer);

  // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
  ui_run();
  
  if (close(server_socket_fd) < 0) {
    perror("failed to close socket");
    exit(EXIT_FAILURE);
  }

  return 0;
} // main