#include "utility.h"

void* receiver_thread(void *arg)
{
	fprintf(stdout, "thread receiver started, pid %lu\n", pthread_self());
	// reveive the packet from server
	
	// put the packet into evbuffer
	
	pthread_exit(0);
}
