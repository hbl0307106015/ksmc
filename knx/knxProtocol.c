#include "knxProtocol.h"
#include "knxCommon.h"
#include "circularQueue.h"

static struct circular_queue *gKNXRxQueue = NULL;
static struct circular_queue *gKNXTxQueue = NULL;

/* functions */
struct pkt_t* knx_protocol_alloc_pkt(size_t len)
{
	if (len <= 0)
		goto out;

	struct pkt_t *p = (struct pkt_t*)malloc(sizeof(struct pkt_t));
	if (!p)
		goto out;
	
	/* allocate the buffer for $len size */
	p->u = (uint8_t *)malloc(len);
	p->length = len;
	
	if (p->u)
		return p;

out:
	fprintf(stderr, "%s %d p or p->u allocate error\n", __func__, __LINE__);
	return NULL;
}

/* transfer data type for baud rate */
uint16_t transfer_wait_time(speed_t spd, int char_len)
{
	switch(spd)
	{
		case B9600:
			return (TIME_RX_TXD + (TIME_TXD * 3 * char_len) + TIME_TXD_IDLE);
			break;
		case B19200:
			return (TIME_RX_TXD + ((TIME_TXD + 70) * char_len) + TIME_TXD_IDLE);
			break;
		default:
			return (TIME_RX_TXD + (TIME_TXD * char_len) + TIME_TXD_IDLE);
			break;
	}
}

/* return actual user data length */
uint16_t get_user_data_length(uint16_t total_len)
{
	if (total_len < 1)
		return 0;
	if (gEnableRssi)
		return total_len - LENGTH_BEFORE_APPL_DATA - LENGTH_RSSI_INFO;;
	return total_len - LENGTH_BEFORE_APPL_DATA;
}

/* get queue object */
struct circular_queue* knx_protocol_get_queue_rx()
{
	return gKNXRxQueue;
}

struct circular_queue* knx_protocol_get_queue_tx()
{
	return gKNXTxQueue;
}

// init queue
int knx_protocol_init_queue_tx()
{
	return knx_protocol_init_queue(&gKNXTxQueue);
}

int knx_protocol_init_queue_rx()
{
	return knx_protocol_init_queue(&gKNXRxQueue);
}

int knx_protocol_init_queue(struct circular_queue **que)
{	
	if (((*que) = circular_queue_init()))
		return 0;

	fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
	return -1;
}

// deinit queue
void knx_protocol_deinit_queue_tx()
{
	return knx_protocol_deinit_queue(&gKNXTxQueue);
}

void knx_protocol_deinit_queue_rx()
{
	return knx_protocol_deinit_queue(&gKNXRxQueue);
}

void knx_protocol_deinit_queue(struct circular_queue **que)
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

// fill out the packet type structure
int knx_protocol_pkt_fill(struct pkt_t *p, unsigned char *b, size_t len)
{
	if (!p || !b) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}
	
	if (len <= 0) {
		fprintf(stderr, "%s %d, len <= 0\n", __func__, __LINE__);
		goto out;
	}
	
	memcpy(p->u, b, len);
	p->length = len;
	return 0;
out:
	return -1;
}

//store a packet to the queue
int knx_protocol_store_packet(struct circular_queue *que, void *d)
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
void* knx_protocol_retrieve_packet(struct circular_queue *que)
{
	if (!que) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		return NULL;
	}
	
	void *p = dequeue(que);
	return p;
}

#if 0
void handle_smc_packet(unsigned char *b, size_t len)
{
	/* show the received packet */
	dump_buffer(b, len);
	
	/* retrieve the header */
	size_t offset = 0;
	struct smc_proto_header *sh = (struct smc_header *)b;
	switch (sh->type)
	{
		case SMC_PROTO_KNX_SAMPLE:
			offset = sizeof(struct smc_proto_header);
			handle_smc_knx_sample(b + offset, len - offset);
			break;
		default:
			fprintf(stderr, "%s %d, unknown packet type\n", __func__, __LINE__);
			break;
	}
	
	/* add it to the txqueue */
}

void handle_smc_packet_sample(unsigned char *b, size_t len)
{
	if (!b) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		return;
	}
	
	struct circular_queue *txq = NULL;
	txq = knx_protocol_get_queue_tx();
	if (!txq) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		return;
	}
	
	struct pkt_t *p = (struct pkt_t *)malloc(sizeof(struct pkt_t));
	if (!p) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		return;
	}
	
	memcpy(p->u, b, len);
	p->length = len;
	pthread_mutex_lock(&(txq->qmutex));
	knx_protocol_store_packet(que, (void *)p);
	pthread_mutex_unlock(&(txq->qmutex));
}
#endif
