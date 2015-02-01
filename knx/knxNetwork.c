#include "log.h"
#include "knxNetwork.h"
#include "knxProtocol.h"

void knx_store_raw_bytes(uint8_t **b, const uint8_t *r, const size_t len)
{
	memcpy((*b), r, len);
	(*b) += len;
}

void knx_store_bytes8(uint8_t **b, uint8_t c)
{
	memcpy(*b, &c, 1);
	(*b) += 1;
}

void knx_store_bytes16(uint8_t **b, uint16_t c)
{
	c = htons(c);
	memcpy(*b, &c, 2);
	(*b) += 2;
}

void knx_store_bytes32(uint8_t **b, uint32_t c)
{
	c = htonl(c);
	memcpy(*b, &c, 4);
	(*b) += 4;
}

uint8_t* knx_retrieve_raw_bytes(uint8_t **b, const size_t len)
{
	uint8_t *v = NULL;
	v = (uint8_t *)malloc(len);
	if (!v)
		goto out;

	memcpy(v, *b, len);
	return v;
out:
	return NULL;
}

uint8_t knx_retrieve_bytes8(uint8_t **b)
{
	uint8_t u;
	memcpy(&u, *b, 1);
	(*b) += 1;
	return u;
}

uint16_t knx_retrieve_bytes16(uint8_t **b)
{
	uint16_t u;
	memcpy(&u, *b, 2);
	(*b) += 2;
	return ntohs(u);
}

uint32_t knx_retrieve_bytes32(uint8_t **b)
{
	uint32_t u;
	memcpy(&u, *b, 4);
	(*b) += 4;
	return ntohl(u);
}

uint16_t knx_retrieve_header(uint8_t **b)
{
	return knx_retrieve_bytes16(b);
}

ssize_t smc_knx_send_protocol(struct protocol_data *p)
{
	return sendto(p->fd, p->buffer, p->buffer_len, \
		p->flags, p->src_addr, p->src_addr_len);
}

ssize_t smc_knx_send_discovery_resp(struct protocol_data *p)
{
	// send discovery response
	uint8_t buffer[BUFFER_SIZE64] = {0};
	uint8_t *b = buffer;
	
	//header 2 bytes
	knx_store_bytes16(&b, KNX_PROTO_DISCOVERY_RESPONSE);
	p->buffer = buffer;
	
	//send response
	return knx_send_protocol(p);
}

void smc_knx_handle_protocol(struct protocol_data *p)
{
	uint16_t header_type;
	uint8_t *b = (uint8_t *)p->buffer;
	header_type = knx_retrieve_header(&b);
	
	switch(header_type)
	{
		case KNX_PROTO_DISCOVERY_REQUEST:
			knx_handle_discovery(p);
		case KNX_PROTO_STANDARD_PACKET:
			knx_handle_standard_packet(p);
		default:
			fprintf(stderr, "%s %d, unknown packet type\n", __func__, __LINE__);
			break;
	}
}

void smc_knx_handle_discovery(struct protocol_data *p)
{
	fprintf(stdout, "received a discovery packet\n");
	int ret;
	
	ret = knx_send_discovery_resp(p);
	fprintf(stdout, "send a discovery response len=%d\n", ret);
		
	//set the state of knx client as alive
	set_real_time_info(p, INFO_NR_KNX, CLIENT_STATE_ALIVE);
}

void smc_knx_handle_standard_packet(struct protocol_data *p)
{
	/* just dump it now,
	 * this packet will add to a queue later, 
	 * the queue is used for smc & app communication
	 * */
	 fprintf(stdout, "received a standard knx packet\n");
	 dump_buffer((unsigned char *)p->buffer, p->buffer_len);
}

// assemble a discovery request packet
void smc_knx_protocol_assemble_discovery_req(struct pkt_t *p)
{
	if (!p) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		return;
	}
	
	uint8_t *b = p->u;
	knx_store_bytes16(&b, KNX_PROTO_DISCOVERY_REQUEST);
	p->type = PACKET_TYPE_SOCKET|PACKET_RESPONSE_NEEDED;
	p->length = sizeof(uint16_t);
}

void cb_func_discovery_req_timeout(void *d)
{
	if (!d) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		return;
	}
	
	//add the packet to rxq
	struct circular_queue *rxq = NULL;
	rxq = knx_protocol_get_queue_rx();
	if (!rxq) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		return;
	}
	//re-add the timer to timer_heap
	struct knx_timer *t = knx_timer_alloc(sizeof(struct knx_timer));
	if (!t) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		return;
	}
	
	struct pkt_t *pkt = (struct pkt_t *)d;
	struct pkt_t *udata = knx_protocol_alloc_pkt(sizeof(uint16_t));
	memcpy(udata, pkt, sizeof(pkt));
	memcpy(udata->u, pkt->u, pkt->length);
	
	t->valid = true;
	t->expire = 3; //3 second for timeout
	t->user_data = udata;
	t->cb_func = timeout_cb_func_discovery_req;
	fprintf(stdout, "re-add %04x to timer_heap\n", *((uint16_t *)pkt->u));
	timer_heap_add(t);
}

