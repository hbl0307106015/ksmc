#include "log.h"
#include "smcNetwork.h"
#include "smcQueue.h"

void smc_store_raw_bytes(uint8_t **b, const uint8_t *r, const size_t len)
{
	memcpy((*b), r, len);
	(*b) += len;
}

void smc_store_bytes8(uint8_t **b, uint8_t c)
{
	memcpy(*b, &c, 1);
	(*b) += 1;
}

void smc_store_bytes16(uint8_t **b, uint16_t c)
{
	c = htons(c);
	memcpy(*b, &c, 2);
	(*b) += 2;
}

void smc_store_bytes32(uint8_t **b, uint32_t c)
{
	c = htonl(c);
	memcpy(*b, &c, 4);
	(*b) += 4;
}

uint8_t* smc_retrieve_raw_bytes(uint8_t **b, const size_t len)
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

uint8_t smc_retrieve_bytes8(uint8_t **b)
{
	uint8_t u;
	memcpy(&u, *b, 1);
	(*b) += 1;
	return u;
}

uint16_t smc_retrieve_bytes16(uint8_t **b)
{
	uint16_t u;
	memcpy(&u, *b, 2);
	(*b) += 2;
	return ntohs(u);
}

uint32_t smc_retrieve_bytes32(uint8_t **b)
{
	uint32_t u;
	memcpy(&u, *b, 4);
	(*b) += 4;
	return ntohl(u);
}

void PrintSocketAddress(const struct sockaddr *addr, FILE *s)
{
	//test for address and stream
	if (addr == NULL || s == NULL)
		goto out;
	
	void *numericAddress = NULL;
	char addrBuffer[INET6_ADDRSTRLEN] = {0};// buffer to contain result (IPv6 sufficient to hold IPv4)
	in_port_t port; // port to print
	
	//set pointer to address based on address family
	switch (addr->sa_family)
	{
		case AF_INET:
			numericAddress = &(((struct sockaddr_in *)addr)->sin_addr);
			port = ntohs(((struct sockaddr_in *)addr)->sin_port);
			break;
		case AF_INET6:
			numericAddress = &(((struct sockaddr_in6 *)addr)->sin6_addr);
			port = ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
			break;
		default:
			fputs("unknown sockaddr * address type\n", s);
			goto out;
	} //switch
	
	//convert binary to printable address
	if (inet_ntop(addr->sa_family, numericAddress, addrBuffer,\
		sizeof(addrBuffer)) == NULL)
		fputs("invalid address\n", s);
	else {
		fprintf(s, "%s", addrBuffer);
		if (port != 0) //zero not valid in any socket addr
			fprintf(s, "-%u", port);
		else
			fputs("-port number not valid", s);
	}

out:
	return;
}

void smc_iface_init()
{
    int sock4if = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock4if < 0) {
		DieWithSystemMessage("thread app, socket 4 if() failed");
		goto out;
	}
	
	struct ifreq ifr;
	size_t slen = strlen(gIface);
	strncpy(ifr.ifr_name, gIface, slen);
	ifr.ifr_name[slen] = '\0';
	fprintf(stdout, "interface str=%s strlen=%d\n", ifr.ifr_name, slen);
	if (ioctl(sock4if, SIOCGIFADDR, &ifr) < 0) {
		perror("ioctl, get if addr error");
        close(sock4if);
		goto out;
	}
	
	struct sockaddr_in gen_addr;
	memcpy(&gen_addr, &(ifr.ifr_addr), sizeof(gen_addr));
	gIPstr = inet_ntoa(gen_addr.sin_addr);
	slen = strlen(gIPstr);
	fprintf(stdout, "%s-%d\n",  gIPstr, slen);

out:
    close(sock4if);
    return;
}
