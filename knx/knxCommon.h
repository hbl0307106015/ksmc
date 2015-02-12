#ifndef __KNX_COMMON_H__
#define __KNX_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <signal.h>

/*
#if defined(USE_POSIX)
#include <bits/sigaction.h> // for struct sigaction
#endif
*/
#if !defined(__USE_POSIX)
#include <netinet/in.h>
typedef __in_port_t in_port_t; /* IP port type */
#endif /* ANDROID_CHANGES */

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

#include <time.h> // for time_t and time()

#include <termios.h>
#include <fcntl.h>

#include <pthread.h>

extern bool gEnableRssi;
extern bool gFlagExit;
extern uint8_t gNetworkRole;

#if (defined __USE_XOPEN_EXTENDED && !defined __USE_XOPEN2K8) \
    || defined __USE_BSD
extern int usleep (__useconds_t __useconds);
#else
extern int usleep(unsigned long usec);
#endif /* __USE_BSD */

#define BYTES32 32
#define BYTES64 64
#define BYTES128 128
#define BYTES256 256

#define BUFFER_SIZE64 BYTES64

#define MAX_NUM_FILES_DES 2
#define POLL_TIME_OUT_MS 500

/* transfer data type for baud rate */
static inline speed_t transfer_baud_rate(uint16_t int_baud_rate)
{
	switch(int_baud_rate)
	{
		case 9600:
			return B9600;
			break;
		case 19200:
			return B19200;
			break;
		default:
			return B115200;
			break;
	}
}

/* set the configuration of UART device */
void set_tty_attr(int fd, speed_t sp);

/* show the configuration of UART device */
void show_tty_attr(int fd);

#endif /* __KNX_COMMON_H__ */
