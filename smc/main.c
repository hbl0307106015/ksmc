#include "smcCommon.h"
#include "smcProtocol.h"
#include "smcPeerUtility.h"
#include "smcQueue.h"
#include "threadKNX.h"
#include "threadAPP.h"
#include "threadZigBee.h"
#include "log.h"

#define VERSION_STR "0.0.1"

char *gIface = NULL;
char *gIPstr = NULL;
volatile bool gDoExit = false;

static void show_version(void);
static void usage(void);

static void sighandler(int signum);
static void sigaction_init();

static void queue_init(void);

int main(int argc, char *argv[])
{
	int c = 0, ret = 0;
	bool daemonize = false;
	int nochdir = 1, noclose = 1;

	for (;;) {
		c = getopt(argc, argv, "hvBPi:");
		if (c < 0)
			break;
		switch (c) {
		case 'h':
			usage();
			break;
		case 'i':
			gIface = optarg;
			break;
		case 'B':
			daemonize = true;
			nochdir = 0;
			break;
		case 'P':
			noclose = 0;
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
	
	if (!gIface) {
		usage();
		goto out;
	}
	
	fprintf(stdout, "show option parameters' value\n"
		"daemonize: %d\n"
		"iface: %s\n",
		daemonize,
		gIface);

	if (daemonize && daemon(nochdir, noclose))
			fprintf(stdout, "program run in background\n");
	
	sigaction_init();

	queue_init();
	
	smc_iface_init();
	
	#if 1
	pthread_t tKNX;
	struct thread_knx_arg knx_arg;
	//knx_arg.sock = sock;
	//knx_arg.knx_client_info = get_peer_info(INFO_NR_KNX);
	ret = pthread_create(&tKNX, NULL, smc_thread_knx, (void *)&knx_arg);
	if (ret != 0) {
		fprintf(stderr, "thread knx failed...\n");
		goto out;
	}
	fprintf(stdout, "thread knx:%u\n", ((unsigned int)tKNX));	
	#endif
	
	#if 1
	pthread_t tAPP;
	struct thread_app_arg app_arg;
	//app_arg.sock = sock;
	ret = pthread_create(&tAPP, NULL, smc_thread_app, (void *)&app_arg);
	if (ret != 0) {
		fprintf(stderr, "thread APP failed...\n");
		goto out;
	}
	fprintf(stdout, "thread app:%u\n", ((unsigned int)tAPP));
	#endif
	
	#if 1
	pthread_t tZigBee;
	struct thread_zigbee_arg zigbee_arg;
	zigbee_arg.str_service = PORT_NUM_ZIGBEE;
	ret = pthread_create(&tZigBee, NULL, smc_thread_zigbee, (void *)&zigbee_arg);
	if (ret != 0) {
		fprintf(stderr, "thread ZigBee failed...\n");
		goto out;
	}
	fprintf(stdout, "thread zigBee:%u\n", ((unsigned int)tZigBee));
	#endif

	pthread_join(tKNX, NULL);
	pthread_join(tAPP, NULL);
	pthread_join(tZigBee, NULL);

	//NOT REACHED except for (Ctrl-C)
out:
	fputs("shuting down smc...\n", stdout);
    return 0;
}

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
		"usage: smc [-Bhv] -i <network interface name> "
		"\\\n"
		"\n"
		"options:\n"
		"   -h   show this usage\n"
		"   -i   specify the interface\n"
		"   -B   run daemon in the background\n"
		"   -P   run daemon without debug message\n"
		"   -v   show hostapd version\n");

	exit(1);
}

static void sighandler(int signum)
{
	fprintf(stdout, "got signum=%d\n", signum);
	gDoExit = true;
}

static void sigaction_init()
{
	struct sigaction sigact;
	sigact.sa_handler = sighandler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGQUIT, &sigact, NULL);
}

static void queue_init(void)
{
	int ret = 0;
	
	ret = smc_init_queue_by_index(QUEUE_INDEX_RX_GENERIC);
	if (ret < 0) {
		fprintf(stderr, "%s %d init queue generic failed\n", __func__, __LINE__);
		goto out;
	} else
		fprintf(stdout, "queue generic init successfully\n");

	ret = smc_init_queue_by_index(QUEUE_INDEX_TX_KNX);
	if (ret < 0) {
		fprintf(stderr, "%s %d txq knx failed\n", __func__, __LINE__);
		goto out;
	} else
		fprintf(stdout, "txq knx init successfully\n");

	ret = smc_init_queue_by_index(QUEUE_INDEX_TX_ZIGBEE);
	if (ret < 0) {
		fprintf(stderr, "%s %d init txq zigbee failed\n", __func__, __LINE__);
		goto out;
	} else
		fprintf(stdout, "txq zigbee init successfully\n");
	
	return;
out:
	exit(0);
}
