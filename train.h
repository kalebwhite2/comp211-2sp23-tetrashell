void gen_hex(char** randhex, int length); 

#define RED "\e[38;2;255;60;0m"
#define WHITE "\e[38;2;255;255;255m"
void train() {
    /* INTRO */
    printf("%s%s%s", RED, "WELCOME TO TRAIN.\n"
          "PRESS ENTER TO BEGIN.\n"
          "TYPE EXIT TO QUIT.\n", WHITE);
    getchar();

    /* INIT */
    printf("%s%s%s", RED, "A HEX WILL BE PRINTED.\n"
          "ENTER YOUR GUESS OR:\n"
          "PRESS + AND ENTER TO INCREASE DIFFICULTY.\n"
          "PRESS - AND ENTER TO DECREASE DIFFICULTY.\n"
          "YOU WILL BE NOTIFIED WHETHER OR NOT YOUR GUESS IS CORRECT.\n"
          "PRESS ENTER TO CONTINUE.\n", WHITE);

    int length = 4;
    size_t pathsize = 256;
    //malloc buffer and get first command
    char* buffer = malloc(256), *h = malloc(21), *a = malloc(21);
    int c = getline(&buffer, &pathsize, stdin);
    if (c < 0) {
        printf("getline failed in train\n");
        return;
    }
    if (c == 0) {
        buffer[0] = 'a';
        buffer[1] = 0;
    } else {
        buffer[c - 1] = 0;
    }

    /* GAME LOOP */
    while(strcmp(buffer, "exit")) {
        //print hex
        gen_hex(&h, length); 
        printf("%s%s%s%s\n", RED, "ENTER THE DECIMAL REPRESENTATION OF: 0x", h, WHITE);
        
        //get input
        c = getline(&buffer, &pathsize, stdin);
        if (c < 0) {
            printf("getline failed in train\n");
            return;
        }
        if (c == 0) {
            buffer[0] = 'a';
            buffer[1] = 0;
        } else {
            buffer[c - 1] = 0;
        }

        //respond
        if (!strcmp(buffer, "+")) {
            if (length == 20) {
                printf("%s%s%s\n", RED, "YOU ARE ALREADY AT MAX DIFFICULTY.", WHITE);
            } else {
                length++;
                printf("%s%s%s\n", RED, "DIFFICULTY INCREASED.", WHITE);
            }
        }
        else if (!strcmp(buffer, "-")) {
            if (length == 4) {
                printf("%s%s%s\n", RED, "YOU ARE ALREADY AT MIN DIFFICULTY.", WHITE);
            } else {
                length--;
                printf("%s%s%s\n", RED, "DIFFICULTY DECREASED.", WHITE);
            }
        }
        else if (strcmp(buffer, "exit")) {
            char* otpt[5];
            int dec = (int) strtol(h, otpt, 16);
            sprintf(a, "%d", dec);
            if (!strcmp(buffer, a)) {
                printf("%s%s%s\n", RED, "THAT IS CORRECT.\nCLEARLY THIS IS TOO EASY.\n"
                        "DIFFICULTY INCREASED.\n", WHITE);
                length++;
            } else if (length == 4) {
                printf("%s%s%s\n", RED, "INCORRECT.\n"
                        "I WOULD MAKE IT EASIER, BUT THIS IS AS EASY AS IT GETS.\n", WHITE);
            } else {
                printf("%s%s%s\n", RED, "INCORRECT.\n"
                        "LET'S MAKE IT EASIER.\n", WHITE);
                length--;
            }
        }
    }   
    printf("%s%s%s\n", RED, "QUITTING TRAIN.", WHITE);
    free(buffer);
    free(h);
    free(a);
}

void gen_hex(char** randhex, int length) {
    for (int i = 0; i < length; i++) {
        sprintf(*randhex + i, "%x", rand() % 16);
    }

    (*randhex)[length] = '\0';
}
