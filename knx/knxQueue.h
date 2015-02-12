#ifndef __KNX_QUEUE_H__
#define __KNX_QUEUE_H__

#include "circularQueue.h"

#define MAX_KNX_QUEUE_NUM 2

enum {
	QUEUE_INDEX_KNX_RX = 0,
	QUEUE_INDEX_KNX_TX,
	QUEUE_INDEX_MAX,
} knx_queue_enum;

// get queue object
#if 0
struct circular_queue* knx_protocol_get_queue_rx();
struct circular_queue* knx_protocol_get_queue_tx();
#endif
struct circular_queue* knx_get_queue(uint8_t n);

// init queue
#if 0
int knx_protocol_init_queue_tx();
int knx_protocol_init_queue_rx();
#endif
int knx_init_queue_by_index(uint8_t n);
int knx_init_queue(struct circular_queue **que);

// deinit queue
#if 0
void knx_protocol_deinit_queue_tx();
void knx_protocol_deinit_queue_rx();
#endif
void knx_deinit_queue_by_index(uint8_t n);
void knx_deinit_queue(struct circular_queue **que);

void* knx_dequeue(struct circular_queue *que);
int knx_enqueue(struct circular_queue *que, void *d);


#endif /* __KNX_QUEUE_H__ */
