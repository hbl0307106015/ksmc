#ifndef __SMCPROTOCOL_H__
#define __SMCPROTOCOL_H__

#define SMC_PROTO_KNX_SAMPLE		0x0A
#define SMC_PROTO_KNX_DO_A_TEST		0x0B
#define SMC_PROTO_KNX_DISCOV		0x0C

struct smc_proto_header {
	uint8_t type; /* packet type */
	uint8_t *u; /* content */
};

#define KNX_PROTO_DISCOVERY_REQUEST 0x1000
#define KNX_PROTO_DISCOVERY_RESPONSE 0x1001
#define KNX_PROTO_STANDARD_PACKET 0x1002

#endif /* __SMCPROTOCOL_H__ */
