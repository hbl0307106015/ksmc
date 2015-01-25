#ifndef TIMER_H
#define TIMER_H

#include<time.h>
#include"common.h"

struct mytimer {
	time_t expire;
	void (*cb_func)(struct client_data *);
	struct client_data *user_data;
};

mytimer gTimer;

void mytimer_init(int delay);
{
	gTimer.expire = time(NULL) + delay;
}

#endif

