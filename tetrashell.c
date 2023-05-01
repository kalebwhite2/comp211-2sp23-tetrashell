#include <errno.h>
#include <fcntl.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "tetris.h"
#include "train.h"

/* HELPER FUNCTIONS */
char* read_line(TetrisGameState game, char* pathname);
char** parse_args(char* args);
char** parse_args_changes(char* args, char* path_name);
char** parse_args_ranked(char* args, char* path_name);
char* welcome();
int verify_save(TetrisGameState* game, char** pathname, FILE** fp);
void print_image(FILE* fptr);

/* HELPER STRUCTS */
typedef struct State {
  int i;
  TetrisGameState g;
} State;


/* ERROR CHECKERS */
void check_buffer(void* buffer, char* output_text);
void check_0(int to_check, char* output_text);
void check_EOF(int to_check, char* output_text); 
#define check_neg(to_check, output_text) check_EOF(to_check, output_text)

/* COMMANDS */
void visualize(TetrisGameState state);
void play(char* pathname);
int recover(char** args, TetrisGameState* game, char** pathname);
void modify(char** args, TetrisGameState* game, char** pathname);
int switch_func(char** args, TetrisGameState* game, char** pathname);
void help(char** args);
void rank(char* arg, char** args, char** pathname);

/* MAIN */
int main() {
  // change text color
  printf("\e[38;2;255;255;255m\n");

  char* arg;
  char** args;
  char** args2;
  pid_t pid;
  /* FIND SAVE FILE */
  char* pathname = welcome();

  /* CHECK SAVE */
  int c;
  size_t pathsize = 256;
  TetrisGameState game;
  
  FILE* fp;
  while (!verify_save(&game, &pathname, &fp)) {
    printf("\e[38;2;255;60;0mFINDING, READING, OR SAVING FROM FILE FAILED.\n"
           "PLEASE ENTER ANOTHER PATH, OR EXIT TO QUIT. \e[38;2;255;255;255m\n");

    c = getline(&pathname, &pathsize, stdin);
    //getline returns < 0 on failure
    check_neg(c, "getline failed in verify_save!");
    
    pathname[c - 1] = 0;
    if (!strcmp(pathname, "exit") || !strcmp(pathname, "exit")) {
        free(pathname);
        return EXIT_SUCCESS;
    }
  }
  

  /* INIT SAVE STATES */
  State state0, state1, state2;
  state0.i = 0;
  state1.i = 0;
  state2.i = 0;
  State prev_game_states[3] = {state0, state1, state2};
  int modify_called = 0;
  

  /* MAIN LOOP */
  while (strcmp((arg = read_line(game, pathname)), "exit") && strcmp(arg, "e")
         && strcmp(arg, "ex") && strcmp(arg, "exi")) {
    // dup args because strtok will change them otherwise
    char* arg_copy = strdup(arg);
    args = parse_args(arg_copy);
    free(arg_copy);
    if (!strcmp(args[0], "recover") || !strcmp(args[0], "re") ||
        !strcmp(args[0], "rec") || !strcmp(args[0], "reco") ||
        !strcmp(args[0], "recov") || !strcmp(args[0], "recove")) {
      // recover returns 1 on change to save state, so:
      if (recover(args, &game, &pathname)) {
        if (prev_game_states[0].i) {
          prev_game_states[0].i = 0;
        }
        if (prev_game_states[1].i) {
          prev_game_states[1].i = 0;
        }
        if (prev_game_states[2].i) {
          prev_game_states[2].i = 0;
        }
        modify_called = 0;
      }
    }

    //MODIFY
    else if (!strcmp(args[0], "modify") || !strcmp(args[0], "m") ||
        !strcmp(args[0], "mo") || !strcmp(args[0], "mod") ||
        !strcmp(args[0], "modi") || !strcmp(args[0], "modif")) {
      // edit array of previous game states
      if (!modify_called) {
        modify_called = 1;
      } else {
        if (prev_game_states[2].i) {
          // move everything back an index
          memcpy(&prev_game_states[2].g, &prev_game_states[1].g,
                 sizeof(TetrisGameState));
          memcpy(&prev_game_states[1].g, &prev_game_states[0].g,
                 sizeof(TetrisGameState));
          prev_game_states[0].g = game;
        } else if (prev_game_states[1].i) {
          // move everything back an index
          memcpy(&prev_game_states[2].g, &prev_game_states[1].g,
                 sizeof(TetrisGameState));
          memcpy(&prev_game_states[1].g, &prev_game_states[0].g,
                 sizeof(TetrisGameState));
          prev_game_states[0].g = game;
          // set 2's i value
          prev_game_states[2].i = 1;
        } else {
          // move everything back an index
          memcpy(&prev_game_states[1].g, &prev_game_states[0].g,
                 sizeof(TetrisGameState));
          // set 1's i value
          prev_game_states[0].g = game;
          prev_game_states[1].i = 1;
        }
      }

      // call to modify
      args2 = parse_args_changes(arg, pathname);
      modify(args2, &game, &pathname);
      for (int i = 0; i < sizeof(args2) / sizeof(char*); i++) {
          free(args2[i]);
      }
      free(args2);
    }

    //HELP
    else if (!strcmp(args[0], "help") || !strcmp(args[0], "h") ||
        !strcmp(args[0], "he") || !strcmp(args[0], "hel")) {
      help(args);
    }

    //INFO
    else if (!strcmp(args[0], "info") || !strcmp(args[0], "i") ||
        !strcmp(args[0], "in") || !strcmp(args[0], "inf")) {
      printf("Current savefile: %s \n", pathname);
      printf("Score: %d \n", game.score);
      printf("Lines: %d \n", game.lines);
    }

    //SWITCH
    else if (!strcmp(args[0], "switch") || !strcmp(args[0], "s") ||
        !strcmp(args[0], "sw") || !strcmp(args[0], "swi") ||
        !strcmp(args[0], "swit") || !strcmp(args[0], "switc")) {
      // switch returns 1 on success
      if (switch_func(args, &game, &pathname)) {
        // reset prev_game_states
        if (prev_game_states[0].i) {
          prev_game_states[0].i = 0;
        }
        if (prev_game_states[1].i) {
          prev_game_states[1].i = 0;
        }
        if (prev_game_states[2].i) {
          prev_game_states[2].i = 0;
        }
        modify_called = 0;
      }
    }

    //CHECK
    else if (!strcmp(args[0], "check") || !strcmp(args[0], "c") ||
        !strcmp(args[0], "ch") || !strcmp(args[0], "che") ||
        !strcmp(args[0], "chec")) {
      args2 = parse_args_changes(arg, pathname);
      pid_t pid = fork();
      check_neg((int) pid, "fork failed at check!");
      if (pid == 0) {
        execve("/playpen/a5/check", args2, NULL);
      } else {
        int ret;
        wait(&ret);
      }

      for (int i = 0; i < sizeof(args2) / sizeof(char*); i++) {
          free(args2[i]);
      }
      free(args2);
    }

    //RANK
    else if (!strcmp(args[0], "rank") || !strcmp(args[0], "ra") ||
        !strcmp(args[0], "ran")) {
      rank(arg, args, &pathname);
    }

    //UNDO
    else if (!strcmp(args[0], "undo") || !strcmp(args[0], "u") ||
        !strcmp(args[0], "un") || !strcmp(args[0], "und")) {
      if (prev_game_states[2].i) {
        memcpy(&game, &prev_game_states[0].g, sizeof(TetrisGameState));
        memcpy(&prev_game_states[0].g, &prev_game_states[1].g,
               sizeof(TetrisGameState));
        memcpy(&prev_game_states[1].g, &prev_game_states[2].g,
               sizeof(TetrisGameState));
        prev_game_states[2].i = 0;
      } else if (prev_game_states[1].i) {
        memcpy(&game, &prev_game_states[0].g, sizeof(TetrisGameState));
        memcpy(&prev_game_states[0].g, &prev_game_states[1].g,
               sizeof(TetrisGameState));
        prev_game_states[1].i = 0;
      } else if (prev_game_states[0].i) {
        memcpy(&game, &prev_game_states[0].g, sizeof(TetrisGameState));
        prev_game_states[0].i = 0;
      } else {
        printf("\e[38;2;255;60;0mNO SAVED STATES.\e[38;2;255;255;255;0m\n");
      }
    }

    //VISUALIZE
    else if (!strcmp(args[0], "visualize") || !strcmp(args[0], "v") ||
        !strcmp(args[0], "vi") || !strcmp(args[0], "vis") ||
        !strcmp(args[0], "visu") || !strcmp(args[0], "visua") ||
        !strcmp(args[0], "visual") || !strcmp(args[0], "visuali") ||
        !strcmp(args[0], "visualiz")) {
      visualize(game);
    }

    //TRAIN
    else if (!strcmp(args[0], "train") || !strcmp(args[0], "tr") ||
        !strcmp(args[0], "tra") || !strcmp(args[0], "trai")) {
      train();
    }

    //PLAY
    else if (!strcmp(args[0], "play")) {
      play(pathname);
      // reopen file to read back in the game stats
      FILE* fp;
      check_buffer(fp = fopen(pathname, "r"), "fopen failed in play!");
      check_0((fread(&game, sizeof(TetrisGameState), 1, fp)), "fread failed in play!");
      check_EOF((fclose(fp)), "fclose failed in play");
      printf(
          "\e[38;2;255;60;0mYOUR RESULTS HAVE BEEN WRITTEN TO YOUR "
          "SAVE.\e[38;2;255;255;255m\n");
    }

    //USER TRIED TO HIT ENTER RIGHT AWAY OR ENTERED EOF RIGHT AWAY TO MESS WITH THE SHELL
    //(readline returned bad_command)
    else if (!strcmp(args[0], "bad_command")) {
     printf("\e[38;2;255;60;0mPLEASE ENTER A COMMAND. DO NOT MESS WITH THE SHELL.\e[38;2;255;255;255m\n");
    }


    //UNRECOGNIZED COMMAND
    else {
      printf("\e[38;2;255;60;0mCOMMAND NOT RECOGNIZED.\e[38;2;255;255;255m\n");
    }
    free(arg);
  }
  for (int i = 0; i < sizeof(args) / sizeof(char*); i++) {
      free(args[i]);
  }
  free(args);
  free(arg);
  free(pathname);
  return EXIT_SUCCESS;
}

