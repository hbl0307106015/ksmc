#include "knxQueue.h"
#include "knxCommon.h"
#include "knxProtocol.h"
#include "threadKnxTty.h"
#include "threadSocket.h"
#include "circularQueue.h"
#include "log.h"

// marcos
#define VERSION_STR "0.3.0"

// global variables
bool gEnableRssi = false;
bool gFlagExit = false;
uint8_t gNetworkRole = 0;
uint16_t gTxInterval = 0;

void* handle_knx_tty(void *arg);
void* handle_socket(void *arg);

// show the program's version
static void show_version(void);

// show the usage of this program
static void usage(void);

// program initiation function
static void program_init(char **dev,
                 speed_t *spd,
                 bool *rssi,
                 uint8_t *role,
                 char **p);

static void sighandler(int signum);

static void sigaction_init();

int main(int argc, char *argv[])
{
    int ret = 0;
	char *dev = NULL;
	char *port = NULL;
	speed_t spd_baud_rate;
	uint16_t int_baud_rate;
	
	bool daemonize = false;
	int nochdir = 1, noclose = 1;

    program_init(&dev,
                 &spd_baud_rate,
                 &gEnableRssi,
                 &gNetworkRole,
                 &port);

	int c = 0;
	for (;;) {
		c = getopt(argc, argv, "BPb:d:hp:r:sv");
		if (c < 0)
			break;
		switch (c) {
		case 'h':
			usage();
			break;
		case 'b':
			int_baud_rate = atoi(optarg);
			spd_baud_rate = transfer_baud_rate(int_baud_rate);
			break;
		case 'B':
			daemonize = true;
			nochdir = 0;
			break;
		case 'P':
			noclose = 0;
			break;
		case 'd':
			dev = optarg;
			break;
		case 'r':
			gNetworkRole = atoi(optarg);
			break;
		case 'p':
			port = optarg;
			break;
		case 's':
			gEnableRssi = true;
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
		"baud_rate: %u\n"
		"device node: %s\n"
		"enable rssi: %u\n"
		"network role: %u\n"
		"port number: %s\n"
		"daemon-nochdir : %d\n"
		"daemon-noclose : %d\n",
		spd_baud_rate,
		dev,
		gEnableRssi,
		gNetworkRole,
		port,
		nochdir,
		noclose);
	
	if (daemonize && daemon(nochdir, noclose))
			fprintf(stdout, "program run in background\n");
	
	int tty_fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY);
	if (tty_fd < 0) {
		perror("open file");
		goto out;
	}

	// fill out new termios struct and apply it
	set_tty_attr(tty_fd, spd_baud_rate);

	// show the current setting
	show_tty_attr(tty_fd);
	
	// restore the status of UART device, i.e. to the block-status(default)
	if (fcntl(tty_fd, F_SETFL, 0) < 0) {
		perror("fcntl set flag");
		goto out;
	}
	
	// to verify whether the fd is ready for a terminal
	if (isatty(tty_fd) == 0) {
		perror("this is not a terminal device");
		goto out;
	}

	sigaction_init();
	
	#if 1
	pthread_t tkt;
	struct thread_knx_arg tkp;
	tkp.fd = tty_fd;
	tkp.baud_rate = spd_baud_rate;
	ret = pthread_create(&tkt, NULL, handle_knx_tty, (void *)&tkp);
	if (ret != 0) {
		fprintf(stderr, "thread knx failed...\n");
		goto out;
	}
	#endif
	
	#if 1
	pthread_t tsk;
	struct thread_socket_arg tsp;
	tsp.server = DEFAULT_SERVER_ADDRESS;
	tsp.port = port;
	ret = pthread_create(&tsk, NULL, handle_socket, (void *)&tsp);
	if (ret != 0) {
		fprintf(stderr, "thread socket failed...\n");
		goto out;
	}
	#endif
	
	fprintf(stdout, "knx UART thread:%u\n", ((unsigned int)tkt));
	fprintf(stdout, "socket thread:%u\n", ((unsigned int)tsk));
	
	pthread_join(tkt, NULL);
	pthread_join(tsk, NULL);
    fprintf(stdout, "shuting down knx...\n");

out:
	knx_deinit_queue_by_index(QUEUE_INDEX_KNX_RX);
	knx_deinit_queue_by_index(QUEUE_INDEX_KNX_TX);
	if (tty_fd >= 0)
		close(tty_fd);
    return 0;
}

// show the program's version
static void show_version(void)
{
	fprintf(stderr,
		"knx v" VERSION_STR "\n"
		"User space daemon for KNX Devices' management,\n"
		"KNX relevant device Controller and Manager\n"
		"Copyright (c) 2014, VIA Networking Division "
		"and contributors\n");
}

// show the usage of this program
static void usage(void)
{
	show_version();
	fprintf(stderr,
		"\n"
		"usage: knx [-BPhs] [-b <baud rate>] [-d <device node file>] [-r <role>]"
		"\\\n"
		"\n"
		"options:\n"
		"   -h   show this usage\n"
		"   -b   baud rate\n"
		"   -d   device node file (e.g. /dev/ttyS0, /dev/ttyUSB2)\n"
		"   -r   work role (0:transmitter, 1:receiver, 2:repeater)\n"
		"   -p   server port/service (the server address/Name is fixed as localhost=127.0.0.1)\n"
		"   -s   enable rssi information (only effective under role of receiver)\n"
		"   -B   run daemon in the background\n"
		"   -P   do not print debug message\n"
		"   -v   show program version\n");

	exit(1);
}

// program initiation function
static void program_init(char **dev,
                 speed_t *spd,
                 bool *rssi,
                 uint8_t *role,
                 char **p)
{
	fputs("enter program init function\n", stdout);

    (*dev) = "/dev/ttyUSB0";
    (*spd) = B19200;
    (*rssi) = false;
    (*role) = ROLE_TRANSMITTER_RECEIVER;
    (*p) = "echo";

	int ret = 0;
	struct circular_queue *q = NULL;
	
	//ret = knx_protocol_init_queue_tx();
	fputs("init knx txqueue\n", stdout);
	ret = knx_init_queue_by_index(QUEUE_INDEX_KNX_TX);
	if (ret < 0) {
		fprintf(stderr, "%s %d init tx queue failed\n", __func__, __LINE__);
		goto out;
	} else
		fprintf(stdout, "txq init successfully\n");
	q = knx_get_queue(QUEUE_INDEX_KNX_TX);
	if (!q) {
		fprintf(stdout, "null pointer\n");
	}
	

	//ret = knx_protocol_init_queue_rx();
	fputs("init knx rxqueue\n", stdout);
	ret = knx_init_queue_by_index(QUEUE_INDEX_KNX_RX);
	if (ret < 0) {
		fprintf(stderr, "%s %d init rx queue failed\n", __func__, __LINE__);
		goto out;
	} else
		fprintf(stdout, "rxq init successfully\n");
	q = knx_get_queue(QUEUE_INDEX_KNX_RX);
	if (!q) {
		fprintf(stdout, "null pointer\n");
	}
	
	fputs("program init function done\n", stdout);	
out:
	return;
}

static void sighandler(int signum)
{
	fprintf(stdout, "got %d signum\n", signum);
	gFlagExit = true;
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
