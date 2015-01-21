#include "knxCommon.h"
#include "knxProtocol.h"
#include "log.h"

/* global variables */
bool gEnableRssi = false;
bool gFlagExit = false;
uint8_t gNetworkRole = 0;
uint16_t gTxInterval = 0;

/* global function declaration */
extern char *optarg;
extern int getopt(int argc, char * const argv[], const char *optstring);
extern int sigaction(int s, __const struct sigaction *sg, struct sigaction *osg);
extern int sigemptyset(sigset_t *s);

/* marcos */
#define VERSION_STR "0.0.2"

void* handle_knx_tty(void *arg);
void* handle_socket(void *arg);

/* show the program's version */
static void show_version(void)
{
	fprintf(stderr,
		"knx v" VERSION_STR "\n"
		"User space daemon for KNX Devices' management,\n"
		"KNX relevant device Controller and Manager\n"
		"Copyright (c) 2014, VIA Networking Division "
		"and contributors\n");
}

/* show the usage of this program */
static void usage(void)
{
	show_version();
	fprintf(stderr,
		"\n"
		"usage: smc [-hs] [-b <baud rate>] [-d <device node file>] [-r <role>]"
		"\\\n"
		"\n"
		"options:\n"
		"   -h   show this usage\n"
		"   -b   baud rate\n"
		"   -d   device node file (e.g. /dev/ttyS0, /dev/ttyUSB2)\n"
		"   -r   work role (0:transmitter, 1:receiver, 2:repeater)\n"
		"   -s   enable rssi information (only effective under role of receiver)\n"
		"   -B   run daemon in the background (not support yet, comming soon...)\n"
		"   -v   show program version\n");

	exit(1);
}

/* program initiation function */
static void program_init(char **dev,
                 speed_t *spd,
                 bool *rssi,
                 uint8_t *role)
{
    (*dev) = "/dev/ttyUSB0";
    (*spd) = B19200;
    (*rssi) = false;
    (*role) = ROLE_TRANSMITTER_RECEIVER;
}

static void sighandler(int signum)
{
	gFlagExit = true;
}

int main(int argc, char *argv[])
{
    int ret = 0;
	char *dev = NULL;
	speed_t spd_baud_rate;
	uint16_t int_baud_rate;

    program_init(&dev,
                 &spd_baud_rate,
                 &gEnableRssi,
                 &gNetworkRole);

	int c = 0;
	for (;;) {
		c = getopt(argc, argv, "b:d:hr:sv");
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
		"network role: %u\n",
		spd_baud_rate,
		dev,
		gEnableRssi,
		gNetworkRole);

	int tty_fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY);
	if (tty_fd < 0) {
		perror("open file");
		exit(1);
	}

	/* fill out new termios struct and apply it */
	set_tty_attr(tty_fd, spd_baud_rate);

	/* show the current setting */
	show_tty_attr(tty_fd);
	
	/* restore the status of UART device, i.e. to the block-status(default) */
	if (fcntl(tty_fd, F_SETFL, 0) < 0) {
		perror("fcntl set flag");
		exit(1);
	}
	
	/* to verify whether the fd is ready for a terminal */
	if (isatty(tty_fd) == 0) {
		perror("this is not a terminal device");
		exit(1);
	}
	
	struct sigaction sigact;
	sigact.sa_handler = sighandler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGQUIT, &sigact, NULL);

    /*
     * below is the example for transmit frame format, 1st byte is length(not including itseldf).
	 * char buffer[64] = {0xb, 0x0, 0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, 0xaa, 0xbb, 0xcc, 0xff};
     */
    
    unsigned char buffer[RC1180_MAX_BUFFER_SIZE] = {
		0x14, 0x0,\
		0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, \
		'h', 'e', 'l', 'l', 'o', ',', 'w', 'o', 'r', 'l', 'd','!'};
    unsigned char buffer_r[RC1180_MAX_BUFFER_SIZE] = {0};
    
    int i;
    for (i = 0; i < 22; i++)
		printf("%02x ", buffer[i]);
	printf("\n");
    
	fd_set tty_set;
    //bool running = true;
    int frame_length = 0, failed = 0;
    int maxDescriptor = tty_fd + 1, \
		actual_len = 0, finished = 0, nr = 0;
    
    while (gFlagExit != true) {

		FD_ZERO(&tty_set);
		FD_SET(tty_fd, &tty_set);

        if (gNetworkRole) {
            printf("try to write data, result:");
            ret = write(tty_fd, buffer, 21);
            printf("r = %d bytes\n", ret);
            sleep(3);
        } else {
            //printf("try to read data, result:");
			if (select(maxDescriptor, &tty_set, NULL, NULL, NULL) <= 0)
				continue;

			if (FD_ISSET(tty_fd, &tty_set)) {

				/*
				 * read 1 bytes at first, and then the bytes_read 
				 * will be modified by the actual status. 
				*/
				actual_len = read(tty_fd, buffer_r, 1);
				
				if (actual_len == 0)
					continue;

				frame_length = buffer_r[0];
				gTxInterval = transfer_wait_time(spd_baud_rate, frame_length);

				if (actual_len == 1) {
					fprintf(stdout, "frame_length ?= %02x ", frame_length);
					if (frame_length == 0x0) {
						if (gEnableRssi) {
							//usleep(180 + 1770 + 900); /* 1770 = 590 * 3 */
							usleep(gTxInterval);
							read(tty_fd, buffer_r, 1);
						}
						fprintf(stdout, "\n");
					} else {
						usleep(gTxInterval);
						//usleep((unsigned int)(180 + (590 * 3 * frame_length) + 900));
						actual_len = read(tty_fd, buffer_r, frame_length);
						fprintf(stdout, "NO = %d, actual_length := %02x ", (++nr), actual_len);
						if (frame_length == actual_len)
							finished = 1;
						else {
							failed++;
							dump_buffer(buffer_r, actual_len);
							fprintf(stdout, "\n");
						}
					}
				} else {
					fprintf(stderr, "unknown data: ");
					dump_buffer(buffer_r, actual_len);
				}
				
			} /* FD is set */

			if (finished) {
				//dump_buffer(buffer_r, frame_length);
				dump_buffer_appl_data(buffer_r, frame_length, DUMP_FORMAT_STRING);
				finished = 0;
			}
			
        } /* role == 0 */
        
    } /* while (running) */

    fprintf(stderr, "shuting down...\n");

out:
	if (tty_fd >= 0)
		close(tty_fd);
    return 0;
}
