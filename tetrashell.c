#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "tetris.h"
#define STDLINESIZE 128

char* read_line(TetrisGameState game, char* pathname);
char** parse_args(char* args);
char** parse_args_changes(char* args, char* path_name);
char** parse_args_ranked(char* args, char* path_name);
char* welcome();
void print_image(FILE *fptr);
void check_buffer(void* buffer, char* output_text); 
void visualize(TetrisGameState state);

int main() {
    char* arg;
    char** args;
    char** args2; 
    pid_t pid;
    /* FIND SAVE FILE */
    char* pathname = welcome();

    pathname[strcspn(pathname, "\n")] = 0;
    /* MAIN LOOP */
    FILE *fp;
    fp = fopen(pathname, "rb");
    if (fp == NULL) {
      perror("Error: ");
      return EXIT_FAILURE;
     }
    TetrisGameState game;
    int read_check;
    read_check = fread(&game, sizeof(TetrisGameState), 1, fp);
    if (read_check < 1) {
   	 perror("Error: ");
    	return EXIT_FAILURE;
    }   
    int close_check;
    close_check = fclose(fp);
    if (close_check != 0) {
    	perror("Error: ");
    	return EXIT_FAILURE;
    }
    while(strcmp((arg = read_line(game, pathname)), "exit")) {
	char* arg_copy = strdup(arg);
   	args = parse_args(arg_copy);
        if (!strcmp(args[0], "recover") || !strcmp(args[0], "re") || !strcmp(args[0], "rec") || !strcmp(args[0], "reco") || !strcmp(args[0], "recov") || !strcmp(args[0], "recove") ) {
	   int p1[2] = {0};
           if (pipe(p1) < 0) {
 	     perror("Error with pipe");
	     return(EXIT_FAILURE);
	   }	   
           pid = fork();
	   if (pid == -1) {
	    perror("Error: ");
            return 1;
           }else if (pid == 0) {
	    close(p1[0]);
            dup2(p1[1], fileno(stdout));
            dup2(open("/dev/null", O_WRONLY), fileno(stderr));	    
            execve("/playpen/a5/recover", args, NULL);
	    perror("failure with execve");
	   }else{
	    close(p1[1]);
	    char output[4096];
	    char** child_names = malloc(sizeof(char*));
	    child_names[0] = malloc(sizeof(char)*4096);
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
		       child_names = realloc(child_names, sizeof(char*)*count);
		       child_names[outer] = malloc(sizeof(char)*4096); 
                     }
		  }
		}
	      }	
                   int ret;
                   wait(&ret);
		   char input[4];
		   int created;
		   printf("Recovered quicksaves:\n");
		   printf("---- --------------------- ------- -------\n");
		   printf("#    File path             Score   Lines\n");
		   printf("---- --------------------- ------- -------\n");
		   //read the file name so game score changes, make print out all in line add in final three lines
		   for (int i = 0; i < count - 1; i++) {
		     FILE *child_file;
		     child_file = fopen(child_names[i], "rb");
		         if (child_file == NULL) {
     		            printf("Invalid open path");
     			    return EXIT_FAILURE;
    			 }
    		     TetrisGameState game2;
    		     fread(&game2, sizeof(TetrisGameState), 1, child_file);
    		     fclose(child_file);
		     printf("%-4d %-21s %-7d %-7d \n", i+1, child_names[i], game2.score, game.lines);	
           }
		   char requested[10];
		   int num;
		   printf("Would you like to switch to one of these (y/n):");
                   fgets(input, sizeof(input), stdin);
		   input[strcspn(input,"\n")] = 0;
		   if (!strcmp(input, "y")) {
		     printf("Which quicksave (enter a #):");
		     fgets(requested, sizeof(requested), stdin);
		     requested[strcspn(requested,"\n")] = 0;
		     num = atoi(requested) - 1;
		     if (num > -1 && num < count - 1) {	
		     	printf("Done! Current quicksave is now %s\n", child_names[num]);
		     	pathname = child_names[num];
		     	FILE *fp2;
    		     	fp2 = fopen(pathname, "rb");
		          if (fp2 == NULL) {
                            printf("Invalid open path\n");
                            return EXIT_FAILURE;
                         } else {
    		     	    fread(&game, sizeof(TetrisGameState), 1, fp2);
    		     	    fclose(fp2);
		         }
		     }else {
		         printf("invalid file number\n");
	             }		 
       }
     }	   
    }
	 if (!strcmp(args[0], "modify") || !strcmp(args[0], "m") || !strcmp(args[0], "mo") || !strcmp(args[0], "mod") || !strcmp(args[0], "modi") || !strcmp(args[0], "modif")){
           args2 = parse_args_changes(arg, pathname);
	   printf("found modify\n");
           pid = fork();
           if (pid == 0) {
            execve("/playpen/a5/modify", args2, NULL);
           }else{
                   int ret;
                   wait(&ret);
           }
	   free(args2);
	 }
	 if (!strcmp(args[0], "help") || !strcmp(args[0], "h") || !strcmp(args[0], "he") || !strcmp(args[0], "hel")) {
           if (!strcmp(args[1], "modify")) {
             printf("This command calls the `modify` program with the current quicksave to modify either the score or lines, given as the second argument, to a certain value given as the third argument.\n");
           }
	   if (!strcmp(args[1], "check")) {
             printf("This command calls the `check` program with the current quicksave to verify if it will pass legitimacy checks.\n");
 	   }
	   if (!strcmp(args[1], "recover")) {
             printf("This command calls the `recover` program with the given disk image as the 2nd argument and recovers the files in the image.\n");
           }
	   if (!strcmp(args[1], "rank")) {
             printf("This command calls the 'rank' program with the number if given as the 2nd argument and ranks the top quicksaves up to the given number.\n");  
	   } 
	   if (!strcmp(args[1], "exit")) {
	     printf("Exits the program.\n");
           }

	 }
	 if (!strcmp(args[0], "info") || !strcmp(args[0], "i") || !strcmp(args[0], "in") || !strcmp(args[0], "inf")) {
            printf("Current savefile: %s \n", pathname);
	    printf("Score: %d \n", game.score);
	    printf("Lines: %d \n", game.lines);	
         }
	 if (!strcmp(args[0], "switch") || !strcmp(args[0], "s") || !strcmp(args[0], "sw") || !strcmp(args[0], "swi") || !strcmp(args[0], "swit") || !strcmp(args[0], "switc")) {
	    char* current;
	    current = pathname;
	    pathname = args[1];
	    FILE *fp2;
            fp2 = fopen(pathname, "rb");
	    if (fp2 == NULL) {
            	printf("Invalid open path\n");
            	return EXIT_FAILURE;
            }
	    printf("Switch current quicksave from '%s' to '%s'.\n", current, pathname);
            fread(&game, sizeof(TetrisGameState), 1, fp2);
            fclose(fp2);
	 }
	 if (!strcmp(args[0], "check") || !strcmp(args[0], "c") || !strcmp(args[0], "ch") || !strcmp(args[0], "che") || !strcmp(args[0], "chec")) {
           args2 = parse_args_changes(arg, pathname);
           printf("found check\n");
           pid = fork();
           if (pid == 0) {
            execve("/playpen/a5/check", args2, NULL);
           }else{
                   int ret;
                   wait(&ret);
           }
	   free(args2);
	 }  
	 if (!strcmp(args[0], "rank") || !strcmp(args[0], "ra") || !strcmp(args[0], "ran")) {
           if (!args[1]) {
	      args = parse_args_ranked(arg, pathname);	   
              args[0] = "rank";
	      args[1] = "score";
	      args[2] = "10";
	      args[3] = "\n";
           }		 
           if (!args[2]) {
  	      args = parse_args_changes(arg, pathname);
	      args[2] = "10";
           }		      
	   int p[2];
	   if (pipe(p) == -1) {
	     perror("Error ");
	     return 1;
	   } 
           pid = fork();
	   if (pid == -1) {
	     perror("Error ");
	     return 1;
	   }
	   else if (pid == 0) {
	    close(p[1]);
	    dup2(p[0], STDIN_FILENO);
	    args[3] = "uplink";
            execve("/playpen/a5/rank", args, NULL);
           }else{
	          close(p[0]);
  	          write(p[1], pathname, strlen(pathname));		   
                   int ret;
		  close(p[1]);
                   wait(&ret);
           }
         }
      if (!strcmp(args[0], "visualize") || !strcmp(args[0], "v") || !strcmp(args[0], "vi") || !strcmp(args[0], "vis") || !strcmp(args[0], "visu") || !strcmp(args[0], "visua") || !strcmp(args[0], "visual") || !strcmp(args[0], "visuali") || !strcmp(args[0], "visualiz")) {
        visualize(game); 
      }
            free(args);
    }
    free(pathname);
    free(arg);
}

