#ifndef __SMC_NETWORK_H__
#define __SMC_NETWORK_H__

#include "smcCommon.h"

/* Macros */
#define PORT_NUM_APP "4097"
#define PORT_NUM_KNX "4096"
#define PORT_NUM_ZIGBEE "4095"

#define MAX_NUM_POLL_FILES_DES 2
#define POLL_TIME_OUT_MS 500
#define MAX_UDP_PAYLOAD_SIZE 65507

#define THREAD_RETURN_TYPE void*

struct network_ctx_t {
	int fd; // socket file descriptor
	int flags; // flags, mostly is 0
	struct sockaddr *src_addr; //socket address structure
	socklen_t src_addr_len; // socket address length
};

void smc_store_raw_bytes(uint8_t **buffer, const uint8_t *r, const size_t len);
void smc_store_bytes8(uint8_t **buffer, uint8_t c);
void smc_store_bytes16(uint8_t **buffer, uint16_t c);
void smc_store_bytes32(uint8_t **buffer, uint32_t c);

uint8_t* smc_retrieve_raw_bytes(uint8_t **buffer, const size_t len);
uint8_t smc_retrieve_bytes8(uint8_t **buffer);
uint16_t smc_retrieve_bytes16(uint8_t **buffer);
uint32_t smc_retrieve_bytes32(uint8_t **buffer);

void PrintSocketAddress(const struct sockaddr *addr, FILE *s);
void smc_iface_init();

#endif /* __SMC_NETWORK_H__ */
