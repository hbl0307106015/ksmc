#include "utility.h"
#include "pratical.h"

int SetupTCPClientSocket(const char *host, const char *service)
{
	// Tell the system what kind of address info we want
	struct addrinfo addrCriteria; // Criteria for address match
	memset(&addrCriteria, 0, sizeof(addrCriteria));  // Zero out structure
	addrCriteria.ai_family = AF_UNSPEC; // v4 or v6
	addrCriteria.ai_socktype = SOCK_STREAM; // Only Stream sockets
	addrCriteria.ai_protocol = IPPROTO_TCP; // Only TCP Protocol
	
	// Get address(es)
	struct addrinfo *servAddr = NULL; // Holder for returned list of server addrs
	int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);
	if (rtnVal != 0)
		DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));
		
	int sock = -1;
	struct addrinfo *addr  = NULL;
	for (addr = servAddr; addr != NULL; addr = addr->ai_next) {
		
		//create a reliable, stream socket using TCP
		sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		
		if (sock < 0)
			continue;
		
		// establish the connection to the echo server
		if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0)
			break;
		
		// socket connection failed; try next address
		close(sock);
		sock = -1;
	}
	
	freeaddrinfo(servAddr); //free addrinfo allocated in getaddrinfo()
	return sock;
}
