#include "ui.h"

#include <form.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

char maze[10][10] = {"##########", 
                     "##   ### #",
                     "##########",
                     "##########",
                     "### ######",
                     "#        #",
                     "##########",
                     "##########", "##########", "##########" };

int main(void){
    initscr();
    cbreak();
    for (int y = 0; y < 10; y++){
        for (int x = 0; x < 10; x++){
            mvaddch(y, x, maze[y][x] | A_BOLD);
        }
    }
    refresh();
    return 0;
}