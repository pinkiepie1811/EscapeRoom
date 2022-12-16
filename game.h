#if !defined(GAME_H)
#define GAME_H

// Length and width (it's a square) of the maze
#define SIZE 20

/**
 * Reads in the game board from a text file and stores it into an array of strings to display
 * to the game window.
 * 
 * \param file  The string containing the text file to open and read. 
 * 
 * \return      An array of strings holding the game board (must be freed later).
 */
char** read_game(char* file);

#endif