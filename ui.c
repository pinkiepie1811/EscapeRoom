// Modified from Peer-to-Peer Chat lab
#include "ui.h"
#include "game.h"

#include <form.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// The height of the input field in the user interface
#define INPUT_HEIGHT 3

// The timeout for input
#define INPUT_TIMEOUT_MS 10

// Maze starting positions:
#define START_X 3;
#define START_Y 0;

// Time for players to escape in seconds
#define TIME_LIMIT 900

#define START_X_BOSS 10
#define START_Y_BOSS 18

#define NUM_ATTACKS 10

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

// The fields array for the time form
FIELD* time_fields[2];

// The form that holds the time field
FORM* time_form;

// The callback function to run on new input
static input_callback_t input_callback;

// -- BOOLEANS -- //
// When true, the UI should continue running
bool ui_running = false;
// When true, the maze game should run
bool maze_running = false;
// When true, the door game should run
bool door_running = false;
// When true, the boss game should run
bool boss_running = false;
// When true, box game should run;
bool box_running = false;

// The handle for the UI thread
pthread_t ui_thread;

// -- LOCKS -- //
// A lock to protect the entire UI
pthread_mutexattr_t ui_lock_attr;
pthread_mutex_t ui_lock;
// Locks to protect our booleans from race conditions
pthread_mutex_t maze_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t door_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t boss_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t boss_health_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t boss_attack_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t box_lock = PTHREAD_MUTEX_INITIALIZER;

// The player that is using the UI (Player One or Player Two)
// Initialized when the maze game is running
int stored_player = -1;

// Position of the player in the games where position is needed (maze & boss)
int player_x = START_X;
int player_y = START_Y;

// -- MAZE GAME -- //
// 2D array holding the maze read in from maze.txt
char** maze;
// Global variables to keep track of where the player in the maze is
int maze_x = START_X;
int maze_y = START_Y;

// -- MATH GAME -- //
// 2D array holding the paper with math equations from paper.txt
char** paper;
// 2D array holding the door with the lock on it from door.txt
char** door;
// Global array of numbers to keep track of the numbers that the user has
// inputted for the lock on the door
char nums[4] = {'0', '0', '0', '0'};
// The current number on the lock on the door that the user is trying to
// increment or decrement
int curr_num = 0;
char** door;

// -- ANAGRAM GAME --
//2D array holding the box read from box1.txt
char** box1;
//2D array holding the box read from box2.txt
char** box2;
//Global pointer to a string to keep track of the the user answer input
char* box_answer;

// -- BOSS --//
// 2D array holding the battlefield + the final boss from boss.txt
char** boss;

// Boss fight
int monster_health = 10;
int health_change = 0;
int attacks[NUM_ATTACKS][2] = { {3, 7},
                                {8, 9},
                                {15, 12},
                                {4, 17},
                                {9, 16},
                                {12, 7},
                                {12, 12},
                                {1, 4},
                                {10,8},
                                {10,12}
                                };

// Position of other player
int stored_p2_x = START_X_BOSS;
int stored_p2_y = START_Y_BOSS;

// -- TIMER -- //
// Saves the game start time
clock_t start_time = -1;

// -- ERROR PROTECTION FOR LOCKS -- //
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

// -- RETURN BOOLEAN FUNCTIONS PROTECTED BY LOCKING -- //
/**
 * Returns the boolean for the maze_running (protected by locks).
 * 
 * \return true or false depending on if the maze is currently running.
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
 * Returns the boolean for the door_running (protected by locks).
 * 
 * \return true or false depending on if the door/math game is currently running.
 */
bool door_running_check() {
  // Make a copy of the boolean
  // Lock to avoid race conditions
  Pthread_mutex_lock(&door_lock);
  bool door = door_running;
  Pthread_mutex_unlock(&door_lock);
  return door;
} // door_running_check

/**
 * Returns the boolean for the box_running (protected by locks).
 * 
 * \return true or false depending on if anagram/box game is currently running.
 */