char* welcome() {
    //print out the intro text from intro_art.txt
    FILE * imgpath = fopen("intro_art.txt", "r");
    if (imgpath == NULL) {
        fprintf(stderr, "unable to open image \n");
        exit(EXIT_FAILURE);    
    }
    print_image(imgpath);
    fclose(imgpath);

    printf("WELCOME TO TETRASHELL\nTHE SHELL OF YOUR DREAMS\nENTER THE PATH TO YOUR QUICKSAVE: \n");

    //getline will automatically resize pathname if necessary
    char* pathname = malloc(256); 
    size_t pathsize = 256;
    int c;
    c = getline(&pathname, &pathsize, stdin);
    FILE *fp2;
    pathname[c - 1] = 0;
    fp2 = fopen(pathname, "rb");
    if (fp2 == NULL) {
      perror("Error: ");
      exit(1);
     }
    fclose(fp2);
    return pathname;
}

#define MAXIMGLINESIZE 128
void print_image(FILE * imgpath) {
    char str[MAXIMGLINESIZE];

    while(fgets(str, sizeof(str), imgpath) != NULL) {
        printf("%s", str);
    }
}


char* read_line(TetrisGameState game, char* pathname) {
    int line_size = STDLINESIZE, idx = 0, ch;
    char* buffer = malloc(sizeof(char) * line_size);
    check_buffer(buffer, "malloc failed in read_line!");
    char* username = getlogin();
    char name[7];
    int i = 0;
    while (i < 4) {
	name[i] = pathname[i];
	i++;
    }
    while (i < 7) {
	name[i] = '.';
	i++;
    }
    printf("%s@TShell[%s][%d/%d] tetrashell>", username, name, game.score, game.lines);
    while (1) {
        ch = getchar();

        if (ch == EOF || ch == '\n') {
            buffer[idx] =  '\0';
            return buffer;
        } else {
            buffer[idx] = ch;
            idx++;
        }

        if (idx >= line_size) {
            line_size *= 2;
            buffer = realloc(buffer, sizeof(char) * line_size);
            check_buffer(buffer, "read_line buffer realloc failed!");
        }
    }
    return buffer;
}

