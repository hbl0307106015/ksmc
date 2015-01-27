#ifndef MYTIMER_H
#define MYTIMER_H

#include<time.h>
#include"common.h"

struct mytimer {
	time_t expire;
	void (*cb_func)(struct client_data *);
	struct client_data *user_data;
};

extern struct mytimer **gTimer; /* gTimer does not allocated memory space */
extern struct mytimer *gTimer_array[]; /* gTimer_array[] has allocated memory space */

void mytimer_init(struct mytimer *t, int delay);

#endif

