// Use if to avoid including this file more than once
#if !defined(GAME_H)
#define GAME_H

// Length (and width; it's a square) of the maze
#define SIZE 20

/**
 * Reads in the maze from a text file and stores it into an array of strings.
 * 
 * \return An array of strings holding the maze
 */
char** read_game();

#endif