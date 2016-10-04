#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

int main (int argc, char *argv[])
{
    FILE *input;
    FILE *output;
    /* getopt silent mode set */
    opterr = 0;

    if(argc < 3) {
        printf("Too few parameters.\n");
        return 0;
    } else if(argc > 3) {
        printf("Too many parameters, ignoring the rest.\n");
    } else {
        printf("Starting up!\n");
    }

    /* Opening CSAC log */
    input = fopen(argv[1], "r");
    if (input == NULL) {
        printf("Failed to load %s\n", argv[1]);
        return 0;
    }

    /* Opening outout */
    output = fopen(argv[2], "a");
    if (output == NULL) {
        printf("Failed to load %s\n", argv[2]);
        return 0;
    }

    char buffer[1024];

    int i;
    int cr_check = 0;
    while(fread(buffer , 1024, 1 , input)) {
        for(i = 0; i < 1024; i++) {
            if(buffer[i] != 0x0d) {
                fprintf(output, "%c", buffer[i]);
            } else {
                if(cr_check == 1) {
                    fprintf(output, "\n");
                    cr_check = 0;
                } else {
                    cr_check++;
                }
            }
        }
    }

    fclose(input);
    fclose(output);
    exit(0);
}