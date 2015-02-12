#include "smcQueue.h"

static struct circular_queue *gQueue[MAX_SMC_QUEUE_NUM] = {
	[QUEUE_INDEX_TX_KNX] = NULL,
	[QUEUE_INDEX_TX_ZIGBEE] = NULL,
	[QUEUE_INDEX_RX_GENERIC] = NULL
};

// get queue object
struct circular_queue* smc_get_queue(uint8_t n)
{
	if (n > QUEUE_INDEX_MAX)
		goto out;
		
	return gQueue[n];

out:
	fprintf(stdout, "%s %d, index overflow\n", __func__, __LINE__);
	return NULL;
}

// init queue
int smc_init_queue_by_index(uint8_t n)
{
	return smc_init_queue(&(gQueue[n]));
}

int smc_init_queue(struct circular_queue **que)
{	
	if (((*que) = circular_queue_init()))
		return 0;

	fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
	return -1;
}

// deinit queue
void smc_deinit_queue_by_index(uint8_t n)
{
	return smc_deinit_queue(&(gQueue[n]));
}

void smc_deinit_queue(struct circular_queue **que)
{
	if (!(*que)) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}
	
	/* release the element if exist */
	if (is_queue_empty(*que))
		goto out;

	uint16_t i = 0, qsize = get_queue_size(*que);

	do {
		free((*que)->q[((*que)->front + i) % MAX_QUEUE_SIZE]);
	} while (++i < qsize);

	/* free the queue */
	circular_queue_destroy(*que);

out:
	return;
}

// store a packet to the queue
int smc_enqueue(struct circular_queue *que, void *d)
{
	if ((!que) || (!d)) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		return -1;
	}

	if (enqueue(que, d))
		return 0;

	return -1;
}

// retrieve a packet from the queue
void* smc_dequeue(struct circular_queue *que)
{
	if (!que) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		return NULL;
	}
	
	void *p = dequeue(que);
	return p;
}







