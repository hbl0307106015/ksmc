#include "pratical.h"

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