char* welcome() {
  // print out the intro text from intro_art.txt
  FILE* imgpath = fopen("intro_art.txt", "r");
  check_buffer(imgpath, "unable to open image");
  print_image(imgpath);
  check_EOF(fclose(imgpath), "fclose failed in welcome");

  printf(
      "WELCOME TO TETRASHELL\nTHE SHELL OF YOUR DREAMS\nENTER THE PATH TO YOUR "
      "QUICKSAVE: \n");

  // getline will automatically resize pathname if necessary
  char* pathname = malloc(256);
  check_buffer(pathname, "allocation to pathname failed!");
  size_t pathsize = 256;

  int c;
  c = getline(&pathname, &pathsize, stdin);
  check_neg(c, "getline failure in welcome"); //getline returns -1 on failure

  // NOTE TO GRADERS: UNSURE HOW TO DO THIS WITHOUT MEMORY LOSS; MINIMIZED BY USE OF TMP
  pathname[strcspn(pathname, "\n")] = 0;
  char* tmp = malloc(strlen(pathname) + 1); 
  strcpy(tmp, pathname);
  free(pathname);
  return tmp;
}

#define MAXIMGLINESIZE 128
void print_image(FILE* imgpath) {
  char str[MAXIMGLINESIZE];

  while (fgets(str, sizeof(str), imgpath) != NULL) {
    printf("%s", str);
  }
}

