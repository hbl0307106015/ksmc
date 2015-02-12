#include "log.h"
#include "smcQueue.h"
#include "smcProtocol.h"
#include "smcPeerUtility.h"
#include "smcPeerUtility.h"

// allocate a packet
struct pkt_t* smc_pkt_alloc(size_t len)
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

// fill out the packet type structure
int smc_pkt_fill(struct pkt_t *p, unsigned char *b, size_t len)
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
	p->length = len + sizeof(uint16_t);
	return 0;
out:
	return -1;
}

ssize_t smc_send_protocol(struct protocol_data *p)
{
	return sendto(p->network_ctx.fd, p->buffer, p->buffer_len, \
		p->network_ctx.flags, p->network_ctx.src_addr, p->network_ctx.src_addr_len);
}

ssize_t smc_knx_send_discovery_resp(struct protocol_data *p)
{
	// send discovery response
	uint8_t buffer[BUFFER_SIZE64] = {0};
	uint8_t *b = buffer;
	
	//header 2 bytes
	smc_store_bytes16(&b, KNX_PROTO_DISCOVERY_RESPONSE);
	p->buffer = buffer;
	
	//send response
	return smc_send_protocol(p);
}

void smc_knx_handle_protocol(struct protocol_data *p)
{
	if (!p) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}
	
	uint16_t header_type;
	uint8_t *b = (uint8_t *)p->buffer;
	header_type = smc_retrieve_header(&b);
	
	switch(header_type)
	{
		case KNX_PROTO_DISCOVERY_REQUEST:
			smc_knx_handle_discovery(p);
			break;
		case KNX_PROTO_STANDARD_PACKET:
			smc_knx_handle_standard_packet(p);
			break;
		default:
			fprintf(stderr, "%s %d, unknown packet type 0x%04x\n", __func__, __LINE__, header_type);
			break;
	}

out:
	return;
}

void smc_knx_handle_discovery(struct protocol_data *p)
{
	fprintf(stdout, "received a discovery packet\n");
	int ret;
	
	ret = smc_knx_send_discovery_resp(p);
	fprintf(stdout, "send a discovery response len=%d\n", ret);
		
	//set the state of knx client as alive
	set_peer_info(&(p->network_ctx), INFO_NR_KNX, PEER_STATE_ALIVE);
}

void smc_knx_handle_standard_packet(struct protocol_data *p)
{
	/* just dump it now,
	 * this packet will add to a queue later, 
	 * the queue is used for smc & app communication
	 * */
	fprintf(stdout, "received a standard knx packet\n");
	dump_buffer((unsigned char *)p->buffer, p->buffer_len);
	 
	/*get the txq que so that the thread_socket can
	* retrieve and send it to APP peer
	*/
	struct circular_queue *txq = NULL;
	txq = smc_get_queue(QUEUE_INDEX_RX_GENERIC);
	if (!txq) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}
	
	struct pkt_t *pkt = smc_pkt_alloc(p->buffer_len);
	smc_pkt_fill(pkt, p->buffer, p->buffer_len);
	fprintf(stdout, "enqueuing a packet, len=%d\n", (int)p->buffer_len);
	
	pthread_mutex_lock(&(txq->qmutex));
	smc_enqueue(txq, (void *)pkt);
	pthread_mutex_unlock(&(txq->qmutex));
	
out:
	return;
}

void smc_app_handle_protocol(struct protocol_data *p)
{
	if (!p) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}
	
	uint16_t header_type;
	uint8_t *b = (uint8_t *)p->buffer;
	header_type = smc_retrieve_header(&b);
	
	switch(header_type)
	{
		case APP_PROTO_DISCOVERY_REQUEST:
			fprintf(stdout, "%s %d, app discovery request packet type 0x%04x\n", __func__, __LINE__, header_type);
			set_peer_info(&(p->network_ctx), INFO_NR_APP, PEER_STATE_ALIVE);
			smc_zigbee_send_discovery_resp(p);
			break;
		case ZIGBEE_PROTO_LED_CTL_PACKET:
			fprintf(stdout, "handling app led ctl packet. type=%04x len=%d\n", header_type, p->buffer_len);
			dump_buffer(p->buffer, p->buffer_len);
			smc_app_handle_led_ctl_packet(p);
			break;
		default:
			fprintf(stderr, "%s %d, unknown packet type 0x%04x\n", __func__, __LINE__, header_type);
			break;
	}

