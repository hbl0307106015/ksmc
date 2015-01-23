#ifndef __THREAD_SOCKET_H__
#define __THREAD_SOCKET_H__

#include "knxCommon.h"

#define DEFAULT_SERVER_ADDRESS "127.0.0.1"
#define MAX_STRING_LENGTH 255

struct thread_socket_arg {
	char *port; // server port/service
	char *server; // server address/name
};

#endif /* __THREAD_SOCKET_H__ */
