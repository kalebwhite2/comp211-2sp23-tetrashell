#include <curses.h>
#include <ncurses.h>
//This is a great way to do error checking, so why would I do it differently?
#define checkCursesError(val) internalCheckCursesError(val, #val)

void gen_hex(char** randhex, int length); 
void internalCheckCursesError(int value, const char *func);

void train() {
   /* INIT */
   if (initscr() == NULL) {
       printf("failed to init screen in train!\n");
       exit(EXIT_FAILURE);
   }
   clear();
   start_color();
   checkCursesError(noecho());
   refresh();
    
   /* INIT SCREEN */
   int width = 60, height = 30;
   int starty = (LINES - height) / 2, startx = (COLS - width) / 2;
   WINDOW* game_window = newwin(height, width, starty, startx);
   wborder(game_window, '|', '|', '-', '-', '+', '+', '+', '+');
   mvwprintw(game_window, 5, 5, "TRAINING");
   wrefresh(game_window);

   /* INIT MENU */
   int width2 = 16, height2 = 8, offset = 4;
   int starty2 = getmaxy(game_window) - 10;
   int startx1 = startx + offset, startx2 = startx + width2 + 2 * offset, startx3 = startx2 + width2 + offset;
   WINDOW* t = newwin(height2, width2, starty2, startx1); 
   checkCursesError(wborder(t, '|', '|', '-', '-', '+', '+', '+', '+'));
   wrefresh(t);
   WINDOW* change_difficulty = newwin(height2, width2, starty2, startx2); 
   checkCursesError(wborder(change_difficulty, '|', '|', '-', '-', '+', '+', '+', '+'));
   wrefresh(change_difficulty);
   WINDOW* exit = newwin(height2, width2, starty2, startx3); 
   checkCursesError(wborder(exit, '|', '|', '-', '-', '+', '+', '+', '+'));
   wrefresh(exit);
    
   //WINDOW* menu[3] = {t, change_difficulty, exit};



   refresh();
   wgetch(game_window);

   /* EXIT */ 
   endwin();
}

void gen_hex(char** randhex, int length) {
    for (int i = 0; i < length; i++) {
        sprintf(*randhex + i, "%x", rand() % 16);
    }
    *randhex[length] = '\0';
}

void internalCheckCursesError(int value, const char *func) {
    if (value != ERR) {
        return;
    }
    clear();
    endwin();
    printf("failure in train at %s\n", func);
    exit(EXIT_FAILURE);
}



