#ifndef __THREAD_APP_H__
#define __THREAD_APP_H__

#include "smcCommon.h"

#define MAX_APP_NUM 16

struct thread_app_arg {
	int sock; //socket file descriptor
	char *str_service;// port or service
};

void* smc_thread_app(void *arg);

#endif /* __THREAD_APP_H__ */

