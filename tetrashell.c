#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

#define STDLINESIZE 128

char* read_line();
char** parse_args(char* args);
char* welcome();
void print_image(FILE *fptr);
void check_buffer(void* buffer, char* output_text); 

int main() {
    char* arg;
    char** args; 
    
    /* FIND SAVE FILE */
    char* pathname = welcome();

    /* MAIN LOOP */
    
    while(strcmp((arg = read_line()), "exit")) {

        args = parse_args(arg);
        if (!strcmp(args[0], "recover")) {
           printf("found recover\n");
	       break; 
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

void check_buffer(void* buffer, char* output_text) {
    if (!buffer) {
        fprintf(stderr, "%s\n", output_text);
        exit(EXIT_FAILURE);
    }
}
