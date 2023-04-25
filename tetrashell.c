#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

#define STDLINESIZE 128

char* read_line();
char** parse_args(char* args);

int main() {
    char* arg;
    char** args; 
    while(strcmp(arg = read_line(), "exit")) {
        printf("%s\n", arg);
        args = parse_args(arg);
        if (!strcmp(args[1], "recover")) {
           printf("found recover\n"); 
        }
    }
    free(args);
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
}


//TODO: rewrite parseargs using strtok
