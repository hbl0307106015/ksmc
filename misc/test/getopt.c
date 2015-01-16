#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <getopt.h>

bool gEnableRssi = false;
uint8_t gNetworkRole = 0;

extern char *optarg;

#define VERSION_STR "0.0.2"

enum {
    ROLE_TRANSMITTER = 0,
    ROLE_RECEIVER,
    ROLE_TRANSMITTER_RECEIVER,
} network_role;

static void show_version(void)
{
	fprintf(stderr,
		"smc v" VERSION_STR "\n"
		"User space daemon for KNX Devices' management,\n"
		"KNX relevant device Controller and Manager\n"
		"Copyright (c) 2014, VIA Networking "
		"and contributors\n");
}

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
		"   -B   run daemon in the background (comming soon)\n"
		"   -v   show hostapd version\n");

	exit(1);
}

static speed_t transfer_baud_rate(uint16_t int_baud_rate)
{
	switch(int_baud_rate)
	{
		case 9600:
			return B9600;
			break;
		case 19200:
			return B19200;
			break;
		default:
			return B115200;
			break;
	}
}

static void program_init(char **dev,
                 speed_t *spd,
                 bool *rssi,
                 uint8_t *role)
{
    (*dev) = "/dev/ttyUSB0";
    //strcpy(*dev, "/dev/ttyUSB0");
    (*spd) = B19200;
    (*rssi) = true;
    (*role) = ROLE_TRANSMITTER_RECEIVER;
}

int main(int argc, char *argv[])
{
	int c;
	char *dev_node = NULL;
	speed_t spd_baud_rate;
	uint16_t int_baud_rate;

    program_init(&dev_node,
                 &spd_baud_rate,
                 &gEnableRssi,
                 &gNetworkRole);

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
			dev_node = optarg;
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
		dev_node,
		gEnableRssi,
		gNetworkRole);
	
	return 0;
}
