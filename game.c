#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Reads in the game board from a text file and stores it into an array of strings to display
 * to the game window.
 * 
 * \param file  The string containing the text file to open and read. 
 * 
 * \return      An array of strings holding the game board (must be freed later).
 */
char** read_game(char* file) {
  // -- MALLOC MEMORY -- //
  // Malloc space for a 2D array (array of strings)
  char** game_maze = (char**)malloc(sizeof(char*) * SIZE);
  // Malloc space for each string in the array
  for (int i = 0; i < SIZE; i++) {
    game_maze[i] = (char*)malloc(SIZE);
  }

  // -- OPEN THE FILE TO READ -- //
  FILE* mazefile = fopen(file, "r");
  if (mazefile == NULL) {
    perror("Failed to read board game from text file");
    exit(EXIT_FAILURE);
  }

  // -- READ IN FROM THE FILE -- //
  // 'cell' is the variable to hold one character from the text file we read
  // We read the text file character by character
  char cell;
  for (int row = 0; row < SIZE; row++) {
    for (int col = 0; col < SIZE; col++) {
      // Read in one character at a time
      if (fscanf(mazefile, "%c", &cell) != 1) {
        perror("Failed to read character from maze.txt");
        exit(EXIT_FAILURE);
      }
      // Put the character in our array
      game_maze[row][col] = cell;
    }
    // Read the newline in the file but don't do anything with it
    fscanf(mazefile, "%c", &cell);
  }

  // Return the array of strings that holds the maze from maze.txt
  return game_maze;
} // readMaze