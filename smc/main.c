#include "smcCommon.h"
#include "smcProtocol.h"
#include "threadKNX.h"
#include "threadAPP.h"
#include "pratical.h"
#include "log.h"

#define VERSION_STR "0.0.1"

volatile bool gDoExit = false;

static void show_version(void)
{
	fprintf(stderr,
		"smc v" VERSION_STR "\n"
		"User space daemon for smart controller,\n"
		"KNX and ZigBee relevant device Controller and Manager\n"
		"Copyright (c) 2014, Networking "
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
	int c = 0, ret = 0;
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
	
	#if 0
	pthread_t tKNX;
	struct thread_knx_arg knx_arg;
	knx_arg.sock = sock;
	knx_arg.real_time_info = get_real_time_info(INFO_NR_KNX);
	ret = pthread_create(&tKNX, NULL, smc_thread_knx, (void *)&knx_arg);
	if (ret != 0) {
		fprintf(stderr, "thread knx failed...\n");
		goto out;
	}
	#endif
	
	pthread_t tAPP;
	struct thread_app_arg app_arg;
	app_arg.sock = sock;
	ret = pthread_create(&tAPP, NULL, smc_thread_app, (void *)&app_arg);
	if (ret != 0) {
		fprintf(stderr, "thread knx failed...\n");
		goto out;
	}
	//app_arg.real_time_info = get_real_time_info(INFO_NR_APP);
	//fprintf(stdout, "thread knx:%u\n", ((unsigned int)tKNX));
	fprintf(stdout, "thread app:%u\n", ((unsigned int)tAPP));
	
	//pthread_join(tKNX, NULL);
	pthread_join(tAPP, NULL);
	
	//NOT REACHED except for (Ctrl-C)
out:
	fputs("shuting down smc...\n", stdout);
	close(sock);
    return 0;
}
