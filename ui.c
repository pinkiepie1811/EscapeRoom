// Modified from Peer-to-Peer Chat lab
#include "ui.h"
#include "game.h"

#include <form.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


// The height of the input field in the user interface
#define INPUT_HEIGHT 3

// The timeout for input
#define INPUT_TIMEOUT_MS 10

// The ncurses forms code is loosely based on the first example at
// http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/forms.html

// The fields array for the display form
FIELD* display_fields[2];

// The form that holds the display field
FORM* display_form;

// The fields array for the game form
FIELD* game_fields[2];

// The form that holds the game field
FORM* game_form;

// The fields array for the narrative form
FIELD* narrative_fields[2];

// The form that holds the narrative field
FORM* narrative_form;

// The fields array for the input form
FIELD* input_fields[2];

// The form that holds the input field
FORM* input_form;

// The handle for the UI thread
pthread_t ui_thread;

// A lock to protect the entire UI
pthread_mutexattr_t ui_lock_attr;
pthread_mutex_t ui_lock;

// The callback function to run on new input
static input_callback_t input_callback;

// When true, the UI should continue running
bool ui_running = false;

// When true, the maze game should run
bool maze_running = false;
// Global variables to keep track of where the player in the maze is
int maze_x = 3;
int maze_y = 0;
// Global maze
char** maze;

// The player that is using the UI (Player One or Player Two)
int stored_player = -1;

// Lock for the maze_running boolean 
pthread_mutex_t maze_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t door_lock = PTHREAD_MUTEX_INITIALIZER;


// When true, the door game should run
bool door_running = false;
// Door stuff
char nums[4] = {'0', '0', '0', '0'};
int curr_num = 0;
char** door;

// Error Protection for locking
void Pthread_mutex_lock(pthread_mutex_t* mutex) {
  if (pthread_mutex_lock(mutex) != 0) {
    perror("pthread_mutex_lock failed");
    exit(EXIT_FAILURE);
  }
} // Pthread_mutex_lock

// Error Protection for unlocking
void Pthread_mutex_unlock(pthread_mutex_t* mutex) {
  if (pthread_mutex_unlock(mutex) != 0) {
    perror("pthread_mutex_unlock failed");
    exit(EXIT_FAILURE);
  }
} // Pthread_mutex_unlock

/**
 * Returns the boolean for the maze_running
 * 
 * \return true or false depending on if the maze is currently running
 */
bool maze_running_check() {
  // Make a copy of the boolean
  // Lock to avoid race conditions
  Pthread_mutex_lock(&maze_lock);
  bool maze = maze_running;
  Pthread_mutex_unlock(&maze_lock);
  return maze;
} // maze_running_check

/**
 * Initialize the user interface and set up a callback function that should be
 * called every time there is a new message to send.
 *
 * \param callback  A function that should run every time there is new input.
 *                  The string passed to the callback should be copied if you
 *                  need to retain it after the callback function returns.
 */
void ui_init(input_callback_t callback) {
  // Initialize curses
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  timeout(INPUT_TIMEOUT_MS);
  keypad(stdscr, TRUE);

  // Get the number of rows and columns in the terminal display
  int rows = 0;
  int cols = 0;

  getmaxyx(stdscr, rows, cols);  // This uses a macro to modify rows and cols

  // Calculate the height of the display field
  int display_height = rows - INPUT_HEIGHT - 1;

  // Create the game window
  // height, width, start row, start col, overflow buffer lines, buffers
  game_fields[0] = new_field(display_height / 2 - 1, cols / 2 -1, 0, 0, 0, 0);
  game_fields[1] = NULL;

  // Create the narrative window
  // height, width, start row, start col, overflow buffer lines, buffers
  narrative_fields[0] = new_field(display_height / 2 - 1, cols / 2 -1, display_height / 2 + 1, 0, 0, 0);
  narrative_fields[1] = NULL;

  // Create the larger message display window
  // height, width, start row, start col, overflow buffer lines, buffers
  display_fields[0] = new_field(display_height, cols / 2, 0, cols / 2, 0, 0);
  display_fields[1] = NULL;

  // Create the input field
  input_fields[0] = new_field(INPUT_HEIGHT, cols / 2, display_height + 1, cols / 2, 0, 0);
  input_fields[1] = NULL;

  // Grow the display & narrative field buffers as needed
  field_opts_off(display_fields[0], O_STATIC);
  field_opts_off(narrative_fields[0], O_STATIC);

  // Don't advance to the next field automatically when using the input field
  field_opts_off(input_fields[0], O_AUTOSKIP);

  // Turn off word wrap (nice, but causes other problems)
  field_opts_off(input_fields[0], O_WRAP);
  field_opts_off(display_fields[0], O_WRAP);
  field_opts_off(game_fields[0], O_WRAP);
  // field_opts_off(narrative_fields[0], O_WRAP);

  // Create the forms
  game_form = new_form(game_fields);
  display_form = new_form(display_fields);
  input_form = new_form(input_fields);
  narrative_form = new_form(narrative_fields);

  // Display the forms
  post_form(game_form);
  post_form(display_form);
  post_form(input_form);
  post_form(narrative_form);
  refresh();

  // Draw a horizontal split
  for (int i = cols / 2; i < cols; i++) {
    mvprintw(display_height, i, "-");
  }

  // Draw a vertical split
  for (int j = 0; j < rows; j++) {
    mvprintw(j, (cols / 2) - 1, "|");
  }

  // Update the display
  refresh();

  // Save the callback function
  input_callback = callback;

  // Initialize the UI lock
  pthread_mutexattr_init(&ui_lock_attr);
  pthread_mutexattr_settype(&ui_lock_attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&ui_lock, &ui_lock_attr);

  // Read the maze
  maze = read_game("maze.txt");
  door = read_game("door.txt");

  // Running
  ui_running = true;
} // ui_init

