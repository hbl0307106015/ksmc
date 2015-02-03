#ifndef __THREAD_APP_H__
#define __THREAD_APP_H__

#include "smcCommon.h"


#define MAX_APP_NUM 16

struct network_ctx {
	int fd; //socket file descriptor
	int flags; // flags, mostly is 0
	struct sockaddr *addr; //socket address structure
	socklen_t addr_len; // socket address length
};

struct thread_app_arg {
	int sock; //socket file descriptor
};


extern struct network_ctx app_manager[MAX_APP_NUM];

void* smc_thread_app(void *arg);

#endif /* __THREAD_APP_H__ */

