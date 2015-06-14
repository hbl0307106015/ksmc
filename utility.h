#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <semaphore.h>
#include <pthread.h>

extern pthread_mutex_t gSockMutex;
extern pthread_mutex_t gEvbufferMutex;
extern sem_t gEmptySem, gFullSem;

#endif /* __UTILITY_H__ */
