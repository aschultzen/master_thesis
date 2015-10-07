/* 
*   TODO:
*   Make client ->
*                   Connect to server (Reuse server code)
*                   Identify and handle rejection (./client port, ip, ID, GPS_CON)
*                   Use Store_GPRMC command to store data from GPS
*                   
*   Server  ->
*               Implement store_GPRMC command
*               
*/      

#include "sensor_server.h"

int main(int argc, char *argv[])
{
    /* Checking parameters */
    t_print("Starting the server...\n");
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    start_server(atoi(argv[1]), argv[2]);
    exit(0);
} 