#ifndef MYTIMER_HEAP_H
#define MYTIMER_HEAP_H

#include"common.h"
#include"mytimer.h"

struct mytimer_heap {
	struct mytimer **timer_array;
	int capacity;
	int cur_size;
};

extern struct mytimer_heap gTimer_heap;

static inline bool mytimer_heap_is_empty()
{
	return ((gTimer_heap.cur_size) <= 0);
}

void mytimer_heap_init(int cap);

void mytimer_heap_init_by_array(struct mytimer **init_array, int size, int cap);

void mytimer_heap_destroy();

void mytimer_heap_add(struct mytimer *t);

void mytimer_heap_del(struct mytimer *t);

struct mytimer* mytimer_heap_top();

void mytimer_heap_pop();

void mytimer_heap_tick();

static void mytimer_heap_percolate_down(int hole);

static void mytimer_heap_resize();

#endif
