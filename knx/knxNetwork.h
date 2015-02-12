#ifndef __KNX_NETWORK_H__
#define __KNX_NETWORK_H__

#include "knxCommon.h"
#include "knxProtocol.h"

void knx_store_raw_bytes(uint8_t **buffer, const uint8_t *r, const size_t len);
void knx_store_bytes8(uint8_t **buffer, uint8_t c);
void knx_store_bytes16(uint8_t **buffer, uint16_t c);
void knx_store_bytes32(uint8_t **buffer, uint32_t c);

uint8_t* knx_retrieve_raw_bytes(uint8_t **buffer, const size_t len);
uint8_t knx_retrieve_bytes8(uint8_t **buffer);
uint16_t knx_retrieve_bytes16(uint8_t **buffer);
uint32_t knx_retrieve_bytes32(uint8_t **buffer);



#endif /* __KNX_NETWORK_H__ */