int verify_save(TetrisGameState* game, char** pathname, FILE** fp) {
  *fp = fopen(*pathname, "r");
  if (*fp == NULL) {
    perror("Error");
    return 0;
  }
  int read_check;
  read_check = fread(game, sizeof(TetrisGameState), 1, *fp);
  if (read_check < 1) {
    perror("Error");
    return 0;
  }
  int close_check;
  close_check = fclose(*fp);
  if (close_check != 0) {
    perror("Error");
    return 0;
  }
  return 1;
}

#define RED "\e[38;2;255;60;0m"
#define WHITE "\e[38;2;255;255;255m"
char* read_line(TetrisGameState game, char* pathname) {
  int line_size = 256, ch, idx = 0, format_idx = 0;
  char* buffer = malloc(sizeof(char) * line_size);
  check_buffer(buffer, "malloc failed in read_line!");
  char* username = getlogin();

  //shell command line
  char name[7];
  while (format_idx < 4) {
    name[format_idx] = pathname[format_idx];
    format_idx++;
  }
  while (format_idx < 7) {
    name[format_idx] = '.';
    format_idx++;
  }

  /* SHELL INPUT LINE */
  printf("%s%s@TShell[%s%s%s][%s%d/%d%s] tetrashell>%s ", username, RED, WHITE, name, 
         RED, WHITE, game.score, game.lines, RED, WHITE);

  //read in line
  while (1) {
    ch = getchar();

    if (ch == EOF || ch == '\n') {
      //edge case: \n or EOF entered right away
      if (idx == 0) {
          char* r = "bad_command";
          return r;
      }
      buffer[idx] = '\0';
      return buffer;
    } else {
      buffer[idx] = ch;
      idx++;
    }

    //resize buf
    if (idx >= line_size) {
      line_size *= 2;
      buffer = realloc(buffer, sizeof(char) * line_size);
      check_buffer(buffer, "read_line buffer realloc failed!");
    }
  }

  return buffer;
}

