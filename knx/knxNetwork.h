#ifndef __KNX_NETWORK_H__
#define __KNX_NETWORK_H__

#include "knxCommon.h"
#include "knxProtocol.h"

/* Macros */
#define THREAD_RETURN_TYPE void*


void knx_store_raw_bytes(uint8_t **buffer, const uint8_t *r, const size_t len);
void knx_store_bytes8(uint8_t **buffer, uint8_t c);
void knx_store_bytes16(uint8_t **buffer, uint16_t c);
void knx_store_bytes32(uint8_t **buffer, uint32_t c);

uint8_t* knx_retrieve_raw_bytes(uint8_t **buffer, const size_t len);
uint8_t knx_retrieve_bytes8(uint8_t **buffer);
uint16_t knx_retrieve_bytes16(uint8_t **buffer);
uint32_t knx_retrieve_bytes32(uint8_t **buffer);

uint16_t knx_retrieve_header(uint8_t **b);

ssize_t knx_send_protocol(struct protocol_data *p);

ssize_t knx_send_discovery_resp(struct protocol_data *p);


void knx_handle_protocol(struct protocol_data *p);
void knx_handle_discovery(struct protocol_data *p);
void knx_handle_standard_packet(struct protocol_data *p);

#endif /* __KNX_NETWORK_H__ */
