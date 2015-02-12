#ifndef __SMC_COMMON_H__
#define __SMC_COMMON_H__

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>

#include<getopt.h>
#include<netdb.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#if defined(__USE_POSIX)
#include<ifaddrs.h>
#endif

#include<net/if.h>
#include<sys/ioctl.h>
#include<netinet/in.h>

#include<poll.h>
#include<pthread.h>

extern char *gIface;
extern char *gIPstr;
extern volatile bool gDoExit;


#if !defined(__USE_POSIX)
typedef __in_port_t in_port_t; /* IP port type */
#endif /* ANDROID_CHANGES */



#define BUFFER_SIZE64 64
#define BUFFER_SIZE128 128
#define BUFFER_SIZE256 256

#define MAX_STRING_LENGTH BUFFER_SIZE256





#endif /* __SMC_COMMON_H__ */
