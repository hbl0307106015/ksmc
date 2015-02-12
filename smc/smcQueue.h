#ifndef __SMC_QUEUE_H__
#define __SMC_QUEUE_H__

#include "circularQueue.h"

#define  MAX_SMC_QUEUE_NUM 3

enum {
	QUEUE_INDEX_TX_KNX = 0,
	QUEUE_INDEX_TX_ZIGBEE,
	QUEUE_INDEX_RX_GENERIC,
	QUEUE_INDEX_MAX,
} queue_index_enum;

// get queue object
struct circular_queue* smc_get_queue(uint8_t n);

// init queue
int smc_init_queue_by_index(uint8_t n);
int smc_init_queue(struct circular_queue **que);

// deinit queue
void smc_deinit_queue_by_index(uint8_t n);
void smc_deinit_queue(struct circular_queue **que);

// enqueue & dequeue
// store a packet to the queue
int smc_enqueue(struct circular_queue *que, void *d);

// retrieve a packet from the queue
void* smc_dequeue(struct circular_queue *que);

#endif /* __SMC_QUEUE_H__ */
