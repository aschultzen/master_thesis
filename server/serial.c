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
void open_serial(char *portname)
{
    clock_t begin, end;
    double time_spent;

    char buffer[1024];
    
    int fd = open (portname, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        printf ("error %d opening %s: %s\n", errno, portname, strerror (errno));
    }

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
    /* no hardware flow control */
    tty.c_cflag &= ~CRTSCTS;
    /* enable receiver, ignore status lines */
    tty.c_cflag |= CREAD | CLOCAL;
    /* disable input/output flow control, disable restart chars */
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    /* disable canonical input, disable echo,
    disable visually erase chars,
    disable terminal-generated signals */
    tty.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    /* disable output processing */
    tty.c_oflag &= ~OPOST;
    
    /* wait for 12 characters to come in before read returns */
    /* WARNING! THIS CAUSES THE read() TO BLOCK UNTIL ALL */
    /* CHARACTERS HAVE COME IN! */
    tty.c_cc[VMIN] = 0;
    /* no minimum time to wait before read returns */
    tty.c_cc[VTIME] = 0;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
        printf ("error %d setting term attributes", errno);

    //usleep(1000*1000);
   
    //tcflush(fd, TCIFLUSH);

    memset(buffer, '\0',sizeof(buffer));

    begin = clock();
    /* here, do your time-consuming job */
    while( strstr(buffer, "$GNGGA" ) == NULL && strstr(buffer, "$GNGSA" ) == NULL){
        memset(buffer, '\0',sizeof(buffer));
        read(fd, buffer, 1024);
    }

    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Time spent: %F\n", time_spent);

    printf("%s\n", buffer);
}
    

