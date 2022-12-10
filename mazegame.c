#include "mazegame.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <form.h>

/**
 * Reads in the maze from 'maze.txt' and stores it into an array of strings.
 * 
 * \return An array of strings holding the maze
 */
char** readMaze() {
    /* -- MALLOC MEMORY -- */
    // Malloc space for a 2D array (array of strings)
    char** game_maze = (char**)malloc(sizeof(char*) * SIZE);
    // Malloc space for each string in the array
    for (int i = 0; i < SIZE; i++){
       game_maze[i] = (char*)malloc(SIZE);
    }

    /* -- OPEN THE FILE-- */
    FILE* mazefile = fopen("maze.txt", "r");
    if (mazefile == NULL) {
        perror("Failed to read maze from maze.txt");
        exit(EXIT_FAILURE);
    }

    /* -- READ IN FROM THE FILE -- */
    // Variable to hold one character from 'maze.txt'
    // We read 'maze.txt' character by character
    char cell;
    for (int row = 0; row < SIZE; row ++){
        for (int col = 0; col < SIZE; col ++){
            // Read in one character at a time
            if (fscanf(mazefile, "%c", &cell) != 1){
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