char** parse_args(char* args) {
  char** buffed = malloc(sizeof(char*));
  check_buffer(buffed, "malloc failed in parse_args!");
  int size = 1, i = 0;
  char* created;
  created = strtok(args, " ");
  while (created != NULL) {
    if (i >= size) {
      size *= 2;
      buffed = realloc(buffed, size * sizeof(char*));
      check_buffer(buffed, "realloc failed in parse_args!");
    }
    buffed[i] = strdup(created);
    created = strtok(NULL, " ");
    i++;
  }
  if (i >= size) {
    buffed = realloc(buffed, (size + 1) * sizeof(char*));
    check_buffer(buffed, "realloc failed in parse_args!");
  }
  buffed[i] = NULL;
  return buffed;
}

char** parse_args_changes(char* args, char* path_name) {
  char** buffed = malloc(sizeof(char*));
  check_buffer(buffed, "malloc failed in parse_args_changes!");
  int size = 1, i = 0;
  char* created;
  created = strtok(args, " \n");
  while (created != NULL) {
    if (i >= size) {
      size *= 2;
      buffed = realloc(buffed, size * sizeof(char*));
      check_buffer(buffed, "realloc failed in parse_args_changes!");
    }
    buffed[i] = strdup(created);
    created = strtok(NULL, " \n");
    i++;
  }
  if (i >= size) {
    size *= 2;
    buffed = realloc(buffed, (size) * sizeof(char*));
    check_buffer(buffed, "realloc failed in parse_args_changes!");
  }
  buffed[i] = strdup(path_name);
  i++;
  if (i >= size) {
    buffed = realloc(buffed, (size + 1) * sizeof(char*));
    check_buffer(buffed, "realloc failed in parse_args_changes!");
  }
  buffed[i] = NULL;
  return buffed;
}

char** parse_args_ranked(char* args, char* path_name) {
  char** buffed = malloc(sizeof(char*));
  check_buffer(buffed, "malloc failed in parse_args_ranked!");
  int size = 1, i = 0;
  char* created;
  created = strtok(args, " \n");
  while (created != NULL) {
    if (i >= size) {
      size *= 2;
      buffed = realloc(buffed, size * sizeof(char*));
      check_buffer(buffed, "realloc failed in parse_args_ranked!");
    }
    buffed[i] = strdup(created);
    created = strtok(NULL, " \n");
    i++;
  }
  if (i >= size) {
    size *= 2;
    buffed = realloc(buffed, (size) * sizeof(char*));
    check_buffer(buffed, "realloc failed in parse_args_ranked!");
  }
  buffed[i] = strdup(path_name);
  i++;
  if (i >= size) {
    size *= 2;
    buffed = realloc(buffed, (size) * sizeof(char*));
    check_buffer(buffed, "realloc failed in parse_args_ranked!");
  }
  buffed[i] = "ok";
  i++;
  if (i >= size) {
    buffed = realloc(buffed, (size + 1) * sizeof(char*));
    check_buffer(buffed, "realloc failed in parse_args_ranked!");
  }
  buffed[i] = NULL;
  return buffed;
}

