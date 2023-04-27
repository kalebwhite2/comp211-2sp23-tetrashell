#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

#define STDLINESIZE 128

char* read_line();
char** parse_args(char* args){
    char** buffed = malloc(sizeof(char*));
    int size = 1;
    char* created;
    int i;
    i = 0;
    created = strtok(args, " ");
    while (created != NULL) {
	    if (i >= size) {
              size *= 2;
	      buffed = realloc(buffed, size*sizeof(char*));
	    }
	    buffed[i] = strdup(created);
	    printf("%s\n", created);
	    created = strtok(NULL, " ");
	    i++;
    }
    if (i >= size) {
       buffed = realloc(buffed, (size + 1)*sizeof(char*));
    }
    buffed[i] = NULL;
    return buffed;
}

int main() {
    char* arg;
    char** args; 
    while(strcmp(arg = read_line(), "exit")) {
        //printf("%s\n", arg);
        args = parse_args(arg);
        if (!strcmp(args[0], "recover")) {
           printf("found recover\n");
	   break; 
        }
	free(args);
    }
    free(arg);
}

char* read_line() {
    int line_size = STDLINESIZE;
    char* buffer = malloc(sizeof(char) * line_size);
    int idx = 0;
    int ch;

    if (!buffer) {
        fprintf(stderr, "allocation error in read_line");
        exit(EXIT_FAILURE);
    }

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
    }
    return buffer;
}


