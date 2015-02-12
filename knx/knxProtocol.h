#ifndef __KNX_PROTOCOL_H__
#define __KNX_PROTOCOL_H__

#include "knxCommon.h"
#include "knxQueue.h"

// global variables
extern uint16_t gTxInterval;

// Macro
#define VERSION_STR "0.3.0"

#define GET_OFFSET_RECV_C(b) (b + 0)
#define GET_OFFSET_RECV_HEADER(b) (b + 1)
#define GET_OFFSET_RECV_RF_INFO(b) (b + 2)
#define GET_OFFSET_RECV_KNX_CTRL(b) (b + 9)
// get the offset of TX serial number of Domain Address
#define GET_OFFSET_RECV_TXSN_OR_DA(b) (b + 3)
#define GET_OFFSET_RECV_APPL_HEADER(b) (b + 10)
#define GET_OFFSET_RECV_SRC(b) GET_OFFSET_RECV_APPL_HEADER(b)
#define GET_OFFSET_RECV_APPL_DATA (b + 17)

#define LENGTH_BEFORE_APPL_DATA 17
#define LENGTH_RSSI_INFO 1

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

// knx packet type
struct pkt_t {
	uint8_t *u; // content of the buffer
	size_t length; // length of the buffer
};

// network role enumeration
enum {
    ROLE_TRANSMITTER = 0,
    ROLE_RECEIVER,
    ROLE_TRANSMITTER_RECEIVER,
} network_role;

// functions
// allocate a memory for generic packet type structure
struct pkt_t* knx_protocol_alloc_pkt(size_t len);

// fill out the packet type structure
int knx_protocol_pkt_fill(struct pkt_t *p, unsigned char *b, size_t len);

// transfer data type for baud rate
uint16_t transfer_wait_time(speed_t spd, int char_len);

// return actual user data length
uint16_t get_user_data_length(uint16_t total_len);

// store a packet to the queue
int knx_protocol_store_packet(struct circular_queue **que, void *d);

// retrieve a packet from the queue
void* knx_protocol_retrieve_packet(struct circular_queue **que);

uint16_t knx_retrieve_header(uint8_t **b);
void knx_store_header(uint8_t **b, uint16_t t);

#endif /* __KNX_PROTOCOL_H__ */
