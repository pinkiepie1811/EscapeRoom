#include "mazegame.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <form.h>



char** readMaze(){
    char** game_maze = (char**)malloc(sizeof(char*) * SIZE);
    for (int i = 0; i < SIZE; i++){
        game_maze[i] = (char*)malloc(SIZE);
    }
    FILE * mazefile = fopen("maze.txt", "r");
    if (mazefile == NULL) {
        printf("Failed to read maze");
    }

    char cell;
    for (int row = 0; row < SIZE; row ++){
        for (int col = 0; col < SIZE; col ++){
            if (fscanf(mazefile, "%c", &cell) != 1){
                perror("Failed to read cell");
            }
            game_maze[row][col] = cell;
        }
        fscanf(mazefile, "%c", &cell);
    }
    return game_maze;
}
/**
int main(void){
    FILE * inputfile = fopen("maze.txt", "r");
    if (inputfile == NULL) {
        printf("died");
    }
    readMaze(inputfile);
    initscr();
    cbreak();
    for (int y = 0; y < SIZE; y++){
        for (int x = 0; x < SIZE; x++){
            mvaddch(y, x, maze[y][x] | A_BOLD);
        }
    }
    refresh();
    return 0;
}
*/