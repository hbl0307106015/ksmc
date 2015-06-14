#include "utility.h"

void* handler_thread(void *arg)
{
	fprintf(stdout, "thread handler started, pid %lu\n", pthread_self());
	
	// retrieve packet from the evbuffer
	
	// check packet header which indicates length
	pthread_exit(0);
}
