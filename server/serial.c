#include "serial.h"

volatile sig_atomic_t serial_done;

void serial_handle_sig(int signum)
{
    serial_done = 1;
}


int set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0) {
        printf ("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
        printf ("error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

void set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0) {
        printf ("error %d from tggetattr", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 20;            // 0.5 seconds read timeout

    tcflush(fd, TCIFLUSH);

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
        printf ("error %d setting term attributes", errno);
}

/* 
* As of 7.1.16 this is broken since both
* static char *serial_display_connections;
* static char *serial_display_message;
* where both removed. There is however, a new structure
* that should be usable. 
*/
void open_serial(char *portname, struct client_table_entry *ect)
{
    serial_done = 0;

    /* Registering the SIGINT handler */
    struct sigaction sigint_action;
    memset(&sigint_action, 0, sizeof(struct sigaction));
    sigint_action.sa_handler = serial_handle_sig;
    sigaction(SIGINT, &sigint_action, NULL);

    /* Registering the SIGTERM handler */
    struct sigaction sigterm_action;
    memset(&sigterm_action, 0, sizeof(struct sigaction));
    sigterm_action.sa_handler = serial_handle_sig;
    sigaction(SIGTERM, &sigterm_action, NULL);

    int fd = open (portname, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        printf ("error %d opening %s: %s\n", errno, portname, strerror (errno));
    }

    /*unsigned char led_on[] = "1 1 LED";

    char in_buf [100];
    bzero(in_buf, 100*sizeof(char));

    set_interface_attribs (fd, B9600, 0);
    set_blocking (fd, 1);

    sleep(serial_done);

    read (fd, in_buf, sizeof in_buf);
    struct client_table_entry* client_list_iterate;
    client_list_iterate = ect;

    while(!serial_done) {
        usleep(SERIAL_SLEEP);
    }*/
}
    

