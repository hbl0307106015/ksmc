#include "utility.h"

void* maintainer_thread(void *arg)
{
	fprintf(stdout, "thread maintainer started, pid %lu\n", pthread_self());
	
	pthread_exit(0);
}
