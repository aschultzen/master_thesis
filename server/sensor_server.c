#include "sensor_server.h"

int main(int argc, char *argv[])
{
    /* Checking parameters */
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    start_server(atoi(argv[1]));
    exit(0);
}