/**
 * Run the main UI loop. This function will only return the UI is exiting.
 */
void ui_run() {
  // Loop as long as the UI is running
  while (ui_running) {
    // Get a character
    int ch = getch();

    // If there was no character, try again
    if (ch == -1) continue;

    // There was some character. Lock the UI
    pthread_mutex_lock(&ui_lock);

    // Handle input
    // Case: Delete/backspace character
    if (ch == KEY_BACKSPACE || ch == KEY_DC || ch == 127) {
      // Delete the last character when the user presses backspace
      form_driver(input_form, REQ_DEL_PREV);

    } 
    // Case: If the input is arrow keys and the maze is running currently
    else if (((ch == KEY_DOWN) || (ch == KEY_UP) || (ch == KEY_RIGHT) || (ch == KEY_LEFT)) && maze_running && stored_player == 1) {
      // Adjust the position of the player in the maze accordingly
      if (ch == KEY_RIGHT) {
        maze_x++;
      } 
      else if (ch == KEY_LEFT) {
        maze_x--;
      } 
      else if (ch == KEY_DOWN) {
        maze_y++;
      } 
      else if (ch == KEY_UP) {
        maze_y--;
      }
      // If the player is trying to move within the maze, clear the previous maze and print the new maze with
      // updated coordinates for the paper
      if ((maze_x >= 0) && (maze_x < SIZE) && (maze_y >= 0) && (maze_y < SIZE)) {
        form_driver(game_form, REQ_CLR_FIELD);
        ui_maze(1);
      }
      // Otherwise, the player is out of bounds; keep them at the start
      else {
        maze_x = 3;
        maze_y = 0;
      }
    } 
    // Case: Enter character
    else if (ch == KEY_ENTER || ch == '\n') {
      // When the user presses enter, report new input

      // Shift to the "next" field (same field) to update the buffer
      form_driver(input_form, REQ_NEXT_FIELD);

      // Get a pointer to the start of the input buffer
      char* buffer = field_buffer(input_fields[0], 0);

      // Get a pointer to the end of the input buffer
      char* buffer_end = buffer + strlen(buffer) - 1;

      // Seek backward until we find a non-space character in the buffer
      while (buffer_end[-1] == ' ' && buffer_end >= buffer) {
        buffer_end--;
      }

      // Compute the length of the input buffer
      int buffer_len = buffer_end - buffer;

      // If there's a message, handle it
      if (buffer_len > 0) {
        // Copy the message string out so it can be null-terminated
        char message[buffer_len + 1];
        memcpy(message, buffer, buffer_len);
        message[buffer_len] = '\0';

        // Run the callback function provided to ui_init
        input_callback(message);

        // Clear the input field, but only if the UI didn't exit
        if (ui_running) form_driver(input_form, REQ_CLR_FIELD);
      }
    } 
    else {
      // Report normal input characters to the input field
      form_driver(input_form, ch);
    }

    // Unlock the UI
    pthread_mutex_unlock(&ui_lock);
  }
} // ui_run

/**
 * Add a new message to the user interface's display pane.
 *
 * \param username  The username that should appear before the message. The UI
 *                  code will copy the string out of message, so it is safe to
 *                  reuse the memory pointed to by message after this function.
 *
 * \param message   The string that should be added to the display pane. As with
 *                  the username, the UI code will copy the string passed in.
 */
