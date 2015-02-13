#include "knxQueue.h"
#include "knxCommon.h"
#include "knxNetwork.h"
#include "knxProtocol.h"
#include "smcProtocol.h"

// functions
struct pkt_t* knx_protocol_alloc_pkt(ssize_t len)
{
	if (len <= 0)
		goto out;

	struct pkt_t *p = (struct pkt_t*)malloc(sizeof(struct pkt_t));
	if (!p)
		goto out;
	
	// allocate the buffer for $len size
	p->u = (uint8_t *)malloc(len);
	p->length = len;
	
	if (p->u)
		return p;

out:
	fprintf(stderr, "%s %d p or p->u allocate error\n", __func__, __LINE__);
	return NULL;
}

// fill out the packet type structure
int knx_protocol_fill_pkt(struct pkt_t *p, unsigned char *b, size_t len)
{
	if (!p || !b) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}
	
	if (len <= 0) {
		fprintf(stderr, "%s %d, len <= 0\n", __func__, __LINE__);
		goto out;
	}
	
	uint8_t *buffer = p->u;
	knx_store_header(&buffer, SMC_KNX_PACKET_TYPE);
	memcpy(buffer, b, len);
	p->length = len + sizeof(uint16_t);
	return 0;
out:
	return -1;
}

// transfer data type for baud rate
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

// return actual user data length
uint16_t get_user_data_length(uint16_t total_len)
{
	if (total_len < 1)
		return 0;
	if (gEnableRssi)
		return total_len - LENGTH_BEFORE_APPL_DATA - LENGTH_RSSI_INFO;;
	return total_len - LENGTH_BEFORE_APPL_DATA;
}

// store a packet to the queue
#if 0
int knx_protocol_store_packet(struct circular_queue **que, void *d)
{
	return knx_enqueue(*que, d);
}
#endif

int knx_protocol_store_packet(uint8_t qid, uint8_t *b, ssize_t buffer_len)
{
	if (qid > QUEUE_INDEX_MAX || !b) {
		fprintf(stderr, "%s %d, parameter error, qid or buffer pointer wrong!", __func__, __LINE__);
		goto out;
	}
	
	struct pkt_t *pkt = NULL;
	struct circular_queue *rxq = NULL;
	
	rxq = knx_get_queue(qid);
	if (!rxq) {
		fprintf(stdout, "%s %d, null pointer", __func__, __LINE__);
		goto out;
	}
	
	if (is_queue_full(rxq)) {
		fprintf(stdout, "%s %d, uart knx queue is full", __func__, __LINE__);
		goto out;
	}
	
	pkt = knx_protocol_alloc_pkt(buffer_len);
	if (!pkt) {
		fprintf(stdout, "%s %d, memory out", __func__, __LINE__);
		goto out;
	}
	
	pkt = knx_protocol_fill_pkt(p, buffer, buffer_len);
	dump_buffer(buffer, buffer_len);

	pthread_mutex_lock(&(rxq->qmutex));
	//knx store packet in queue
	enqueue...(rxq, (void *)pkt);
	pthread_mutes_unlock(&(rxq->qmutex));
	
out:
	return;
}

// retrieve a packet from the queue
void* knx_protocol_retrieve_packet(struct circular_queue **que)
{
	return knx_dequeue(*que);
}


uint16_t knx_retrieve_header(uint8_t **b)
{
	return knx_retrieve_bytes16(b);
}

void knx_store_header(uint8_t **b, uint16_t t)
{
	return knx_store_bytes16(b, t);
}
