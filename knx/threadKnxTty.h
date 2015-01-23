#ifndef __THREAD_KNX_TTY_H__
#define __THREAD_KNX_TTY_H__

#include "knxCommon.h"

struct thread_knx_arg {
	int fd; // file descriptor
	speed_t baud_rate; // baud rate of UART device
};

#endif /* __THREAD_KNX_TTY_H__ */
