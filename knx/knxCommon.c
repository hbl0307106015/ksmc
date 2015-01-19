#include "knxCommon.h"

/* set the configuration of UART device */
void set_tty_attr(int fd, speed_t sp)
{
	struct termios new_to;
    memset(&new_to, 0, sizeof(new_to));
    new_to.c_cflag |= (CLOCAL | CREAD); /* Ignore modem control lines, enable receiver */
    new_to.c_cflag &= (~CSIZE); /* Clear character size mask.  Values are CS5, CS6, CS7, or CS8. */
    
    new_to.c_cflag |= CS8; /* 8 data bit */
    new_to.c_cflag &= (~PARENB); /* no parity */
    new_to.c_cflag &= (~CSTOPB); /* 1 stop bit */
    
    /* none check bit */
    #if defined(_BSD_SOURCE) || defined(_SVID_SOURCE)
    new_to.c_cflag &= (~CRTSCTS);
    #endif /* defined(_BSD_SOURCE) || defined(_SVID_SOURCE) */
	new_to.c_iflag &= (~IXON);
    new_to.c_iflag &= (~IXOFF);
    
    new_to.c_cc[VTIME] = 0; /* minimum wait time */
    new_to.c_cc[VMIN] = 0; /* minimum of received packet length */
    
    /* set baudrate as 'sp' specifies */
    cfsetispeed(&new_to, sp);
    cfsetospeed(&new_to, sp);
    
    /* apply the setting right now */
    if (tcsetattr(fd, TCSANOW, &new_to) != 0) {
        perror("tcsetattr error");
        exit(1);
    }	
}

/* show the configuration of UART device */
void show_tty_attr(int fd)
{
	struct termios to;
    memset(&to, 0, sizeof(to));
    tcgetattr(fd, &to);
    speed_t sp_o = cfgetospeed(&to);
    speed_t sp_i = cfgetispeed(&to);
    printf("output baud rate output is %u\n", sp_o);
    printf("input baud rate input is %u\n", sp_i);
    printf("data bit: %u\n", (to.c_cflag) & CS8);
    printf("stop bit: %u\n", (to.c_cflag) & CSTOPB);
    printf("parity 1 bit: %u\n", (to.c_cflag) & PARENB);
    printf("parity 2 bit: %u\n", (to.c_cflag) & PARODD);
    #if defined(_BSD_SOURCE) || defined(_SVID_SOURCE)
    printf("RTS/CTS: %u\n", (to.c_cflag) & CRTSCTS);
	#endif /* defined(_BSD_SOURCE) || defined(_SVID_SOURCE) */
    printf("flow control xon: %u\n", (to.c_iflag) & IXON);
    printf("flow control xoff: %u\n", (to.c_iflag) & IXOFF);
    printf("if enable receiver: %u\n", (to.c_cflag) & CREAD);
	printf("VTIME & VMIN %u %u\n", to.c_cc[VTIME], to.c_cc[VMIN]);
}
