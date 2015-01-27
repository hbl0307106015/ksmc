#include"mytimer_heap.h"

void mytimer_heap_init(int cap)
{
	if(cap < 0) {
		fprintf(stderr, "error: cap < 0");
		return;
	}
	
	gTimer_heap.cur_size = 0;
	gTimer_heap.capacity = cap;
	gTimer_heap.timer_array = (struct mytimer *)malloc(sizeof(struct mytimer *) * cap);
	
	if (!gTimer_heap.timer_array) {
		perror("mytimer_heap_init");
		exit(0);
	}
	
	int i;
	for (i = 0; i < cap; i++)
		gTimer_heap.timer_array[i] = NULL;
}

void mytimer_heap_init_by_array(struct mytimer **init_array, int size, int cap)
{
	if(cap < size) {
		fprintf(stderr, "error: cap < size");
		exit(0);
	}
	
	gTimer_heap.cur_size = size;
	gTimer_heap.capacity = cap;
	gTimer_heap.timer_array = (struct mytimer *)malloc(sizeof(struct mytimer *) * gTimer_heap.capacity);
	
	if (!gTimer_heap.timer_array) {
		perror("mytimer_heap_init_by_array");
		exit(0);
	}
	
	int i;
	for (i = 0; i < gTimer_heap.capacity; i++)
		gTimer_heap.timer_array[i] = NULL;

	if (size > 0) {
		for (i = 0; i < gTimer_heap.cur_size; i++) {
			if (init_array[i]) {
				gTimer_heap.timer_array[i] = init_array[i];
				fprintf(stdout, "init %d expire=%d buf=%s\n", i, init_array[i]->expire, init_array[i]->user_data->buf);
			} else {
				fprintf(stderr, "init_array[%d] is NULL\n", i);
				exit(0);
			}
		}
	}
	
	for (i = ((gTimer_heap.cur_size - 1)/2); i >= 0; --i) {
		fprintf(stdout, "percolate_down %i...\n", i);
		mytimer_heap_percolate_down(i);
		fprintf(stdout, "percolate_down %i done\n", i);
	}
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
	
	t->valid = false;
	t->cb_func == NULL;
}

struct mytimer* mytimer_heap_top()
{
	if (mytimer_heap_is_empty()) {
		fprintf(stderr, "error: top t==empty\n");
		return NULL;
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
		gTimer_heap.timer_array[0] = gTimer_heap.timer_array[--(gTimer_heap.cur_size)];
		mytimer_heap_percolate_down(0);
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
		
		if (gTimer_heap.timer_array[0]->cb_func)
			gTimer_heap.timer_array[0]->cb_func(gTimer_heap.timer_array[0]->user_data);
			
		mytimer_heap_pop();
		tmp = gTimer_heap.timer_array[0];
	}
}

static void mytimer_heap_percolate_down(int hole)
{
	if (hole < 0) {
		fprintf(stderr, "error: percolate i < 0\n");
		return;
	}
	
	if (gTimer_heap.cur_size < 2) {
		fprintf(stderr, "warning: percolate cur_size < 2\n");
		return;
	}
	
	struct mytimer *temp = gTimer_heap.timer_array[hole];
	int child = 0;
	
	// the condition for stop is that hole node should be the leaf of the btree
	for(; (hole * 2 + 1) <= (gTimer_heap.cur_size - 1); hole = child)
	{
		child = (hole * 2 + 1);//get the left child
		
		//choose the smaller child node
		if ((child < (gTimer_heap.cur_size - 1)) &&\
			(gTimer_heap.timer_array[child + 1]->expire < gTimer_heap.timer_array[child]->expire))
				child++;

		if(gTimer_heap.timer_array[child]->expire < temp->expire)
			gTimer_heap.timer_array[hole] = gTimer_heap.timer_array[child];//move the smallest to the hole
		else
			break;
	}
	
	gTimer_heap.timer_array[hole] = temp;
}

static void mytimer_heap_resize()
{
	struct mytimer **temp = (struct mytimer *)malloc(sizeof(struct mytimer *) * gTimer_heap.capacity * 2);

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
