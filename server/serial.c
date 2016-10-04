#include "serial.h"

void set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0) {
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

    /* Setting No parity (8N1) */
    tty.c_cflag &= ~PARENB;     // No Parity
    tty.c_cflag &= ~CSTOPB;     // 1 stop bit
    tty.c_cflag &= ~CSIZE;      // See under...
    tty.c_cflag |= CS8;         // 8 bits

    /* Disable HW flow control */
    tty.c_cflag &= ~CRTSCTS;

    /* Enable the receiver and set local mode */
    tty.c_cflag |= CREAD | CLOCAL;

    /* Disable SW flow control */
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* Disable canonical input */
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
    int write_status = write(file_descriptor, query, strlen(query));
    if( write_status < 0) {
        t_print("Serial write failed\n");
        return -1;
    }

    memset(buffer, '\0', buf_len);

    /*int write_status2 = write(file_descriptor, "\xd\xa", 2);
    if( write_status2 < 0) {
        t_print("Serial write failed\n");
        return -1;
    }*/

    // Note! There has to be a sleep.
    usleep(30000);

    int counter = 0;

    while(counter < buf_len) {
        char temp[1];
        read(file_descriptor, temp, 1);
        //printf("%#x ", temp[0]);
        if(temp[0] != 0x58 && temp[0] != 0x38) {
            buffer[counter] = temp[0];
            counter++;
            if(temp[0] == 0xa) {
                return counter;
            }
        }
    }
    return counter;
}
