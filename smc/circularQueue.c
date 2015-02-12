#include "circularQueue.h"

struct circular_queue* circular_queue_init()
{
	struct circular_queue *que = \
		(struct circular_queue *)malloc(sizeof(struct circular_queue));
	if (!que) {
		fprintf(stdout, "queue init, memory out\n");
		exit(0);
	}
	
	memset(que, 0, sizeof(struct circular_queue));
	que->front = 0;
	que->rear = 0;
	que->tag = QUEUE_EMPTY;
	pthread_mutex_init(&(que->qmutex), NULL);
	
	return que;
}

void circular_queue_destroy(struct circular_queue *que)
{
	if (!que) {
		fprintf(stdout, "queue destroy, null pointer\n");
		return;
	}
	
	pthread_mutex_destroy(&(que->qmutex));
	free(que);
}

bool enqueue(struct circular_queue *que, void *d)
{
	if (is_queue_full(que)) {
		fprintf(stdout, "enqueue, queue is full\n");
		return false;
	}

	que->q[que->rear] = d;
	que->rear = (que->rear + 1) % MAX_QUEUE_SIZE;

	if ((que->front == que->rear) && (que->tag == QUEUE_HAS_ELEMENT)) {
		que->tag = QUEUE_FULL;
	} else if(que->tag == QUEUE_EMPTY)
		que->tag = QUEUE_HAS_ELEMENT;

	return true;
}

void* dequeue(struct circular_queue *que)
{
	if (is_queue_empty(que)) {
		fprintf(stdout, "dequeue, queue is empty\n");
		return false;
	}
	
	struct data *p = NULL;
	
	p = que->q[(que->front)];
	que->front = (que->front + 1) % MAX_QUEUE_SIZE;
	
	if ((que->front == que->rear) && (que->tag == QUEUE_HAS_ELEMENT))
		que->tag = QUEUE_EMPTY;
	else if (que->tag == QUEUE_FULL)
		que->tag = QUEUE_HAS_ELEMENT;
	
	return p;
}

void circular_queue_show(const struct circular_queue *que)
{
	if (!que) {
		fprintf(stdout, "queue show, null pointer\n");
		goto out;
	}
	
	if (is_queue_empty(que)) {
		fprintf(stdout, "queue show, queue is empty\n");
		goto out;
	}
	
out:
	return;
}
