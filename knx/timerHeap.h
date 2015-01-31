#ifndef __TIMER_HEAP_H__
#define __TIMER_HEAP_H__

#include"knxTimer.h"

struct timer_heap {
	struct knx_timer **timer_array;
	int capacity;
	int cur_size;
};

#define DEFAULT_TIMER_HEAP_SIZE 64

extern struct timer_heap gTimer_heap;

static inline bool timer_heap_is_empty()
{
	return ((gTimer_heap.cur_size) <= 0);
}

void timer_heap_init(int cap);

void timer_heap_init_by_array(struct knx_timer **init_array, int size, int cap);

void timer_heap_destroy();

void timer_heap_add(struct knx_timer *t);

void timer_heap_del(struct knx_timer *t);

struct knx_timer* timer_heap_top();

void timer_heap_pop();

void timer_heap_tick();

#endif /* __TIMER_HEAP_H__ */