#define TETRISPIECEARRAYSIZE 16
#define BOARDW 20
void visualize(TetrisGameState state) {
  /* BOARD */
  printf("\n\e[38;2;255;60;0mTHIS IS YOUR BOARD:\n\n");
  printf("%s%s\n", "+---- Gameboard -----+", "\e[38;2;255;255;255m");
  // board width + 1
  char print_str[BOARDW + 1] = {};

  // will need to translate i, j, to index in board field to TGS
  int idx_in_board;
  for (int i = 0; i < BLOCKS_TALL; i++) {
    for (int j = 0; j < BLOCKS_WIDE; j++) {
      idx_in_board = 10 * i + j;
      if (!state.board[idx_in_board]) {
          print_str[2 * j] = ' ';
          print_str[2 * j + 1] = ' ';
      } else {
          print_str[2 * j] = state.board[idx_in_board];
          print_str[2 * j + 1] = state.board[idx_in_board];
      }
    }

    // print border & print_str, three calls for readability
    printf("%s%s", "\e[38;2;255;60;0m|", "\e[38;2;255;255;255m");
    printf("%s", print_str);
    printf("%s%s\n", "\e[38;2;255;60;0m|", "\e[38;2;255;255;255m");
  }

  printf("%s%s\n\n", "\e[38;2;255;60;0m", "+--------------------+");

  /* NEXT PIECE */
  printf("THIS IS YOUR NEXT PIECE:\e[38;2;255;255;255m\n\n");
  printf("%s", "\e[38;2;255;60;0m+----+\e[38;2;255;255;255m");
  for (int i = 0; i < TETRISPIECEARRAYSIZE; i++) {
    if (i % 4 == 3) {
      printf("%c%s", tetris_pieces[state.next_piece][i],
             "\e[38;2;255;60;0m|\e[38;2;255;255;255m");
    } else if (i % 4 == 0) {
      printf("%s%c", "\n\e[38;2;255;60;0m|\e[38;2;255;255;255m",
             tetris_pieces[state.next_piece][i]);
    } else {
      printf("%c", tetris_pieces[state.next_piece][i]);
    }
  }
  printf("%s", "\n\e[38;2;255;60;0m+----+\e[38;2;255;255;255m\n\n");
}

#define READ 0
#define WRITE 1
#define STDIN 0
#define STDOUT 1
void play(char* pathname) {
  printf("\e[38;2;255;60;0mPRESS ENTER TO PLAY\e[38;2;255;255;255m\n");
  getchar();

  // make pipes: p1 for stdout from child, p2 for stdin from parent
  int w_stdout[2];
  check_neg((pipe(w_stdout)), "pipe failed in play");
  int w_stdin[2];
  check_neg((pipe(w_stdin)), "pipe failed in play");

  pid_t pid = fork();
  check_neg(pid, "fork failed in play");
  if (pid == 0) {
    // close read end of stdout, write end of stdin
    close(w_stdout[READ]);
    close(w_stdin[WRITE]);

    // replace stdin & stdout
    dup2(w_stdin[READ], STDIN);
    dup2(w_stdout[WRITE], STDOUT);

    // run tetris
    execl("tetris", pathname, (char*)0);

    // close pipes #TODO: memory leak? do these actually get closed?
    close(w_stdout[WRITE]);
    close(w_stdin[READ]);
  } else {
    // we're going to use ncurses for getch; clearing of stdout shouldn't matter
    // because it's redirected?
    initscr();
    noecho();
    cbreak();

    // set up piping: close write end of stdout, read end of stdin
    close(w_stdout[WRITE]);
    close(w_stdin[READ]);

    // print buffer
    char buffer[4096];

    // read, write loop
    while (1) {
      // read
      ssize_t count = read(w_stdout[READ], buffer, sizeof(buffer));
      check_neg((int)count, "failed to read from tetris' stdout in play");
      if (count == 0) {
        break;
      } else {
        printf("%s", buffer);
      }

      // write
      int inp;
      inp = getch();
      count = write(w_stdin[WRITE], &inp, sizeof(int));
      check_neg((int)count, "failed to write to tetris' stdin in play");
    }
    endwin();

    // close pipes
    close(w_stdout[READ]);
    close(w_stdout[WRITE]);
    wait(0);
  }
}

