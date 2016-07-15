#include "serial.h"

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                t_print("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 100;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                t_print ("error %d setting term attributes", errno);
}

/*
* As of 7.1.16 this is broken since both
* static char *serial_display_connections;
* static char *serial_display_message;
* where both removed. There is however, a new structure
* that should be usable.
*/
int configure_gps_serial(int fd)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);

    if (tcgetattr (fd, &tty) != 0) {
        printf ("error %d from tcgetattr", errno);
        exit(0);
    }

    cfsetospeed (&tty, B9600);
    cfsetispeed (&tty, B9600);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_oflag &= ~OPOST;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
        printf ("error %d setting term attributes", errno);
        return -1;
    }
    return 0;
}

/* 
* This function is pretty much identical
* to configure_gps_serial. Consider removing
* one of them 
*/
int configure_csac_serial(int fd)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);

    if (tcgetattr (fd, &tty) != 0) {
        printf ("error %d from tcgetattr", errno);
        exit(0);
    }

    cfsetospeed (&tty, B57600);
    cfsetispeed (&tty, B57600);

    tty.c_cflag &= ~PARENB;     // No Parity
    tty.c_cflag &= ~CSTOPB;     // 1 stop bit
    tty.c_cflag &= ~CSIZE;      // See under...
    tty.c_cflag |= CS8;         // 8 bits

    tty.c_cflag &= ~CRTSCTS;    // Disable HW flow control
                                // The csac does not use flow
                                // control
                                
    tty.c_cflag |= CREAD | CLOCAL;  // Enable receiver
                                    // Local line, don't change port owner
    
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Software flow control
    tty.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Enable canonical input
                                    //
    tty.c_oflag &= ~OPOST;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
        printf ("error %d setting term attributes", errno);
        return -1;
    }

    return 0;
}

int open_serial(char *portname, serial_device device)
{
    int fd = open (portname, O_RDWR | O_NOCTTY | O_NDELAY); //O_SYNC
    if (fd < 0) {
        t_print ("Error %d opening %s: %s\n", errno, portname, strerror (errno));
    }

    switch(device) {
        case GPS:
            if(configure_gps_serial(fd) < 0) {
                t_print("Serial config failed...\n");
                return -1;
            }
        case CSAC:
            if(configure_csac_serial(fd) < 0) {
                set_blocking(fd, 1);
                t_print("Serial config failed...\n");                
                return -1;
            }            
    }
    return fd;
}

int serial_query(int file_descriptor, char *query, char *buffer, int buf_len)
{
    /* NOTE FOR LATER BUG HUNT!
    * The query is always followed by the newline. 
    * It is also preceded by one if there is a space between the 
    * command and parameter e.g(QC ^).
    * You also need some sort of tiemout function.
    */

    int write_status = write(file_descriptor, query, strlen(query));
    if( write_status < 0){
        t_print("Serial write failed\n");
        return -1;
    }

    usleep(100000);

    int counter = 0;

    while(counter <= buf_len){
        char temp[1];
        int read_status = read(file_descriptor, temp, 1);
        buffer[counter] = temp[0];
        counter++;
    }
    /*int read_status = read(file_descriptor, buffer, buf_len);
    if( read_status < 0){
        t_print("Serial read failed\n");
        return -1;
    }*/

    return 1;
}