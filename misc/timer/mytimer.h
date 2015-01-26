#ifndef MYTIMER_H
#define MYTIMER_H

#include<time.h>
#include"common.h"

struct mytimer {
	time_t expire;
	void (*cb_func)(struct client_data *);
	struct client_data *user_data;
};

extern struct mytimer **gTimer;

void mytimer_init(struct mytimer *t, int delay);

#endif