int recover(char** args, TetrisGameState* game, char** pathname) {
  int p1[2] = {0};
  check_neg((pipe(p1)), "error with pipe in recover");
  pid_t pid = fork();
  check_neg(pid, "fork failed in recover");
  if (pid == 0) {
    close(p1[0]);
    dup2(p1[1], fileno(stdout));
    dup2(open("/dev/null", O_WRONLY), fileno(stderr));
    execve("/playpen/a5/recover", args, NULL);
    perror("failure with execve");
  } else {
    close(p1[1]);
    char output[4096];
    char** child_names = malloc(sizeof(char*));
    child_names[0] = malloc(sizeof(char) * 4096);
    unsigned int inner = 0;
    unsigned int outer = 0;
    unsigned int count = 1;
    ssize_t sizes = 0;
    while ((sizes = read(p1[0], output, 4096)) > 0) {
      for (ssize_t g = 0; g < sizes; ++g) {
        child_names[outer][inner++] = output[g];
        if (output[g] == '\n') {
          child_names[outer++][inner - 1] = 0;
          inner = 0;
          if (outer >= count) {
            count++;
            child_names = realloc(child_names, sizeof(char*) * count);
            child_names[outer] = malloc(sizeof(char) * 4096);
          }
        }
      }
    }
    int ret;
    wait(&ret);

    // early exit if count is still 1
    if (count == 1) {
      printf(
          "\e[38;2;255;60;0mNO FILES READ FROM IMG. PLEASE ENTER A VALID DISK "
          "IMG.\e[38;2;255;255;255m\n");
      return 0;
    }

    /* PRNT OUTPUT */
    char input[4];
    int created;
    printf("Recovered quicksaves:\n");
    printf("---- --------------------- ------- -------\n");
    printf("#    File path             Score   Lines\n");
    printf("---- --------------------- ------- -------\n");
    // read the file name so game score changes, make print out all in line
    // add in final three lines
    for (int i = 0; i < count - 1; i++) {
      FILE* child_file;
      child_file = fopen(child_names[i], "rb");
      check_buffer(child_file, "open failed in recover");
      TetrisGameState game2;
      check_0((fread(&game2, sizeof(TetrisGameState), 1, child_file)), "read failed in recover");
      fclose(child_file);
      printf("%-4d %-21s %-7d %-7d \n", i + 1, child_names[i], game2.score,
             game2.lines);
    }
    
    /* SWITCH FILE */
    char requested[10];
    int num;
    printf("Would you like to switch to one of these (y/n):");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    if (!strcmp(input, "y")) {
      printf("Which quicksave (enter a #):");
      fgets(requested, sizeof(requested), stdin);
      requested[strcspn(requested, "\n")] = 0;
      num = atoi(requested) - 1;
      if (num > -1 && num < count - 1) {
        printf("Done! Current quicksave is now %s\n", child_names[num]);
        *pathname = child_names[num];
        FILE* fp2;
        fp2 = fopen(*pathname, "rb");
        check_buffer(fp2, "tried to open invalid quicksave when switching in recover");
        fread(game, sizeof(TetrisGameState), 1, fp2);
        fclose(fp2);

        /* FREE CHILD_NAMES */
        for (int i = 0; i < sizeof(child_names) / sizeof(char*); i++) {
            free(child_names[i]);
        }
        free(child_names);

        return 1;

      } else {
        printf("invalid file number \n");

        /* FREE CHILD NAMES */
        for (int i = 0; i < sizeof(child_names) / sizeof(char*); i++) {
            free(child_names[i]);
        }
        free(child_names);

        return 0;
      }
    } else {
        /* FREE CHILD NAMEs */
        printf("here\n");
        for (int i = 0; i < sizeof(child_names) / sizeof(char*); i++) {
            free(child_names[i]);
        }
        free(child_names);

        return 0;
    }
  }
}

