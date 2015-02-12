#ifndef __SMCPEERUTILITY_H__
#define __SMCPEERUTILITY_H__

#include "smcCommon.h"
#include "smcNetwork.h"

enum {
	PEER_STATE_UNALIVE,
	PEER_STATE_ALIVE,
	PEER_STATE_MAX
} peer_state;

enum {
	INFO_NR_APP,
	INFO_NR_KNX,
	INFO_NR_ZIGBEE,
	INFO_NR_MAX
} info_number_enum;

struct peer_real_time_info {
	struct network_ctx_t network_ctx; //client information, including socket fd, addr, buffer, len, etc
	uint8_t state; //real time state
};

struct peer_real_time_info *get_peer_info(const uint8_t n);
struct network_ctx_t* get_peer_ctx(const uint8_t n);
uint8_t get_peer_state(const uint8_t n);

int set_peer_info(const struct network_ctx_t *ctx, const uint8_t n, uint8_t s);
int set_peer_ctx(const struct network_ctx_t *ctx, const uint8_t n);
int set_peer_state(const uint8_t n, uint8_t s);

#endif /* __SMCPEERUTILITY_H__ */
