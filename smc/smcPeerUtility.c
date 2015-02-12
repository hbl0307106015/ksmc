#include "smcPeerUtility.h"

int set_peer_info(const struct network_ctx_t *ctx, const uint8_t n, uint8_t s);
int set_peer_ctx(const struct network_ctx_t *ctx, const uint8_t n);
int set_peer_state(const uint8_t n, uint8_t s);


static struct peer_real_time_info gClientList[INFO_NR_MAX] = {
		[INFO_NR_APP] = {
				.state = PEER_STATE_UNALIVE,
			},
		[INFO_NR_KNX] = {
				.state = PEER_STATE_UNALIVE,
			},
		[INFO_NR_ZIGBEE] = {
				.state = PEER_STATE_UNALIVE,
			},
};

struct peer_real_time_info *get_peer_info(const uint8_t n)
{
	if (n >= INFO_NR_MAX)
		return NULL;

	return (&(gClientList[n]));
}

struct network_ctx_t *get_peer_ctx(const uint8_t n)
{
	if (n >= INFO_NR_MAX)
		return NULL;

	return (&(gClientList[n].network_ctx));
}

uint8_t get_peer_state(const uint8_t n)
{
	if (n >= INFO_NR_MAX)
		return INFO_NR_MAX;

	return (gClientList[n].state);
}

int set_peer_info(const struct network_ctx_t *ctx, const uint8_t n, uint8_t s)
{
	if (n > INFO_NR_MAX || s > PEER_STATE_MAX || !ctx) {
		fprintf(stdout, "error!!! peer parameters overflow\n");
		goto out;
	}
	
	gClientList[n].state = s;
	gClientList[n].network_ctx.fd = ctx->fd;
	gClientList[n].network_ctx.flags = ctx->flags;
	gClientList[n].network_ctx.src_addr = ctx->src_addr;
	gClientList[n].network_ctx.src_addr_len = ctx->src_addr_len;
	
	return 0;
	
out:
	return -1;
}

int set_peer_ctx(const struct network_ctx_t *ctx, const uint8_t n)
{
	if (n > INFO_NR_MAX || !ctx)
		goto out;

	gClientList[n].network_ctx.fd = ctx->fd;
	gClientList[n].network_ctx.flags = ctx->flags;
	gClientList[n].network_ctx.src_addr = ctx->src_addr;
	gClientList[n].network_ctx.src_addr_len = ctx->src_addr_len;
	
	return 0;
	
out:
	return -1;	
}

int set_peer_state(const uint8_t n, uint8_t s)
{
	if (n > INFO_NR_MAX || s > PEER_STATE_MAX)
		goto out;
	
	gClientList[n].state = s;

	return 0;
	
out:
	return -1;
}
