#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

#define STDLINESIZE 128

char* read_line();
char** parse_args(char* args);
char* welcome();
void print_image(FILE *fptr);

int main() {
    char* arg;
    char** args; 

    char* pathname = welcome();

    //init loop 
    printf("tetrashell> ");
    arg = read_line();
    while(strcmp(arg, "exit")) {

        args = parse_args(arg);
        if (!strcmp(args[0], "recover")) {
           printf("found recover\n");
	       break; 
        }
	    free(args);

        //run next
        printf("tetrashell> ");
        arg = read_line();
    }

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

    char* pathname = read_line(); 
    
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
    int line_size = STDLINESIZE;
    char* buffer = malloc(sizeof(char) * line_size);
    int idx = 0;
    int ch;

    if (!buffer) {
        fprintf(stderr, "allocation error in read_line");
        exit(EXIT_FAILURE);
    }

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

char** parse_args(char* args){
    char** buffed = malloc(sizeof(char*));
    int size = 1, i = 0;
    char* created;
    created = strtok(args, " ");
    while (created != NULL) {
	    if (i >= size) {
              size *= 2;
	      buffed = realloc(buffed, size*sizeof(char*));
	    }
	    buffed[i] = strdup(created);
	    created = strtok(NULL, " ");
	    i++;
    }
    if (i >= size) {
       buffed = realloc(buffed, (size + 1)*sizeof(char*));
    }
    buffed[i] = NULL;
    return buffed;
}


