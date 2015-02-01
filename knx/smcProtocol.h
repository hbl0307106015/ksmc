#ifndef __SMCPROTOCOL_H__
#define __SMCPROTOCOL_H__

struct protocol_data {
	int fd; // socket file descriptor
	int flags; // flags, mostly is 0
	struct sockaddr *src_addr; //socket address structure
	socklen_t src_addr_len; // socket address length
	void *buffer; // I/O buffer, unsigned char* or uint8*
	size_t buffer_len; // length of the I/O buffer
};

#define KNX_PROTO_DISCOVERY_REQUEST 0x1000
#define KNX_PROTO_DISCOVERY_RESPONSE 0x1001
#define KNX_PROTO_STANDARD_PACKET 0x1002

#endif /* __SMCPROTOCOL_H__ */
