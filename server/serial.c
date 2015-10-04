#include "serial.h"
/*
* Code blatantly stolen from:
* http://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
*/


int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
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

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                printf ("error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 20;            // 0.5 seconds read timeout

        tcflush(fd, TCIFLUSH);

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                printf ("error %d setting term attributes", errno);
}

void open_serial(char *portname, char *connections) {
        char in_buf [100];
        bzero(in_buf, 100*sizeof(char));
        int n = 0;
        int n_written = 0;
        int counter = 0;
    
        unsigned char led_on[] = "0 0 LED";

	int fd = open (portname, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0)
	{
	        printf ("error %d opening %s: %s\n", errno, portname, strerror (errno));
	}

	set_interface_attribs (fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 1);

        sleep(2);

        n = read (fd, in_buf, sizeof in_buf); 

        while(1){
                counter = 0;
                while(counter < 8){
                        bzero(in_buf, sizeof(in_buf));

                        led_on[0] = '0' + counter;
                        led_on[2] = connections[counter]; //Setting STATE according to SLOTS
                        n_written = write( fd, led_on, sizeof(led_on) -1);

                        sleep(2);
                        n = read (fd, in_buf, sizeof in_buf); 
                        ++counter;
                }
        }
}
