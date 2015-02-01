#ifndef __KNX_TIMER_H__
#define __KNX_TIMER_H__

#include"knxCommon.h"

struct knx_timer_data {
	int fd;
	char buf[BUFFER_SIZE64];
};

struct knx_timer {
	bool valid;
	time_t expire;
	void (*cb_func)(void *);
	void *user_data;
};

extern struct knx_timer **gTimer; /* gTimer does not allocated memory space */
extern struct knx_timer *gTimer_array[]; /* gTimer_array[] has allocated memory space */

static inline bool is_timer_valid(struct knx_timer *t)
{
	return (t->valid == true);
}

struct knx_timer* knx_timer_alloc(size_t size);

#endif /* __KNX_TIMER_H__ */