void modify(char** args, TetrisGameState* game, char** pathname) {
  //normal execution 
  if (!strcmp(args[1], "lines") || !strcmp(args[1], "score")) {
      pid_t pid = fork();
      if (pid == 0) {
        execve("/playpen/a5/modify", args, NULL);
      } else {
        int ret;
        wait(&ret);
      }

      // reopen file to read back in the game stats
      FILE* fp;
      fp = fopen(*pathname, "r");
      check_buffer(fp, "open failed in modify");
      check_0((fread(game, sizeof(TetrisGameState), 1, fp)), "couldn't read back from modify");
      fclose(fp);
      return;
  }

  //next piece
  else if (!strcmp(args[1], "next_piece")) {
      char* str_part;
      long int next_piece = strtol(args[2], &str_part, 10);
      //make sure str_part is empty --> there should be nothing but ints in the str
      if (strcmp(str_part, "")) {
        printf("\e[38;2;255;60;0mUNRECOGNIZED COMMAND GIVEN"
              " TO MODIFY NEXT PIECE. \e[38;2;255;255;255m\n");
        return;
      }
      if (next_piece > 18 || next_piece < 0) {
        printf("\e[38;2;255;60;0mAMOUNT GIVEN TO MODIFY NEXT PIECE IS TOO LARGE OR TOO SMALL."
              "\nPLEASE ENTER A VALUE BETWEEN 0 AND 18. \e[38;2;255;255;255m\n");
        return;

      }

      //cast is fine bc short contains all acceptable values
      game->next_piece = (short) next_piece;   

      
  }

  //board
  else if (!strcmp(args[1], "board")) {
     char* str_part_one, * str_part_two;
     long int x = strtol(args[2], &str_part_one, 10);
     long int y = strtol(args[3], &str_part_two, 10);
     char * print_str = args[4];
     //make sure str_part is empty --> there should be nothing but ints in the str
     if (strcmp(str_part_one, "") || strcmp(str_part_two, "")) {
        printf("\e[38;2;255;60;0mUNRECOGNIZED COMMAND GIVEN TO MODIFY BOARD. "
               " PLEASE ENTER COMMAND IN FORMAT \e[38;2;255;255;255m\n"
               " modify board int int str\n"); 
        return;
     }
     // x and y must be acceptable values for computation to be correct
     if (x < 0 || x > 9) {
        printf("\e[38;2;255;60;0mX VALUE GIVEN TO MODIFY BOARD IS TOO LARGE OR SMALL. "
              "\nPLEASE ENTER A VALUE BETWEEN 0 AND 9. \e[38;2;255;255;255m\n");
        return;
     }
     if (y < 0 || y > 19) {
        printf("\e[38;2;255;60;0mY VALUE GIVEN TO MODIFY BOARD IS TOO LARGE OR SMALL. "
              "\nPLEASE ENTER A VALUE BETWEEN 0 AND 19. \e[38;2;255;255;255m\n");
        return;
     }
     int start_idx = 10 * y + x;
     //make sure string to print doesn't flow out of board
     if (strlen(print_str) + start_idx > 199) {
        printf("\e[38;2;255;60;0mSTRING WOULD OVERFLOW BOARD. "
              "\nPLEASE MAKE STRING SHORTER OR START EARLIER IN BOARD. \e[38;2;255;255;255m\n");
        return;
     }
     for (int i = 0; i < strlen(print_str); i++) {
        game->board[i + start_idx] = print_str[i];
     }

  }

  else if (!strcmp(args[1], "current_piece")) {
     char* str_part_one, * str_part_two;
     long int x = strtol(args[2], &str_part_one, 10);
     long int y = strtol(args[3], &str_part_two, 10);

     if (strcmp(str_part_one, "") || strcmp(str_part_two, "")) {
        printf("\e[38;2;255;60;0mUNRECOGNIZED COMMAND GIVEN TO MODIFY CURRENT_PIECE. "
               " PLEASE ENTER COMMAND IN FORMAT \e[38;2;255;255;255m\n"
               " modify current_piece int int\n"); 
        return;
     }
     // x and y must be acceptable values for computation to be correct
     if (x < 0 || x > 9) {
        printf("\e[38;2;255;60;0mX VALUE GIVEN TO MODIFY CURRENT_PIECE IS TOO LARGE OR SMALL. "
              "\nPLEASE ENTER A VALUE BETWEEN 0 AND 9. \e[38;2;255;255;255m\n");
        return;
     }
     if (y < 0 || y > 19) {
        printf("\e[38;2;255;60;0mY VALUE GIVEN TO MODIFY CURRENT_PIECE IS TOO LARGE OR SMALL. "
              "\nPLEASE ENTER A VALUE BETWEEN 0 AND 19. \e[38;2;255;255;255m\n");
        return;
     }
     int start_idx = 10 * y + x;
     game->current_piece = start_idx;

  }

  else {
      printf("\e[38;2;255;60;0mUNRECOGNIZED COMMAND GIVEN"
              "TO MODIFY. \e[38;2;255;255;255m\n");
      return;
  }

  //the code to write modify next_piece, board is the same, so we'll combine that here
  if (!strcmp(args[1], "board") || !strcmp(args[1], "next_piece")) {
      FILE * fp; 
      check_buffer((fp = fopen(*pathname, "wb")), "file open to write failed in modify!");
      check_0((fwrite(game, sizeof(TetrisGameState), 1, fp)), "write failed in modify");
      fclose(fp);
  }
}

