#include"smcCommon.h"
#include"pratical.h"

#define VERSION_STR "0.0.1"
#define MAX_STRING_LENGTH 255
#define MAX_UDP_PAYLOAD_SIZE 65507

volatile bool gDoExit = false;

static void show_version(void)
{
	fprintf(stderr,
		"smc v" VERSION_STR "\n"
		"User space daemon for smart controller,\n"
		"KNX and ZigBee relevant device Controller and Manager\n"
		"Copyright (c) 2014, VIA Networking "
		"and contributors\n");
}

static void usage(void)
{
	show_version();
	fprintf(stderr,
		"\n"
		"usage: smc [-Bhv] [-p <server port/service>]"
		"\\\n"
		"\n"
		"options:\n"
		"   -h   show this usage\n"
		"   -p   server port/service\n"
		"   -B   run daemon in the background (comming soon)\n"
		"   -v   show hostapd version\n");

	exit(1);
}

static void sighandler(int signum)
{
	gDoExit = true;
}

int main(int argc, char *argv[])
{
	int c = 0;
	bool daemonize = false;
	char *str_service = NULL;

	for (;;) {
		c = getopt(argc, argv, "hp:vB");
		if (c < 0)
			break;
		switch (c) {
		case 'h':
			usage();
			break;
		case 'p':
			str_service = optarg;
			break;
		case 'B':
			daemonize = true;
			break;
		case 'v':
			show_version();
			exit(1);
			break;
		default:
			usage();
			break;
		}
	}
	
	fprintf(stdout, "show option parameters' value\n"
		"server port/service: %s\n"
		"daemonize: %d\n",
		str_service,
		daemonize);

	//construct the server address structure
	struct addrinfo addrCriteria; //criteria for address
	memset(&addrCriteria, 0, sizeof(addrCriteria));
	addrCriteria.ai_family = AF_UNSPEC; //any address family
	addrCriteria.ai_flags = AI_PASSIVE; //accept on any address/port
	addrCriteria.ai_socktype = SOCK_DGRAM; //only datagram socket
	addrCriteria.ai_protocol = IPPROTO_UDP; //only UDP socket
	
	struct addrinfo *servAddr = NULL; //list of server address(the result)
	int rtnVal = getaddrinfo(NULL, str_service, &addrCriteria, &servAddr);
	if (rtnVal != 0)
		DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));
	
	//create socket for incoming connections
	int sock = socket(servAddr->ai_family, servAddr->ai_socktype, \
		servAddr->ai_protocol);
	if (sock < 0)
		DieWithSystemMessage("socket() failed");
		
	//bind to the local address
	if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
		DieWithSystemMessage("bind() failed");
		
	//free address list which allocated by getaddrinfo()
	freeaddrinfo(servAddr);
	
	struct sigaction sigact;
	sigact.sa_handler = sighandler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGQUIT, &sigact, NULL);
	
	gDoExit = false;
	struct sockaddr_storage clntAddr; //client address
	socklen_t clntAddrLen = sizeof(clntAddr); // length of client address structure, (in-out parameter)
	unsigned char buffer[MAX_STRING_LENGTH] = {0}; // I/O buffer
	ssize_t numBytesRcvd = 0;
	ssize_t numBytesSent = 0;
	
	do {
		
		numBytesRcvd = recvfrom(sock, buffer, MAX_STRING_LENGTH, \
			0, (struct sockaddr *)&clntAddr, &clntAddrLen);
		if (numBytesRcvd < 0)
			DieWithSystemMessage("recvfrom() failed");
		
		fputs("Handling client ", stdout);
		PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
		fputc('\n', stdout);
		
		//send received datagram back to the client
		numBytesSent = sendto(sock, buffer, numBytesRcvd, \
			0, (struct sockaddr *)&clntAddr, sizeof(clntAddr));
		if (numBytesSent < 0)
			DieWithSystemMessage("sendto() failed");
		else if (numBytesSent != numBytesRcvd)
			DieWithUserMessage("sento()", "sent unexpected number of bytes");

	} while (gDoExit != true);
	
	// NOT REACHED except for (Ctrl-C)
	fputs("shuting down...\n", stdout);
	close(sock);
    return 0;
}
