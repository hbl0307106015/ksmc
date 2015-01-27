#include "common.h"
#include "mytimer.h"
#include "mytimer_heap.h"


struct mytimer **gTimer;// for method 1
struct mytimer *gTimer_array[]; // for method 2
struct mytimer_heap gTimer_heap;

void timeout_cb_func(struct client_data *arg);

int main(int argc, char *argv[])
{
	#if 1
	/* method 1: start with existed array */
	int i;
	gTimer = (struct mytimer *)malloc(sizeof(struct mytimer *) * 3);
	for (i = 0; i < 3; i++) {
		gTimer[i] = malloc(sizeof(struct mytimer));
		memset(gTimer[i], 0, sizeof(struct mytimer));
	}
	
	gTimer[0]->cb_func = timeout_cb_func;
	gTimer[1]->cb_func = timeout_cb_func;
	gTimer[2]->cb_func = timeout_cb_func;
	mytimer_init(gTimer[0], 1);
	mytimer_init(gTimer[1], 12);
	mytimer_init(gTimer[2], 6);
	
	mytimer_heap_init_by_array(gTimer, 3, 6);
	#endif
	
	#if 0
	/* method 2: start from scratch */
	mytimer_heap_init(6);
	
	int i;
	for (i = 0; i < 6; i++) {
		gTimer_array[i] = malloc(sizeof(struct mytimer));
		memset(gTimer_array[i], 0, sizeof(struct mytimer));
		
		gTimer_array[i]->cb_func = timeout_cb_func;
		mytimer_init(gTimer_array[i], (i + 1) * 2);
		
		mytimer_heap_add(gTimer_array[i]);
	}
	#endif
	
	/* the main loop */
	time_t interval = 0;
	struct mytimer *t = NULL;
	while (!mytimer_heap_is_empty())
	{
		t = mytimer_heap_top();
		if (!t) {
			fprintf(stdout, "heap is empty now");
			break;
		}
		
		interval = t->expire - time(NULL);
		if (interval > 0)
			sleep(interval);

		mytimer_heap_tick();
	}
	
	return 0;
}

void timeout_cb_func(struct client_data *arg)
{
	if (arg) {
		fprintf(stdout, "%s %u\n", \
			arg->buf, time(NULL));
	}
}