char** parse_args(char* args){
    char** buffed = malloc(sizeof(char*));
    check_buffer(buffed, "malloc failed in parse_args!");
    int size = 1, i = 0;
    char* created;
    created = strtok(args, " ");
    while (created != NULL) {
	    if (i >= size) {
              size *= 2;
	      buffed = realloc(buffed, size*sizeof(char*));
          check_buffer(buffed, "realloc failed in parse_args!");
	    }
	    buffed[i] = strdup(created);
	    created = strtok(NULL, " ");
	    i++;
    }
    if (i >= size) {
       buffed = realloc(buffed, (size + 1)*sizeof(char*));
       check_buffer(buffed, "realloc failed in parse_args!");
    }
    buffed[i] = NULL;
    return buffed;
}

char** parse_args_changes(char* args, char* path_name){
    char** buffed = malloc(sizeof(char*));
    check_buffer(buffed, "malloc failed in parse_args!");
    int size = 1, i = 0;
    char* created;
    created = strtok(args, " \n");
    while (created != NULL) {
            if (i >= size) {
              size *= 2;
              buffed = realloc(buffed, size*sizeof(char*));
          check_buffer(buffed, "realloc failed in parse_args!");
            }
            buffed[i] = strdup(created);
	    //buffed[i][strcspn(buffed[i], "\n")] = 0;
            created = strtok(NULL, " \n");
            i++;
    }
    if (i >= size) {
	  size *= 2;  
    buffed = realloc(buffed, (size)*sizeof(char*));
    check_buffer(buffed, "realloc failed in parse_args!");
    } 
    buffed[i] = strdup(path_name);
    i++;
    if (i >= size) {
       buffed = realloc(buffed, (size + 1)*sizeof(char*));
       check_buffer(buffed, "realloc failed in parse_args!");
    }
    buffed[i] = NULL;
    return buffed;
}