bool box_running_check(){
  bool box;
  // Make a copy of the boolean
  // Lock to avoid race conditions
  Pthread_mutex_lock(&box_lock);
  box = box_running;
  Pthread_mutex_unlock(&box_lock);
  return box;
} //box_running_check

/**
 * Returns the boolean for the boss_running (protected by locks).
 * 
 * \return true or false depending on if the final boss match is currently running.
 */
bool boss_running_check() {
  // Make a copy of the boolean
  // Lock to avoid race conditions
  Pthread_mutex_lock(&boss_lock);
  bool boss = boss_running;
  Pthread_mutex_unlock(&boss_lock);
  return boss;
} // boss_running_check

// -- UI FUNCTIONS -- //
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
  game_fields[0] = new_field(display_height / 2 - 1, cols / 2 - 1, 0, 0, 0, 0);
  game_fields[1] = NULL;

  // Create the narrative window
  // height, width, start row, start col, overflow buffer lines, buffers
  narrative_fields[0] = new_field(display_height / 2 - 1, cols / 2 - 1, display_height / 2, 0, 0, 0);
  narrative_fields[1] = NULL;

  // Create the larger message display window
  // height, width, start row, start col, overflow buffer lines, buffers
  display_fields[0] = new_field(display_height, cols / 2, 0, cols / 2, 0, 0);
  display_fields[1] = NULL;

  // Create the input field
  // height, width, start row, start col, overflow buffer lines, buffers
  input_fields[0] = new_field(INPUT_HEIGHT, cols / 2, display_height + 1, cols / 2, 0, 0);
  input_fields[1] = NULL;

  // Create the time field
  // height, width, start row, start col, overflow buffer lines, buffers
  time_fields[0] = new_field(INPUT_HEIGHT, cols / 2 - 1, display_height + 1, 0, 0, 0);
  time_fields[1] = NULL;

  // Grow the display & narrative field buffers as needed
  field_opts_off(display_fields[0], O_STATIC);
  field_opts_off(narrative_fields[0], O_STATIC);

  // Don't advance to the next field automatically when using the input field
  field_opts_off(input_fields[0], O_AUTOSKIP);

  // Turn off word wrap (nice, but causes other problems)
  field_opts_off(input_fields[0], O_WRAP);
  field_opts_off(display_fields[0], O_WRAP);
  field_opts_off(game_fields[0], O_WRAP);
  // TODO: field_opts_off(narrative_fields[0], O_WRAP);

  // Create the forms
  game_form = new_form(game_fields);
  display_form = new_form(display_fields);
  input_form = new_form(input_fields);
  narrative_form = new_form(narrative_fields);
  time_form = new_form(time_fields);

  // Display the forms
  post_form(game_form);
  post_form(display_form);
  post_form(input_form);
  post_form(narrative_form);
  post_form(time_form);
  refresh();

  // Draw a horizontal split
  for (int i = 0; i < cols; i++) {
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
  maze = read_game("game-boards/maze.txt");
  door = read_game("game-boards/door.txt");
  paper = read_game("game-boards/paper.txt");
  box1 = read_game("game-boards/box1.txt");
  box2 = read_game("game-boards/box2.txt");
  boss = read_game("game-boards/boss.txt");

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
    // Case: If the input is arrow keys and the maze or final boss is running currently
    else if (((ch == KEY_DOWN) || (ch == KEY_UP) || (ch == KEY_RIGHT) || (ch == KEY_LEFT)) && ((maze_running_check() && stored_player == 1) || boss_running_check())) {
      // Adjust the position of the player in the maze accordingly
      if (ch == KEY_RIGHT) {
        player_x++;
      } 
      else if (ch == KEY_LEFT) {
        player_x--;
      } 
      else if (ch == KEY_DOWN) {
        player_y++;
      } 
      else if (ch == KEY_UP) {
        player_y--;
      }
      // -- FOR MAZE -- //
      // If the player is trying to move within the maze, clear the previous maze and print the new maze with
      // updated coordinates for the paper
      if (maze_running_check()) {
        if ((player_x >= 0) && (player_x < SIZE) && (player_y >= 0) && (player_y < SIZE)) {
          form_driver(game_form, REQ_CLR_FIELD);
          ui_maze(1);
        }
      // Otherwise, the player is out of bounds; keep them at the start
        else {
          player_x = START_X;
          player_y = START_Y;
        }
      }
      // -- FOR BOSS -- //
      else if (boss_running_check()) {
        ui_boss(stored_p2_x, stored_p2_y);
      }
    } 
    // Case: If the input is arrow keys and the door is running currently
    else if (((ch == KEY_DOWN) || (ch == KEY_UP) || (ch == KEY_RIGHT) || (ch == KEY_LEFT)) && door_running_check()) {
      // Adjust the which number the player is changing if keys are right or left arrow
      if (ch == KEY_RIGHT) {
        curr_num++;
        // Wrap around
        if (curr_num > 3) {
          curr_num = 0;
        }
      } 
      else if (ch == KEY_LEFT) {
        curr_num--;
        // Wrap arround
        if (curr_num < 0) {
          curr_num = 3;
        }
      }
      // Adjust the number we are currently on
      // Up = increment number; down = decrement number
      else if (ch == KEY_DOWN) {
        nums[curr_num]--;
        // Wrap around
        if (nums[curr_num] < '0') {
          nums[curr_num] = '9';
        }
      } 
      else if (ch == KEY_UP) {
        nums[curr_num]++;
        // Wrap around
        if (nums[curr_num] > '9') {
          nums[curr_num] = '0';
        }
      }
      ui_door();
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
        
        if (message[0] == '[' && message[buffer_len -1] == ']'){
          box_answer = (char*)malloc(buffer_len+1);
          strncpy(box_answer, message, buffer_len+1);
          ui_box(stored_player);
        }
        else {
           // Run the callback function provided to ui_init
          input_callback(message);
        }

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
 *                  If the username is "Narrator", the username will not appear
 *                  before the message. 
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
      // If username is "Narrator", then we are using the narrative form
      used_form = narrative_form;
    }
    else {
      // Otherwise, we are using the display form
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

void narrate_display(const char* message) {
  ui_display("Narrator",message);
  sleep(3);
}

/**
 * Keeps track of the timer telling the players how much longer they
 * have to escape. 
 */
int ui_time() {
  // Only start the timer is the UI is running
  if (ui_running) {

    // Lock the UI
    Pthread_mutex_lock(&ui_lock);

    // Clear the previous time
    form_driver(time_form, REQ_CLR_FIELD);
    // Go to next line so that the time is in the middle of the form
    form_driver(time_form, REQ_NEW_LINE);

    // If start_time is not initalized yet, get the current time with clock()
    if (start_time == -1) {
      start_time = clock();
    }
    // Get how much time the players have left
    long time = TIME_LIMIT - ((clock() - start_time) / CLOCKS_PER_SEC);
    // Parse time into minutes and seconds
    int min = time / 60;
    int sec = time % 60;

    // Local variable to hold the formatted time that we will print
    // Maximum of 6 characters
    char time_str[6];
    sprintf(time_str, "%d:%02d", min, sec);
    // Print the time
    for (int i = 0; i < 5; i++) {
      form_driver(time_form, time_str[i]);
    }

    // Unlock the UI
    Pthread_mutex_unlock(&ui_lock);
    if (time <= 0) {
      return -1;
    }
  }
  return 1;
} // ui_time

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
  if (stored_player == 1) maze_running = true;
  Pthread_mutex_unlock(&maze_lock);

  if (ui_running) {
    // Lock the UI
    Pthread_mutex_lock(&ui_lock);
    // PLAYER TWO // 
    if (player == 2) {
      form_driver(game_form, REQ_CLR_FIELD);
      // Print the maze
      for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++){
          if (maze[y][x] == '*') {
            form_driver(game_form, 'X');
          }
          else {form_driver(game_form, maze[y][x]);}
        }
        form_driver(game_form, REQ_NEW_LINE);
      }
      ui_display("Narrator", "There's a map on the wall! Maybe you can use it to help your friend?");
    }
    // PLAYER ONE // 
    else if (player == 1) {
      // If the player hit a wall, set them back to the beginning
      if (maze[player_y][player_x] == '*') {
        player_x = 3;
        player_y = 0;
        ui_display("Narrator", "You hit a wall! Your consciousness fades, and you wake up... back at the start.");
      }
      // If the player is at the end, the maze is not running
      else if (maze[player_y][player_x] == 'E') {
        Pthread_mutex_lock(&maze_lock);
        maze_running = false;
        Pthread_mutex_unlock(&maze_lock);
      }
      // Now display the borders of the maze and the player's current position
      for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
          // Print the player
          if (y == player_y && x == player_x) {
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

/**
 *Run the door/math game in UI
*/
void ui_door() {
  stored_player = 2;
  
 // Set door_running to true
  Pthread_mutex_lock(&door_lock);
  door_running = true;
  Pthread_mutex_unlock(&door_lock);
  // CHANGE
  char solution[4] = {'0', '5', '4','2'};

  int solved = true;
    for (int i = 0; i < 4; i++) {
      if (solution[i] != nums[i]) solved = false;
    }
  if (solved) {
    Pthread_mutex_lock(&door_lock);
    door_running = false;
    Pthread_mutex_unlock(&door_lock);
  }

if (ui_running) {
    // Print the door
    Pthread_mutex_lock(&ui_lock);
    form_driver(game_form, REQ_CLR_FIELD);
    
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
 * Display paper/math game in UI
 */
void ui_paper() {
  if (ui_running) {
    Pthread_mutex_lock(&ui_lock);
    form_driver(game_form, REQ_CLR_FIELD);
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++){
          form_driver(game_form, paper[y][x]);
        }
        form_driver(game_form, REQ_NEW_LINE);
    }
    Pthread_mutex_unlock(&ui_lock);
  }
} // ui_paper

/**
 * Run box/anagram game in UI
 * \param player_id the user calling this function
*/
void ui_box(int player_id){
  stored_player = player_id; //store playerID in global

  Pthread_mutex_lock(&box_lock);
  box_running = true; //change status of box_running boolean
  Pthread_mutex_unlock(&box_lock);

  char* solution_1 = "[pmosera]";
  char* solution_2 = "[charliecurtsinger]";
  
  bool solve = false;
  if(box_answer != NULL){
  if (player_id == 1 && strcmp(box_answer, solution_1) == 0){
      solve = true;
    }
  else if (player_id == 2 && (strcmp(box_answer, solution_2) == 0))
      solve = true;
  }//when there is user input, check with the solutions

  if (solve){
      Pthread_mutex_lock(&box_lock);
      box_running = false;
      Pthread_mutex_unlock(&box_lock);
  } //when anagram solved, change status of box_running boolean

  if(ui_running){
    Pthread_mutex_lock(&ui_lock);
    form_driver(game_form, REQ_CLR_FIELD);
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++){
          if (player_id == 1)
            form_driver(game_form, box1[y][x]);
          else 
            form_driver(game_form, box2[y][x]);
        }
        form_driver(game_form, REQ_NEW_LINE);
    }
    Pthread_mutex_unlock(&ui_lock);
  }// display box
} // ui_box

/**
 * @brief update the position of the other player
 * 
 * @param p2_x current x position of the other player
 * @param p2_y current y position of the other player
 */
void change_p2_posx(int p2_x) {
  stored_p2_x=p2_x;
}
void change_p2_posy(int p2_y) {
  stored_p2_y=p2_y;
}

/**
 * Return the current position of the player using the UI
*/
int get_pos_x() {
  return player_x;
}
int get_pos_y() {
  return player_y;
}

/**
 * @brief TODO
 * 
 * @return int 
 */
int change_damage() {
  Pthread_mutex_lock(&boss_health_lock);
  int damage = health_change;
  health_change = 0;
  Pthread_mutex_unlock(&boss_health_lock);
  return damage;
}

void do_damage(int dam) {
  Pthread_mutex_lock(&boss_health_lock);
  monster_health-=dam;
  Pthread_mutex_unlock(&boss_health_lock);
}

/**
 * @brief TODO
 * 
 */
void boss_attack(){
  srand(time(NULL));
    for (int i = 0; i < NUM_ATTACKS; i++) {
      Pthread_mutex_lock(&boss_attack_lock);
      attacks[i][1]++;
      if (attacks[i][1] > 18) {
        attacks[i][0] = (rand() % 18) + 1;
         attacks[i][1] = 7;
      }
      Pthread_mutex_unlock(&boss_attack_lock);
    }
}

void ui_boss() {
  if (!boss_running_check()) {
    player_x = START_X_BOSS;
    player_y = START_Y_BOSS;
    Pthread_mutex_lock(&boss_lock);
    boss_running = true;
    Pthread_mutex_unlock(&boss_lock);
  }

  if (ui_running) {
    Pthread_mutex_lock(&ui_lock);

    form_driver(game_form, REQ_CLR_FIELD);

    for (int a = 0; a < NUM_ATTACKS; a++) {
      Pthread_mutex_lock(&boss_attack_lock);
      if ((attacks[a][0] == player_x) && (attacks[a][1] == player_y)) {
        player_x = START_X_BOSS;
        player_y = START_Y_BOSS;
      }
      Pthread_mutex_unlock(&boss_attack_lock);
    }

    if ((boss[player_y][player_x] == '|') || (boss[player_y][player_x] == '*')) {
      player_x = START_X_BOSS;
      player_y = START_Y_BOSS;
    }

    else if (player_y <= 6) {
      Pthread_mutex_lock(&boss_health_lock);
      monster_health--;
      health_change++;
      Pthread_mutex_unlock(&boss_health_lock);
      ui_display("Narrator", "You attacked the monster! He throws you backwards!");
      player_y = START_Y_BOSS;
    }
    for (int y = 0; y < SIZE; y++) {
      for (int x = 0; x < SIZE; x++){
        if (y == player_y && x == player_x) {
          form_driver(game_form, '1');
        }
        else if (stored_p2_x == x && stored_p2_y == y) {
          form_driver(game_form, '2');
        }
        else {
          bool attack = false;
          Pthread_mutex_lock(&boss_attack_lock);
          for (int a = 0; a < NUM_ATTACKS; a++) {
            
            if ((attacks[a][0] == x) && (attacks[a][1] == y)) {
            form_driver(game_form, '|');
            attack = true;
            }
            
          }
          Pthread_mutex_unlock(&boss_attack_lock);
          if (!attack) form_driver(game_form, boss[y][x]);
        }
        }
      form_driver(game_form, REQ_NEW_LINE);
      }
    
    
    Pthread_mutex_unlock(&ui_lock);
  }
  Pthread_mutex_lock(&boss_health_lock);
  if (monster_health <= 0) {
    Pthread_mutex_lock(&boss_lock);
    boss_running = false;
    Pthread_mutex_unlock(&boss_lock);
  }
  Pthread_mutex_unlock(&boss_health_lock);
} // ui_boss

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
    free(paper[i]);
    free(box1[i]);
    free(box2[i]);
    free(boss[i]);
  }
  free(maze);
  free(door);
  free(paper);
  free(box1);
  free(box2);
  free(boss);
  free(box_answer);
} // ui_exit
