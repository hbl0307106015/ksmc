#include "common.h"
#include "mytimer.h"
#include "mytimer_heap.h"


struct mytimer **gTimer;
struct mytimer_heap gTimer_heap;

void timeout_cb_func(struct client_data *arg);

int main(int argc, char *argv[])
{
	/* init timer */
	int i;
	gTimer = (struct mytimer *)malloc(sizeof(struct mytimer *) * 3);
	for (i = 0; i < 3; i++) {
		gTimer[i] = malloc(sizeof(struct mytimer));
		memset(gTimer[i], 0, sizeof(struct mytimer));
	}

	#if 1
	gTimer[0]->cb_func = timeout_cb_func;
	gTimer[1]->cb_func = timeout_cb_func;
	gTimer[2]->cb_func = timeout_cb_func;
	mytimer_init(gTimer[0], 1);
	mytimer_init(gTimer[1], 12);
	mytimer_init(gTimer[2], 6);
	#endif
	
	/* init heap */
	/* 1 : start from scratch */
	mytimer_heap_init_by_array(gTimer, 3, 6);
	
	#if 0
	gTimer_heap.timer_array[0]->cb_func = timeout_cb_func;
	gTimer_heap.timer_array[1]->cb_func = timeout_cb_func;
	gTimer_heap.timer_array[2]->cb_func = timeout_cb_func;
	mytimer_init(gTimer_heap.timer_array[0], 10);
	mytimer_init(gTimer_heap.timer_array[1], 20);
	mytimer_init(gTimer_heap.timer_array[2], 30);
	#endif

	/* add timer */
	#if 0
	mytimer_heap_add(&gTimer[0]);
	mytimer_heap_add(&gTimer[1]);
	mytimer_heap_add(&gTimer[2]);
	#endif

	/* 2 : start by existed array-gTimer */

	

	
	/* wait for a moment */
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
