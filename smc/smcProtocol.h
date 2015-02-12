#ifndef __SMCPROTOCOL_H__
#define __SMCPROTOCOL_H__

#include "smcCommon.h"
#include "smcNetwork.h"

struct protocol_data {
	struct network_ctx_t network_ctx;
	void *buffer; // I/O buffer, unsigned char* or uint8*
	size_t buffer_len; // length of the I/O buffer
};

// generic packet type
struct pkt_t {
	uint8_t *u; // content of the buffer
	size_t length; // length of the buffer
};

// allocate a packet
struct pkt_t* smc_pkt_alloc(size_t len);

// fill out the packet type structure
int smc_pkt_fill(struct pkt_t *p, unsigned char *b, size_t len);

/* MACROs for KNX */
#define RC1180_MAX_BUFFER_SIZE BUFFER_SIZE64

/* KNX1 Timing Table 
 * Note:(KNX2 has all of the features of KNX1 inaddition to 
 * KNX RF Multi functions, and multi repeater)
 * The measure unit is micro-second (us)
 * */
#define TIME_RX_TXD 180
#define TIME_TXD	590
#define TIME_TXD_IDLE	900

#define TIME_PACKET_TIMEOUT_2s 2000000
#define TIME_RXD_CTS 20
#define TIME_RXD_TX 960
#define TIME_TX 3600
#define TIME_TX_IDLE 960


#define KNX_PROTO_DISCOVERY_REQUEST 0x0001
#define KNX_PROTO_DISCOVERY_RESPONSE 0x0002
#define KNX_PROTO_STANDARD_PACKET 0x1000

/* MACROs for ZigBee */
#define ZIGBEE_PROTO_DISCOVERY_REQUEST 0x6000
#define ZIGBEE_PROTO_SAMPLE_PACKET 0x6002
#define ZIGBEE_PROTO_LED_STATE_PACKET 0x2000
#define ZIGBEE_PROTO_LED_CTL_PACKET 0x2001


/* MACROs for APP */
#define APP_PROTO_DISCOVERY_REQUEST 0x3000
#define APP_PROTO_DISCOVERY_RESPONSE 0x3001
#define APP_PROTO_SAMPLE_PACKET 0xB002


// functions

ssize_t smc_send_protocol(struct protocol_data *p);

// functions for knx
ssize_t smc_knx_send_discovery_resp(struct protocol_data *p);
void smc_knx_handle_protocol(struct protocol_data *p);
void smc_knx_handle_discovery(struct protocol_data *p);
void smc_knx_handle_standard_packet(struct protocol_data *p);

// functions for app
void smc_app_handle_protocol(struct protocol_data *p);
void smc_app_handle_led_ctl_packet(struct protocol_data *p);
ssize_t smc_app_transmit_protocol(struct protocol_data *p);

// functions for zigbee
void smc_zigbee_handle_protocol(struct protocol_data *p);
void smc_zigbee_handle_led_state(struct protocol_data *p);
ssize_t smc_zigbee_transmit_protocol(struct protocol_data *p);
ssize_t smc_zigbee_send_discovery_resp(struct protocol_data *p);

uint16_t smc_retrieve_header(uint8_t **b);
void smc_store_header(uint8_t **b, uint16_t t);


#endif /* __SMCPROTOCOL_H__ */
