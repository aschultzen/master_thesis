#include "serial.h"

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

int open_serial(char *portname, serial_device device)
{
    int fd = open (portname, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        t_print ("Error %d opening %s: %s\n", errno, portname, strerror (errno));
    }

    if(device == GPS) {
        if(configure_gps_serial(fd) < 0) {
            exit(0);
        }
    }

    return fd;
}



