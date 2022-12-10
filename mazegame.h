// Use if to avoid including this file more than once
#if !defined(MAZEGAME_H)
#define MAZEGAME_H

// Length (and width; it's a square) of the maze
#define SIZE 20

/**
 * Reads in the maze from 'maze.txt' and stores it into an array of strings.
 * 
 * \return An array of strings holding the maze
 */
char** readMaze();

#endif