#ifndef __THREAD_KNX_H__
#define __THREAD_KNX_H__

struct thread_knx_arg {
	int sock; //socket file descriptor
	char *str_service;// port or service
	struct peer_real_time_info *knx_client_info; //real time information structure
};

void* smc_thread_knx(void *arg);


#endif /* __THREAD_KNX_H__ */