void ui_display(const char* username, const char* message) {
  // Lock the UI
  Pthread_mutex_lock(&ui_lock);

  // Don't do anything if the UI is not running
  if (ui_running) {
    // Which form are we using?
    FORM* used_form;
    if (strcmp(username,"Narrator") == 0) {
      used_form = narrative_form;
    }
    else {
      used_form = display_form;
    }

    // Add a newline
    form_driver(used_form, REQ_NEW_LINE);

    // Get the username
    const char* c = username;

    // Display the username if the username is not "Narrator"
    if (strcmp(username, "Narrator") != 0) {
      while (*c != '\0') {
        form_driver(used_form, *c);
        c++;
      }
      form_driver(used_form, ':');
      form_driver(used_form, ' ');
    }

    // Copy the message over to the field
    c = message;
    while (*c != '\0') {
      form_driver(used_form, *c);
      c++;
    } 
    // If the username is "Narrator", put an extra newline for readability
    if (strcmp(username,"Narrator") == 0) { 
      form_driver(used_form, REQ_NEW_LINE);
    }
  }
  // Default 
  else {
    printf("%s: %s\n", username, message);
  }

  // Unlock the UI
  Pthread_mutex_unlock(&ui_lock);
} // ui_display

/**
 * Run the maze in the UI
 * 
 * \param player  An int indicating which player is calling the function
 */
void ui_maze(int player) {
  // Store which player is calling the UI
  stored_player = player;

  // Set maze_running to true
  Pthread_mutex_lock(&maze_lock);
  maze_running = true;
  Pthread_mutex_unlock(&maze_lock);

  if (ui_running) {
    // Lock the UI
    Pthread_mutex_lock(&ui_lock);
    // PLAYER TWO // 
    if (player == 2) {
      // Print the maze
      for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++){
          form_driver(game_form, maze[y][x]);
        }
        form_driver(game_form, REQ_NEW_LINE);
      }
    }
    // PLAYER ONE // 
    else if (player == 1) {
      // If the player hit a wall, set them back to the beginning
      if (maze[maze_y][maze_x] == '*') {
        maze_x = 3;
        maze_y = 0;
      }
      // If the player is at the end, the maze is not running
      else if (maze[maze_y][maze_x] == 'E') {
        Pthread_mutex_lock(&maze_lock);
        maze_running = false;
        Pthread_mutex_unlock(&maze_lock);
      }
      // Now display the borders of the maze and the player's current position
      for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
          // Print the player
          if (y == maze_y && x == maze_x) {
            form_driver(game_form, '@');
          }
          // Display the vertical border
          else if (y == 0 || y == SIZE -1) {
            form_driver(game_form, maze[y][x]);
          }
          // Display the horizontal border
          else if (x == 0 || x == SIZE -1){
            form_driver(game_form, maze[y][x]);
          }
          // Everywhere else, print a space
          else {
            form_driver(game_form, ' ');
          }
        }
        // Print a newline
        form_driver(game_form, REQ_NEW_LINE);
      }
    }
  }
  // Unlock the UI
  Pthread_mutex_unlock(&ui_lock);
} // ui_maze


void ui_door() {
  stored_player = 2;
  
 // Set maze_running to true
  Pthread_mutex_lock(&door_lock);
  door_running = true;
  Pthread_mutex_unlock(&door_lock);

if (ui_running) {
    // Print the maze
    Pthread_mutex_lock(&ui_lock);
    for (int y = 0; y < SIZE; y++) {
      for (int x = 0; x < SIZE; x++){
        if (door[y][x] <= '3' && door[y][x] >= '0') {

          char ch = nums[(int) (door[y][x] - '0')];
          form_driver(game_form, ch);
        }
        else {
          form_driver(game_form, door[y][x]);
        } 
      }
      form_driver(game_form, REQ_NEW_LINE);
    }

    Pthread_mutex_unlock(&ui_lock);
  }
} // ui_door

/**
 * Stop the user interface and clean up.
 */
void ui_exit() {
  // Block access to the UI
  Pthread_mutex_lock(&ui_lock);

  // The UI is not running
  ui_running = false;

  // Clean up
  unpost_form(display_form);
  unpost_form(input_form);
  unpost_form(game_form);
  unpost_form(narrative_form);
  free_form(game_form);
  free_form(display_form);
  free_form(input_form);
  free_form(narrative_form);
  free_field(game_fields[0]);
  free_field(display_fields[0]);
  free_field(input_fields[0]);
  free_field(narrative_fields[0]);
  endwin();

  // Unlock the UI
  Pthread_mutex_unlock(&ui_lock);

  // Free the maze
  for (int i = 0; i < SIZE; i++) {
    free(maze[i]);
    free(door[i]);
  }
  free(maze);
  free(door);
} // ui_exit