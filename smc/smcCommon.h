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

#include<poll.h>
#include<pthread.h>

extern volatile bool gDoExit;




#define BUFFER_SIZE64 64
#define BUFFER_SIZE128 128
#define BUFFER_SIZE256 256

#define MAX_STRING_LENGTH BUFFER_SIZE256

#define MAX_NUM_POLL_FILES_DES 2
#define POLL_TIME_OUT_MS 500
#define MAX_UDP_PAYLOAD_SIZE 65507



#endif /* __SMC_COMMON_H__ */
