#include "serial.h"

int set_interface_attribs (int fd, int speed, int parity, struct termios *tty)
{
    if (tcgetattr (fd, tty) != 0) {
        printf ("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (tty, speed);
    cfsetispeed (tty, speed);

    tty->c_cflag = (tty->c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty->c_iflag &= ~IGNBRK;         // disable break processing
    tty->c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty->c_oflag = 0;                // no remapping, no delays
    tty->c_cc[VMIN]  = 0;            // read doesn't block
    tty->c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty->c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty->c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    tty->c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty->c_cflag |= parity;
    tty->c_cflag &= ~CSTOPB;
    tty->c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, tty) != 0) {
        printf ("error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

void set_blocking (int fd, int should_block, struct termios *tty)
{
    if (tcgetattr (fd, tty) != 0) {
        printf ("error %d from tggetattr", errno);
        return;
    }

    tty->c_cc[VMIN]  = should_block ? 1 : 0;
    tty->c_cc[VTIME] = 20;            // 0.5 seconds read timeout

    tcflush(fd, TCIFLUSH);

    if (tcsetattr (fd, TCSANOW, tty) != 0)
        printf ("error %d setting term attributes", errno);
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