void help(char** args) {
  if (!strcmp(args[1], "modify")) {
    printf(
        "This command calls the `modify` program with the current "
        "quicksave to modify either the score or lines, given as the "
        "second argument, to a certain value given as the third "
        "argument.\n");
  }
  if (!strcmp(args[1], "check")) {
    printf(
        "This command calls the `check` program with the current quicksave "
        "to verify if it will pass legitimacy checks.\n");
  }
  if (!strcmp(args[1], "recover")) {
    printf(
        "This command calls the `recover` program with the given disk "
        "image as the 2nd argument and recovers the files in the image."
        " The user can switch to one of the recovered game states.\n");
  }
  if (!strcmp(args[1], "rank")) {
    printf(
        "This command calls the 'rank' program with the number if given as "
        "the 2nd argument and ranks the top quicksaves up to the given "
        "number.\n");
  }
  if (!strcmp(args[1], "undo")) {
    printf(
        "Undoes the last modify. The shell saves a max of three game "
        "states.\n");
  }
  if (!strcmp(args[1], "visualize")) {
    printf("Prints the current game board and piece.\n");
  }
  if (!strcmp(args[1], "exit")) {
    printf("Exits the program.\n");
  }
  if (!strcmp(args[1], "info")) {
    printf("Prints the current score and lines.\n");
  }
  if (!strcmp(args[1], "switch")) {
    printf("Changes the current file to the second argument called with "
           "switch. This will reset the undo buffer.\n");
  }
}

int switch_func(char** args, TetrisGameState* game, char** pathname) {
  //TODO: print help when command called with no arg[1]:
  char* current;
  current = *pathname;
  *pathname = args[1];
  FILE* fp2;
  size_t pathsize = 256;
  int c;

  while (!verify_save(game, pathname, &fp2)) {
    printf("\e[38;2;255;60;0mFINDING, READING, OR SAVING FROM FILE FAILED.\n"
           "PLEASE ENTER ANOTHER PATH, OR EXIT TO QUIT. \e[38;2;255;255;255m\n");

    c = getline(pathname, &pathsize, stdin);
    check_neg(c, "getline failure while getting a valid save"); //getline returns < 0 on failure
    
    (*pathname)[c - 1] = 0;
    if (!strcmp(*pathname, "exit") || !strcmp(*pathname, "exit")) {
        return EXIT_FAILURE;
    }
  }

  printf("Switched current quicksave from '%s' to '%s'.\n", current, *pathname);
  return 1;
}


void rank(char* arg, char** args, char** pathname) {
  if (!args[1]) {
    args = parse_args_ranked(arg, *pathname);
    args[0] = "rank";
    args[1] = "score";
    args[2] = "10";
    args[3] = "\n";
  }
  if (!args[2]) {
    args = parse_args_changes(arg, *pathname);
    args[2] = "10";
  }
  int p[2];
  check_neg((pipe(p)), "pipe failed in rank");
  pid_t pid = fork();
  check_neg(pid, "fork failed in rank");
  if (pid == 0) {
    close(p[1]);
    dup2(p[0], STDIN_FILENO);
    args[0] = "/playpen/a5/rank";
    args[3] = "uplink";
    execve("/playpen/a5/rank", args, NULL);
  } else {
    close(p[0]);
    check_0((write(p[1], *pathname, strlen(*pathname))), "write from shell to rank failed");
    int ret;
    close(p[1]);
    wait(&ret);
  }
}

void check_buffer(void* buffer, char* output_text) {
  if (!buffer) {
    perror("Error ");
    fprintf(stderr, "%s\n", output_text);
    exit(EXIT_FAILURE);
  }
}

void check_0(int to_check, char* output_text) {
  if (to_check == 0) {
    fprintf(stderr, "%s\n", output_text);
    exit(EXIT_FAILURE);
  }
}

void check_EOF(int to_check, char* output_text) {
  if (to_check <= EOF) {
    fprintf(stderr, "%s\n", output_text);
    exit(EXIT_FAILURE);
  }
}

