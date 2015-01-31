#ifndef __SMC_NETWORK_H__
#define __SMC_NETWORK_H__

#include "smcCommon.h"
#include "smcProtocol.h"

/* Macros */
#define PORT_NUM_APP 4097
#define PORT_NUM_KNX 4096
#define PORT_NUM_ZIGBEE 4095

#define THREAD_RETURN_TYPE void*


void smc_store_raw_bytes(uint8_t **buffer, const uint8_t *r, const size_t len);
void smc_store_bytes8(uint8_t **buffer, uint8_t c);
void smc_store_bytes16(uint8_t **buffer, uint16_t c);
void smc_store_bytes32(uint8_t **buffer, uint32_t c);

uint8_t* smc_retrieve_raw_bytes(uint8_t **buffer, const size_t len);
uint8_t smc_retrieve_bytes8(uint8_t **buffer);
uint16_t smc_retrieve_bytes16(uint8_t **buffer);
uint32_t smc_retrieve_bytes32(uint8_t **buffer);

uint16_t smc_retrieve_header(uint8_t **b);

ssize_t smc_send_protocol(struct protocol_data *p);

ssize_t smc_knx_send_discovery_resp(struct protocol_data *p);


void smc_knx_handle_protocol(struct protocol_data *p);
void smc_knx_handle_discovery(struct protocol_data *p);
void smc_knx_handle_standard_packet(struct protocol_data *p);

#endif /* __SMC_NETWORK_H__ */
