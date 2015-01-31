#ifndef __SMCPROTOCOL_H__
#define __SMCPROTOCOL_H__

#include "smcCommon.h"

struct protocol_data {
	int fd; // socket file descriptor
	int flags; // flags, mostly is 0
	struct sockaddr *src_addr; //socket address structure
	socklen_t src_addr_len; // socket address length
	void *buffer; // I/O buffer, unsigned char* or uint8*
	size_t buffer_len; // length of the I/O buffer
};

enum {
	CLIENT_STATE_UNALIVE,
	CLIENT_STATE_ALIVE,
	CLIENT_STATE_MAX
};

enum {
	INFO_NR_APP,
	INFO_NR_KNX,
	INFO_NR_ZIGBEE,
	INFO_NR_MAX
};

struct client_real_time_info {
	struct protocol_data client_info; //client information, including socket fd, addr, buffer, len, etc
	uint8_t state; //real time state
};

#define MAX_CLIENT_CLASS 3
extern struct client_real_time_info gClientInfo[MAX_CLIENT_CLASS];

struct client_real_time_info *get_real_time_info(uint8_t nr);
int set_real_time_info(struct protocol_data *p, uint8_t nr, uint8_t s);
int set_real_time_info_state(uint8_t nr, uint8_t s);

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


#define KNX_PROTO_DISCOVERY_REQUEST 0x1000
#define KNX_PROTO_DISCOVERY_RESPONSE 0x1001
#define KNX_PROTO_STANDARD_PACKET 0x1002

/* MACROs for ZigBee */
#define ZIGBEE_PROTO_DISCOVERY_REQUEST 0x6000
#define ZIGBEE_PROTO_SAMPLE_PACKET 0x6002


/* MACROs for APP */
#define APP_PROTO_DISCOVERY_REQUEST 0xB000
#define APP_PROTO_SAMPLE_PACKET 0xB002

#endif /* __SMCPROTOCOL_H__ */
