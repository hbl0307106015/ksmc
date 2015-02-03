
#include "knxTimer.h"
#include "knxCommon.h"
#include "timerHeap.h"
#include "circularQueue.h"

void* handle_timer(void *arg)
{
	time_t interval = 0;
	struct knx_timer *t = NULL;
	
	fprintf(stdout, "enter threadTimer mainloop\n");
	do {
		
		t = timer_heap_top();
		if (!t) {
			fprintf(stdout, "heap is empty now\n");
			goto next_turn;
		}
		
		if (!is_timer_valid(t)) {
			timer_heap_pop();
			fprintf(stdout, "timer is invalid\n");
			goto next_turn;
		}
		
		interval = t->expire - time(NULL);
		if (interval > 0)
			sleep(interval);

		timer_heap_tick();
	
	next_turn:
		sleep(1);
	} while(gFlagExit != true);


	return (void *)NULL;
}
