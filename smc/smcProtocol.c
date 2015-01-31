#include "smcProtocol.h"
#include "smcCommon.h"

struct client_real_time_info gClientInfo[MAX_CLIENT_CLASS] = {
		[INFO_NR_APP] = {
				.state = CLIENT_STATE_UNALIVE,
			},
		[INFO_NR_KNX] = {
				.state = CLIENT_STATE_UNALIVE,
			},
		[INFO_NR_ZIGBEE] = {
				.state = CLIENT_STATE_UNALIVE,
			},
};

struct client_real_time_info *get_real_time_info(uint8_t nr)
{
	if (nr >= MAX_CLIENT_CLASS)
		return NULL;

	return (&(gClientInfo[nr]));
}

int set_real_time_info(struct protocol_data *p, uint8_t nr, uint8_t s)
{
	if (nr > INFO_NR_MAX || s > CLIENT_STATE_MAX)
		goto out;
	
	gClientInfo[nr].state = s;
	gClientInfo[nr].client_info.fd = p->fd;
	gClientInfo[nr].client_info.flags = p->flags;
	gClientInfo[nr].client_info.src_addr = p->src_addr;
	gClientInfo[nr].client_info.src_addr_len = p->src_addr_len;
	
	return 0;
	
out:
	return -1;
}

int set_real_time_info_state(uint8_t nr, uint8_t s)
{
	if (nr > INFO_NR_MAX || s > CLIENT_STATE_MAX)
		goto out;
	
	gClientInfo[nr].state = s;

	return 0;
	
out:
	return -1;
}
