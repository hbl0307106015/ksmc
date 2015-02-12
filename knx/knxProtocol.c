#include "knxCommon.h"
#include "knxNetwork.h"
#include "knxProtocol.h"
#include "smcProtocol.h"

// functions
struct pkt_t* knx_protocol_alloc_pkt(size_t len)
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
int knx_protocol_store_packet(struct circular_queue **que, void *d)
{
	return knx_enqueue(*que, d);
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
