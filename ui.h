// Modified from Peer-to-Peer Chat lab
#if !defined(UI_H)
#define UI_H

#include <stdbool.h>

/**
 * The type of a callback function run by the user interface every time there is
 * a new message provided in the input pane. The parameter points to memory that
 * will be reused after the callback returns, so if you need to retain the
 * string after the callback you must copy it.
 */
typedef void (*input_callback_t)(const char*);

/**
 * Initialize the user interface and set up a callback function that should be
 * called every time there is a new message to send.
 *
 * \param callback  A function that should run every time there is new input.
 *                  The string passed to the callback should be copied if you
 *                  need to retain it after the callback function returns.
 */
void ui_init(input_callback_t callback);

/**
 * Returns the boolean for the maze_running
 * 
 * \return true or false depending on if the maze is currently running
 */
bool maze_running_check();

/**
 * Returns the boolean for the door_running
 * 
 * \return true or false depending on if the door is currently running
 */
bool door_running_check();

/**
 * 
*/
bool box_running_check();

/**
 * Run the main UI loop. This function will only return the UI is exiting.
 */
void ui_run();

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
void ui_display(const char* username, const char* message);

void narrate_display(const char* message);

/**
 * Run the main UI loop. This function will only return the UI is exiting.
 */
void ui_run();

/**
 * Keeps track of the timer telling the players how much longer they
 * have to escape. 
 */
int ui_time();

/**
 * Run the maze in the UI.
 * 
 * \param player  An int indicating which player is calling the function (Player One or Player Two).
 */
void ui_maze(int player);

/**
 * Returns the boolean for the maze_running (protected by locks).
 * 
 * \return true or false depending on if the maze is currently running.
 */
bool maze_running_check();

/** 
 * Runs the math game (lock on the door) in the UI.
 * Called by Player Two.
 */
void ui_door();

/**
 * Returns the boolean for the door_running (protected by locks).
 * 
 * \return true or false depending on if the door/math game is currently running.
 */
bool door_running_check();

/**
 * Runs the math game (paper with math equations) in the UI.
 * Called by Player One.
 */
void ui_paper();

void ui_box(int player_id);

/**
 * @brief TODO
 * 
 * @param p2_x 
 * @param p2_y 
 */
void change_p2_posx(int p2_x);
void change_p2_posy(int p2_y);

int get_pos_x();
int get_pos_y();
/**
 * @brief TODO
 * 
 * @return int 
 */
int change_damage();

/**
 * @brief TODO
 * 
 */
void do_damage(int dam);

/**
 * @brief TODO
 * 
 */
void boss_attack();

/** 
 * Runs the final boss simulator.
 * TODO: FINISH THIS
 */
void ui_boss();


/**
 * Returns the boolean for the boss_running (protected by locks).
 * 
 * \return true or false depending on if the final boss match is currently running.
 */
bool boss_running_check();

/**
 * Stop the user interface and clean up.
 */
void ui_exit();

#endif
