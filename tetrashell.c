#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tetris.h"
#define STDLINESIZE 128

char* read_line(TetrisGameState game, char* pathname);
char** parse_args(char* args);
char** parse_args_changes(char* args, char* path_name);
char* welcome();
void print_image(FILE *fptr);
void check_buffer(void* buffer, char* output_text); 

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
    TetrisGameState game;
    fread(&game, sizeof(TetrisGameState), 1, fp);
    free(fp);
    
    while(strcmp((arg = read_line(game, pathname)), "exit")) {
	char* arg_copy = strdup(arg);
   	args = parse_args(arg_copy);
        if (!strcmp(args[0], "recover") || !strcmp(args[0], "rec") ) {
	 // p1[2];
	 // if (pipe(p1) < 0} {
 	  //    return(EXIT_FAILURE);
	  //}	   
           printf("found recover\n");
           pid = fork();
           if (pid == 0) {
	    // fclose(p1[0]);	    
            execve("/playpen/a5/recover", args, NULL);
	   }else{
                   int ret;
                   wait(&ret);
           }
        }
	 if (!strcmp(args[0], "modify")) {
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
	 if (!strcmp(args[0], "help")) {
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
	 if (!strcmp(args[0], "info")) {
            printf("Current savefile: %s \n", pathname);
	    printf("Score: %d \n", game.score);
	    printf("Lines: %d \n", game.lines);	
         }
	 if (!strcmp(args[0], "switch")) {
	    char* current;
	    current = pathname;
	    pathname = args[1];
	    printf("Switch current quicksave from '%s' to '%s'.\n", current, pathname);
         }
	 if (!strcmp(args[0], "check")) {
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
	 if (!strcmp(args[0], "rank")) {
	   int p[2];
	   pipe(p); //error check this
           //printf("found check\n");
           pid = fork();
           if (pid == 0) {
	    close(p[1]);
	    dup2(p[0], STDIN_FILENO);
	    /*
	    for (char** c = args; c != NULL; c++) {
	       printf("r= %s\n", *c);
	    }
	    */
	   // args[0] = "/playpen/a5/rank"; 
	    args[3] = "uplink";
            execve("/playpen/a5/rank", args, NULL);
	    //read(
           }else{
	          close(p[0]);
  	          write(p[1], pathname, strlen(pathname));		   
                   int ret;
		  close(p[1]);
                   wait(&ret);
           }
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
    getline(&pathname, &pathsize, stdin);
    
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
	    //buffed[i][strcspn(buffed[i], "\n")] = 0;
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

void check_buffer(void* buffer, char* output_text) {
    if (!buffer) {
        fprintf(stderr, "%s\n", output_text);
        exit(EXIT_FAILURE);
    }
}
