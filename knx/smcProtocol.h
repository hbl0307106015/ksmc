#ifndef __SMCPROTOCOL_H__
#define __SMCPROTOCOL_H__

#include "knxProtocol.h"

struct protocol_data {
	int fd; // socket file descriptor
	int flags; // flags, mostly is 0
	struct sockaddr *src_addr; //socket address structure
	socklen_t src_addr_len; // socket address length
	void *buffer; // I/O buffer, unsigned char* or uint8*
	size_t buffer_len; // length of the I/O buffer
};

struct client_real_time_info {
	struct protocol_data client_info; //client information, including socket fd, addr, buffer, len, etc
	uint8_t state; //real time state
};

#define MAX_CLIENT_CLASS 3
extern struct client_real_time_info gServerInfo[MAX_CLIENT_CLASS];

struct client_real_time_info *get_real_time_info(uint8_t nr);
int set_real_time_info(struct protocol_data *p, uint8_t nr, uint8_t s);
int set_real_time_info_state(uint8_t nr, uint8_t s);

#define KNX_PROTO_DISCOVERY_REQUEST 0x1000
#define KNX_PROTO_DISCOVERY_RESPONSE 0x1001
#define KNX_PROTO_STANDARD_PACKET 0x1002

#endif /* __SMCPROTOCOL_H__ */