char** parse_args_ranked(char* args, char* path_name){
    char** buffed = malloc(sizeof(char*));
    check_buffer(buffed, "malloc failed in parse_args!");
    int size = 1, i = 0;
    char* created;
    created = strtok(args, " \n");
    while (created != NULL) {
            if (i >= size) {
              size *= 2;
              buffed = realloc(buffed, size*sizeof(char*));
          check_buffer(buffed, "realloc failed in parse_args!");
            }
            buffed[i] = strdup(created);
            //buffed[i][strcspn(buffed[i], "\n")] = 0;
            created = strtok(NULL, " \n");
            i++;
    }
    if (i >= size) {
          size *= 2;
    buffed = realloc(buffed, (size)*sizeof(char*));
    check_buffer(buffed, "realloc failed in parse_args!");
    }
    buffed[i] = strdup(path_name);
    i++;
    if (i >= size) {
          size *= 2;
    buffed = realloc(buffed, (size)*sizeof(char*));
    check_buffer(buffed, "realloc failed in parse_args!");
    }
    buffed[i] = "ok";
    i++;
    if (i >= size) {
       buffed = realloc(buffed, (size + 1)*sizeof(char*));
       check_buffer(buffed, "realloc failed in parse_args!");
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
    //board width + 1
    char print_str[BOARDW + 1];
    
    //will need to translate i, j, to index in board field to TGS
    int idx_in_board;
    for (int i = 0; i < BLOCKS_TALL; i++) {
      for (int j = 0; j < BLOCKS_WIDE; j++) {
        idx_in_board = 10 * i + j;
        print_str[2 * j] = state.board[idx_in_board];
        print_str[2 * j + 1] = state.board[idx_in_board];
      }

      //print border & print_str, three calls for readability
      printf("%s%s", "\e[38;2;255;60;0m|", "\e[38;2;255;255;255m");
      printf("%s", print_str);
      printf("%s%s\n", "\e[38;2;255;60;0m|", "\e[38;2;255;255;255m");
    }

    printf("%s%s\n\n", "\e[38;2;255;60;0m", "+--------------------+");

    /* NEXT PIECE */
    printf("THIS IS YOUR NEXT PIECE:\e[38;2;255;255;255m\n\n");
    printf("%s", "+----+");
    for (int i = 0; i < TETRISPIECEARRAYSIZE; i++) {
       if (i % 4 == 3) {
            printf("%c%s", tetris_pieces[state.current_piece][i], "|");
       }
       else if (i % 4 == 0) {
            printf("%s%c", "\n|", tetris_pieces[state.current_piece][i]);
       } else {
            printf("%c", tetris_pieces[state.current_piece][i]);
       }
    }
    printf("%s", "\n+----+\n\n");
}

void check_buffer(void* buffer, char* output_text) {
    if (!buffer) {
        fprintf(stderr, "%s\n", output_text);
        exit(EXIT_FAILURE);
    }
}
