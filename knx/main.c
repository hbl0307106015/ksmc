#include "knxCommon.h"
#include "knxProtocol.h"
#include "threadKnxTty.h"
#include "threadSocket.h"
#include "circularQueue.h"
#include "timerHeap.h"
#include "log.h"

// marcos
#define VERSION_STR "0.3.0"
#define KNX_THREAD_NUM 3

// global variables
bool gEnableRssi = false;
bool gFlagExit = false;
uint8_t gKNXState = STATE_UNALIVE;
uint8_t gNetworkRole = 0;
uint16_t gTxInterval = 0;

struct timer_heap gTimer_heap;

pthread_t thread_manager[KNX_THREAD_NUM];

// global function declaration
void* handle_knx_tty(void *arg);
void* handle_socket(void *arg);
void* handle_timer(void *arg);

// show the program's version
static void show_version(void)
{
	fprintf(stderr,
		"knx v" VERSION_STR "\n"
		"User space daemon for KNX Devices' management,\n"
		"KNX relevant device Controller and Manager\n"
		"Copyright (c) 2014, Networking Division "
		"and contributors\n");
}

// show the usage of this program
static void usage(void)
{
	show_version();
	fprintf(stderr,
		"\n"
		"usage: knx [-hs] [-b <baud rate>] [-d <device node file>] [-r <role>]"
		"\\\n"
		"\n"
		"options:\n"
		"   -h   show this usage\n"
		"   -b   baud rate\n"
		"   -d   device node file (e.g. /dev/ttyS0, /dev/ttyUSB2)\n"
		"   -r   work role (0:transmitter, 1:receiver, 2:repeater)\n"
		"   -p   server port/service (the server address/Name is fixed as localhost=127.0.0.1)\n"
		"   -s   enable rssi information (only effective under role of receiver)\n"
		"   -B   run daemon in the background (not support yet, comming soon...)\n"
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
    (*dev) = "/dev/ttyUSB0";
    (*spd) = B19200;
    (*rssi) = false;
    (*role) = ROLE_TRANSMITTER_RECEIVER;
    (*p) = "echo";
    
    int i;
    for (i = 0; i < KNX_THREAD_NUM; i++)
		memset(&(thread_manager[i]), 0, sizeof(pthread_t));
	
	// init the timer heap
	timer_heap_init(DEFAULT_TIMER_HEAP_SIZE);
	
	int ret = 0;

	ret = knx_protocol_init_queue_tx();
	if (ret < 0) {
		fprintf(stderr, "%s %d init tx queue failed\n", __func__, __LINE__);
		exit(0);
	} else
		fprintf(stdout, "txq init successfully\n");

	ret = knx_protocol_init_queue_rx();
	if (ret < 0) {
		fprintf(stderr, "%s %d init rx queue failed\n", __func__, __LINE__);
		exit(0);
	} else
		fprintf(stdout, "rxq init successfully\n");
}

static void sighandler(int signum)
{
	fprintf(stdout, "got %d signum\n", signum);
	gFlagExit = true;
}

int main(int argc, char *argv[])
{
    int ret = 0;
	char *dev = NULL;
	char *port = NULL;
	speed_t spd_baud_rate;
	uint16_t int_baud_rate;

    program_init(&dev,
                 &spd_baud_rate,
                 &gEnableRssi,
                 &gNetworkRole,
                 &port);

	int c = 0;
	for (;;) {
		c = getopt(argc, argv, "b:d:hp:r:sv");
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
		"port number: %s\n",
		spd_baud_rate,
		dev,
		gEnableRssi,
		gNetworkRole,
		port);

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
	
	struct sigaction sigact;
	sigact.sa_handler = sighandler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGQUIT, &sigact, NULL);


	pthread_t tkt;
	struct thread_knx_arg tkp;
	tkp.fd = tty_fd;
	tkp.baud_rate = spd_baud_rate;
	ret = pthread_create(&tkt, NULL, handle_knx_tty, (void *)&tkp);
	if (ret != 0) {
		perror("thread knx failed...\n");
		goto out;
	}
	thread_manager[0] = tkt;
	
	pthread_t tsk;
	struct thread_socket_arg tsp;
	tsp.server = DEFAULT_SERVER_ADDRESS;
	tsp.port = port;
	ret = pthread_create(&tsk, NULL, handle_socket, (void *)&tsp);
	if (ret != 0) {
		perror("thread socket failed...\n");
		goto out;
	}
	thread_manager[1] = tsk;
	
	pthread_t tmr;
	ret = pthread_create(&tmr, NULL, handle_timer, (void *)NULL);
	if (ret != 0) {
		perror("thread timer failed...\n");
		goto out;	
	}
	thread_manager[2] = tmr;
	
	fprintf(stdout, "knx UART thread:%u\n", ((unsigned int)tkt));
	fprintf(stdout, "socket thread:%u\n", ((unsigned int)tsk));
	fprintf(stdout, "timer thread:%u\n", ((unsigned int)tmr));
	
	pthread_join(tkt, NULL);
	pthread_join(tsk, NULL);
	pthread_join(tmr, NULL);
	fflush(stdout);
	fflush(stderr);
    fprintf(stdout, "shuting down knx...\n");

out:
	knx_protocol_deinit_queue_rx();
	knx_protocol_deinit_queue_tx();
	if (tty_fd >= 0)
		close(tty_fd);
    return 0;
}
