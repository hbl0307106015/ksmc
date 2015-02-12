#include "log.h"
#include "knxNetwork.h"
#include "knxProtocol.h"

void knx_store_raw_bytes(uint8_t **b, const uint8_t *r, const size_t len)
{
	memcpy((*b), r, len);
	(*b) += len;
}

void knx_store_bytes8(uint8_t **b, uint8_t c)
{
	memcpy(*b, &c, 1);
	(*b) += 1;
}

void knx_store_bytes16(uint8_t **b, uint16_t c)
{
	c = htons(c);
	memcpy(*b, &c, 2);
	(*b) += 2;
}

void knx_store_bytes32(uint8_t **b, uint32_t c)
{
	c = htonl(c);
	memcpy(*b, &c, 4);
	(*b) += 4;
}

uint8_t* knx_retrieve_raw_bytes(uint8_t **b, const size_t len)
{
	uint8_t *v = NULL;
	v = (uint8_t *)malloc(len);
	if (!v)
		goto out;

	memcpy(v, *b, len);
	return v;
out:
	return NULL;
}

uint8_t knx_retrieve_bytes8(uint8_t **b)
{
	uint8_t u;
	memcpy(&u, *b, 1);
	(*b) += 1;
	return u;
}

uint16_t knx_retrieve_bytes16(uint8_t **b)
{
	uint16_t u;
	memcpy(&u, *b, 2);
	(*b) += 2;
	return ntohs(u);
}

uint32_t knx_retrieve_bytes32(uint8_t **b)
{
	uint32_t u;
	memcpy(&u, *b, 4);
	(*b) += 4;
	return ntohl(u);
}