out:
	return;
}

void smc_app_handle_led_ctl_packet(struct protocol_data *p)
{
	if (!p) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}

	// set app client state to alive
	set_peer_info(&(p->network_ctx), INFO_NR_APP, PEER_STATE_ALIVE);

	// add the packet to the queue for zigbee
	struct circular_queue *que = NULL;
	que = smc_get_queue(QUEUE_INDEX_TX_ZIGBEE);
	if (!que) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;		
	}
	
	struct pkt_t *pkt = smc_pkt_alloc(p->buffer_len);
	if (!pkt) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}
	
	smc_pkt_fill(pkt, p->buffer, p->buffer_len);
	fputs("enqueue a led ctl packet for zigbee\n", stdout);
	pthread_mutex_lock(&(que->qmutex));
	smc_enqueue(que, (void *)pkt);
	pthread_mutex_unlock(&(que->qmutex));

out:
	return;
}

ssize_t smc_app_transmit_protocol(struct protocol_data *p)
{
	return smc_send_protocol(p);
}

ssize_t smc_zigbee_send_discovery_resp(struct protocol_data *p)
{
	uint8_t buffer[BUFFER_SIZE64] = {0}, *q = buffer;
	uint16_t slen = strlen(gIPstr);
	
	fprintf(stdout, "%s str=%s string-len=%d\n",\
		__func__, gIPstr, slen);
		
	/* replace the header type and then send the packet back to app */
	smc_store_header(&q, APP_PROTO_DISCOVERY_RESPONSE); //protocol type
	smc_store_bytes16(&q, slen); //length of address string
	smc_store_raw_bytes(&q, gIPstr, slen); //length of address string
	p->buffer = buffer;
	p->buffer_len = (q - buffer);
	
	fprintf(stdout, "%s str=%s string-len=%d buffer-len=%d\n",\
		__func__, gIPstr, slen, p->buffer_len);
	return smc_app_transmit_protocol(p);
}

void smc_zigbee_handle_protocol(struct protocol_data *p)
{
	if (!p) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}
	
	uint16_t header_type = 0;
	uint8_t *b = p->buffer;
	header_type = smc_retrieve_header(&b);
	
	switch(header_type)
	{
		case ZIGBEE_PROTO_LED_STATE_PACKET:
			fprintf(stdout, "handling zigbee led state packet. type=%04x len=%d\n", header_type, p->buffer_len);
			dump_buffer(p->buffer, p->buffer_len);
			smc_zigbee_handle_led_state(p);
			break;
		default:
			fprintf(stderr, "%s %d, unknown packet type 0x%04x\n", __func__, __LINE__, header_type);
			break;
	}

out:
	return;
}

void smc_zigbee_handle_led_state(struct protocol_data *p)
{
	if (!p) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}
	
	// set the peer zigbee state to ALIVE
	set_peer_info(&(p->network_ctx), INFO_NR_ZIGBEE, PEER_STATE_ALIVE);
	
	/* save the packet to rx queue so that 
	* the thread_smc_app can send it to app
	*/
	struct circular_queue *rxq = NULL;
	rxq = smc_get_queue(QUEUE_INDEX_RX_GENERIC);
	if (!rxq) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}
	
	struct pkt_t *pkt = NULL;
	pkt = smc_pkt_alloc(p->buffer_len);
	if (!pkt) {
		fprintf(stderr, "%s %d, null pointer\n", __func__, __LINE__);
		goto out;
	}
	smc_pkt_fill(pkt, p->buffer, p->buffer_len);
	
	fputs("add led state packet to queue\n", stdout);
	pthread_mutex_lock(&(rxq->qmutex));
	smc_enqueue(rxq, (void *)pkt);
	pthread_mutex_unlock(&(rxq->qmutex));

out:
	return;
}

ssize_t smc_zigbee_transmit_protocol(struct protocol_data *p)
{
	return smc_send_protocol(p);
}

uint16_t smc_retrieve_header(uint8_t **b)
{
	return smc_retrieve_bytes16(b);
}

void smc_store_header(uint8_t **b, uint16_t t)
{
	return smc_store_bytes16(b, t);
}
