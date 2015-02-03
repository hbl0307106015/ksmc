#include "knxCommon.h"
#include "smcProtocol.h"

struct client_real_time_info *get_real_time_info(uint8_t nr)
{
	if (nr >= INFO_NR_MAX)
		return NULL;

	return (&(gServerInfo[nr]));
}

int set_real_time_info(struct protocol_data *p, uint8_t nr, uint8_t s)
{
	if (nr > INFO_NR_MAX || s > STATE_MAX)
		goto out;

	gServerInfo[nr].state = s;
	gServerInfo[nr].client_info.fd = p->fd;
	gServerInfo[nr].client_info.flags = p->flags;
	gServerInfo[nr].client_info.src_addr = p->src_addr;
	gServerInfo[nr].client_info.src_addr_len = p->src_addr_len;
	return 0;

out:
	return -1;
}

int set_real_time_info_state(uint8_t nr, uint8_t s)
{
	if (nr > INFO_NR_MAX || s > STATE_MAX)
		goto out;
	
	gServerInfo[nr].state = s;

	return 0;
	
out:
	return -1;
}
