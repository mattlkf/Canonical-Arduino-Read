/*  
	A fork of CanonicalArduinoRead by Chris Heydrick
	www.chrisheydrick.com
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>

#define DEBUG 1

int setup(int * pfd, char * tty_dev, int baud){
    struct termios toptions;

    /* open serial port */
    *pfd = open(tty_dev, O_RDWR | O_NOCTTY);
    printf("%s opened as %i\n", tty_dev, *pfd);

    /* wait for the Arduino to reboot */
    usleep(3500000);

    /* Choose the right baud rate identifier */
    speed_t br;
	switch (baud) {
    case 9600:   br = B9600;   break;
    case 19200:  br = B19200;  break;
    case 38400:  br = B38400;  break;
    case 57600:  br = B57600;  break;
    case 115200: br = B115200; break;
    default:	 
    	printf("Unknown baud rate\n");
    	printf("Using default of 9600\n");
    	br = B9600;
    	break;
    }
    printf("Baud rate %d\n", baud);

    /* get current serial port settings */
    tcgetattr(*pfd, &toptions);
    /* set baud rate both ways */
    cfsetispeed(&toptions, br);
    cfsetospeed(&toptions, br);
    /* 8 bits, no parity, no stop bits */
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    /* Canonical mode */
    /* i.e. line buffering - chars will not come through until a \n is sent */
    // toptions.c_lflag |= ICANON;
    /* Non-canonical mode */
    toptions.c_lflag &= ~ICANON;
    /* commit the serial port settings */
    tcsetattr(*pfd, TCSANOW, &toptions);

}

int available(const int fd){
    int n = 0;
    ioctl(fd, FIONREAD, &n);

    return n;
}

int main(int argc, char *argv[]){

    int fd, n, i;
    char buf[64] = "temp text";

    if(argc < 3){
    	printf("Usage: %s <tty> <baudrate>\n", argv[0]);
    	return 0;
    }

    setup(&fd, argv[1], atoi(argv[2]));

    //Send initial character to begin back-and-forth
    write(fd, "Z", 1);
    //The arduino will respond with the next character
    //of the alphabet (wrapped)

    while(1){
    	if(available(fd)){
	    	n = read(fd, buf, 63); //read up to 63 chars
	    	buf[n] = 0; //set null terminator
	    	printf("%d bytes: %s\n", n, buf);
	    	write(fd, buf, 1);
    	}
    }

    return 0;
}
