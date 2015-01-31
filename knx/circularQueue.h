#ifndef __CIRCULAR_QUEUE_H__
#define __CIRCULAR_QUEUE_H__

#include "knxCommon.h"

#define MAX_QUEUE_SIZE 8

struct circular_queue {
	void *q[MAX_QUEUE_SIZE];
	uint16_t front;
	uint16_t rear;
	uint8_t tag;
	pthread_mutex_t qmutex;
};

//typedef circular_queue_t (struct circular_queue *);

enum {
	QUEUE_EMPTY = 0,
	QUEUE_HAS_ELEMENT,
	QUEUE_FULL,
};

static inline bool is_queue_empty(const struct circular_queue *que)
{
	return ((que->front == que->rear) && (que->tag == QUEUE_EMPTY));
}

static inline bool is_queue_full(const struct circular_queue *que)
{
	return ((que->front == que->rear) && (que->tag == QUEUE_FULL));
}

static inline uint16_t get_queue_size(const struct circular_queue *que)
{
	if (is_queue_full(que))
		return MAX_QUEUE_SIZE;
	return ((que->rear - que->front + MAX_QUEUE_SIZE) % MAX_QUEUE_SIZE);
}

struct circular_queue* circular_queue_init();

void circular_queue_destroy(struct circular_queue *que);

bool enqueue(struct circular_queue *q, void *d);

void* dequeue(struct circular_queue *q);

void circular_queue_show(const struct circular_queue *q);

#endif /* __CIRCULAR_QUEUE_H__ */
