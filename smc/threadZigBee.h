#ifndef __THREAD_ZIGBEE_H__
#define __THREAD_ZIGBEE_H__

void* smc_thread_zigbee(void *arg);

struct thread_zigbee_arg {
	int sock;
	char *str_service;// port or service
};

#endif /* __THREAD_ZIGBEE_H__ */
