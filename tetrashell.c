#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tetris.h"
#define STDLINESIZE 128

char* read_line();
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
    
    while(strcmp((arg = read_line()), "exit")) {
	char* arg_copy = strdup(arg);
   	args = parse_args(arg_copy);
        if (!strcmp(args[0], "recover")) {
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
	 if (!strcmp(args[0], "info")) {
            
	    FILE *fp;
	    fp = fopen(pathname, "rb");
            TetrisGameState game;
	    fread(&game, sizeof(TetrisGameState), 1, fp);	
            printf("Current savefile: %s \n", pathname);
	    printf("Score: %d \n", game.score);
	    printf("Lines: %d \n", game.lines);	
	    free(fp);    
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
	/* if (!strcmp(args[0], "rank")) {
	   int p[2];
  	   write(p[1], pathname, sizeof(pathname));		   
           printf("found check\n");
           pid = fork();
           if (pid == 0) {
            execve("/playpen/a5/check", args2, NULL);
           }else{
                   int ret;
                   wait(&ret);
           }
         }*/
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

char* read_line() {
    int line_size = STDLINESIZE, idx = 0, ch;
    char* buffer = malloc(sizeof(char) * line_size);
    check_buffer(buffer, "malloc failed in read_line!");

    printf("tetrashell> ");
    
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
