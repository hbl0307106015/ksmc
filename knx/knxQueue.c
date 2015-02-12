#include "knxCommon.h"
#include "knxQueue.h"

static struct circular_queue *gQueue[MAX_KNX_QUEUE_NUM] = {
	[QUEUE_INDEX_KNX_RX] = NULL,
	[QUEUE_INDEX_KNX_TX] = NULL
};

// get queue object
#if 0
struct circular_queue* knx_protocol_get_queue_rx()
{
	return gKNXRxQueue;
}

struct circular_queue* knx_protocol_get_queue_tx()
{
	return gKNXTxQueue;
}
#endif
struct circular_queue* knx_get_queue(uint8_t n)
{
	if (n > QUEUE_INDEX_MAX)
		goto out;

	return (gQueue[n]);
	
out:
	fprintf(stdout, "%s %d, index overflow\n", __func__, __LINE__);
	return NULL;
}


// init queue
#if 0
int knx_protocol_init_queue_tx()
{
	return knx_protocol_init_queue(&gKNXTxQueue);
}

int knx_protocol_init_queue_rx()
{
	return knx_protocol_init_queue(&gKNXRxQueue);
}
#endif
int knx_init_queue_by_index(uint8_t n)
{
	return knx_init_queue(&(gQueue[n]));
}

int knx_init_queue(struct circular_queue **que)
{	
	if (((*que) = circular_queue_init()))
		return 0;

	fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
	return -1;
}

// deinit queue
#if 0
void knx_protocol_deinit_queue_tx()
{
	return knx_protocol_deinit_queue(&gKNXTxQueue);
}

void knx_protocol_deinit_queue_rx()
{
	return knx_protocol_deinit_queue(&gKNXRxQueue);
}
#endif
void knx_deinit_queue_by_index(uint8_t n)
{
	return knx_deinit_queue(&(gQueue[n]));
}

void knx_deinit_queue(struct circular_queue **que)
{
	if (!(*que)) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}
	
	// release the element if exist
	if (is_queue_empty(*que))
		goto out;

	uint16_t i = 0, qsize = get_queue_size(*que);

	do {
		free((*que)->q[((*que)->front + i) % MAX_QUEUE_SIZE]);
	} while (++i < qsize);

	// free the queue
	circular_queue_destroy(*que);

out:
	return;
}

void* knx_dequeue(struct circular_queue *que)
{
	if (!que) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		return NULL;
	}
	
	void *p = dequeue(que);
	return p;	
}

int knx_enqueue(struct circular_queue *que, void *d)
{
	if ((!que) || (!d)) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		return -1;
	}

	if (enqueue(que, d))
		return 0;

	return -1;	
}






