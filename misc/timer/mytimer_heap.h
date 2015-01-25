#ifndef MYTIMER_HEAP_H
#define MYTIMER_HEAP_H

#include"common.h"
#include"mytimer.h"
#include"mytimer_heap.h"

static mytimer_heap {
	struct mytimer **timer_array;
	int capacity;
	int cur_size;
};

struct mytimer_heap gTimer_heap;

void mytimer_heap_init(int cap)
{
	gTimer_heap.cur_size = 0;
	gTimer_heap.capacity = cap;
	gTimer_heap.timer_array = (struct mytimer_heap *)malloc(sizeof(struct mytimer_heap *) * cap);
	
	if (!gTimer_heap.timer_array) {
		perror("mytimer_heap_init");
		exit(0);
	}
	
	int i;
	for (i = 0; i < cap; i++)
		gTimer_heap.timer_array[i] = NULL;
}

void mytimer_heap_init_by_array(struct mytimer_heap ** init_array, int size, int cap)
{
	if(cap < size) {
		fprintf(stderr, "error: cap < size");
		return;
	}
	
	gTimer_heap.cur_size = size;
	gTimer_heap.capacity = cap;
	gTimer_heap.timer_array = (struct mytimer_heap *)malloc(sizeof(struct mytimer_heap *) * cap);
	
	if (!gTimer_heap.timer_array) {
		perror("mytimer_heap_init");
		exit(0);
	}
	
	int i;
	for (i = 0; i < cap; i++)
		gTimer_heap.timer_array[i] = NULL;
		
	if (size > 0) {
		for (i = 0; i < size; i++)
			gTimer_heap.timer_array[i] = init_array[i];
	}
	
	for (i = ((size - 1)/2); i >= 0; --i)
		mytimer_heap_percolate_down(i);
}

void mytimer_heap_destroy()
{
	int i;
	for (i = 0; i < gTimer_heap.cur_size; i++)
		free(gTimer_heap.timer_array[i]);
}

void mytimer_heap_add(struct mytimer *t)
{
	if (!t) {
		fprintf(stderr, "error: add_timer t==NULL\n");
		return;
	}
	
	if (gTimer_heap.cur_size >= gTimer_heap.capacity)
		mytimer_heap_resize();
		
	int hole = gTimer_heap.cur_size++;
	int parent = 0;
	
	for (; hole > 0; hole = parent) {
		
		parent = (hole - 1)/2;
		
		if (gTimer_heap.timer_array[parent]->expire <= t->expire)
			break;
		
		gTimer_heap.timer_array[hole] = gTimer_heap.timer_array[parent];
	}
	
	gTimer_heap.timer_array[hole] = t;
}

void mytimer_heap_del(struct mytimer *t)
{
	if (!t) {
		fprintf(stderr, "error: del_timer t==NULL\n");
		return;
	}
	
	t->cb_func == NULL;
}

void mytimer_heap_top() const
{
	if (mytimer_heap_is_empty()) {
		fprintf(stderr, "error: top t==empty\n");
		return;
	}
	
	return gTimer_heap.timer_array[0];
}

void mytimer_heap_pop()
{
	if (mytimer_heap_is_empty()) {
		fprintf(stderr, "error: top t==empty\n");
		return;
	}
	
	if (gTimer_heap.timer_array[0]) {
		free(gTimer_heap.timer_array[0]);
		if (gTimer_heap.cur_size > 1) {
			gTimer_heap.timer_array[0] = gTimer_heap.timer_array[--(gTimer_heap.cur_size)];
			mytimer_heap_percolate(0);
		}
	}
}

void mytimer_heap_tick()
{
	struct mytimer *tmp = gTimer_heap.timer_array[0];
	time_t cur = time(NULL);
	
	while (!mytimer_heap_is_empty())
	{
		if (!tmp)
			break;
		
		if (tmp->expire > cur)
			break;
		
		if (gTimer_heap.timer_array[0].cb_func)
			gTimer_heap.timer_array[0].cb_func(gTimer_heap.timer_array[0].user_data);
			
		mytimer_heap_pop();
		tmp = gTimer_heap.timer_array[0];
	}
}

bool mytimer_heap_is_empty()
{
	return (gTimer_heap.cur_size <= 0);
}

static void mytimer_heap_percolate_down(int hole)
{
	if (hole < 0) {
		fprintf(stderr, "error: percolate i<0\n");
		return;
	}
	
	if (gTimer_heap.cur_size < 2) {
		fprintf(stderr, "error: percolate cur_size < 2\n");
		return;
	}
	
	struct mytimer *temp = gTimer_heap.timer_array[hole];
	int child = 0;
	
	for(; (hole * 2 + 1) <= (gTimer_heap.cur_size - 1); hole = child)
	{
		child = (hole * 2 + 1);
		
		if ((child < (gTimer_heap.cur_size - 1)) &&\
			(gTimer_heap.timer_array[child + 1]->expire < gTimer_heap.timer_array[child]->expire))
				child++;
				
		if(gTimer_heap.timer_array[child]->expire < temp->expire)
			gTimer_heap.timer_array[hole] = gTimer_heap.timer_array[child];
		else
			break;
	}
	
	gTimer_heap.timer_array[hole] = temp;
}

static void mytimer_heap_resize()
{
	struct mytimer **temp = (struct mytimer_heap *)malloc(sizeof(struct mytimer_heap *) * gTimer_heap.capacity * 2);

	if (!temp) {
		fprintf(stderr, "error: resize temp==NULL\n");		
		exit(0);
	}

	int i;
	for (i = 0; i < (2 * gTimer_heap.capacity); ++i)
		temp[i] = NULL;
	
	gTimer_heap.capacity *= 2;
	
	for (i = 0; i < (gTimer_heap.cur_size); ++i) // maybe we can use realloc() here ?
		temp[i] = gTimer_heap.timer_array[i];

	free(gTimer_heap.timer_array);
	gTimer_heap.timer_array = temp;
}

#endif

