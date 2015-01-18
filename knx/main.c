#include "knxCommon.h"
#include "knxProtocol.h"
#include "log.h"

bool gEnableRssi = false;
uint8_t gNetworkRole = 0;

extern char *optarg;

#define VERSION_STR "0.0.2"

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
		"   -B   run daemon in the background\n"
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
    int r = 0;
    int tty_fd = 0;
    struct termios to, new_to;

	#if 0
	if (argc != 3) {
		printf("Usage: <program> <tty device> <role:0 or 1>\n"
			"\texample: ./smc /dev/ttyUSB0 1\n");
		exit(1);
	}

    char *dev = argv[1];
    int role = atoi(argv[2]);
    printf("target device (%s) (%s)\n", dev, role ? ("sender") : ("receiver"));
   	#endif
   	
	int c;
	char *dev = NULL;
	speed_t spd_baud_rate;
	uint16_t int_baud_rate;

    program_init(&dev,
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

    tty_fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY);
	if (tty_fd < 0) {
		perror("open file");
		exit(1);
	}

	/* show the current setting */
    memset(&to, 0, sizeof(to));
    tcgetattr(tty_fd, &to);
    speed_t sp_o = cfgetospeed(&to);
    speed_t sp_i = cfgetispeed(&to);
    printf("output baud rate output is %d ?: %d\n", spd_baud_rate, sp_o == spd_baud_rate);
    printf("input baud rate input is %d ? : %d\n", spd_baud_rate, sp_i == spd_baud_rate);
    printf("data bit: %d\n", (to.c_cflag) & CS8);
    printf("stop bit: %d\n", (to.c_cflag) & CSTOPB);
    printf("parity 1 bit: %d\n", (to.c_cflag) & PARENB);
    printf("parity 2 bit: %d\n", (to.c_cflag) & PARODD);
    printf("RTS/CTS: %d\n", (to.c_cflag) & CRTSCTS);
    printf("flow control xon: %d\n", (to.c_iflag) & IXON);
    printf("flow control xoff: %d\n", (to.c_iflag) & IXOFF);
    printf("if enable receiver: %d\n", (to.c_cflag) & CREAD);
	printf("vtime & VMIN %d %d\n", to.c_cc[VTIME], to.c_cc[VMIN]);

	/* config new termios struct and apply it */
	#if 0
    memset(&new_to, 0, sizeof(new_to));
    new_to.c_cflag |= (CLOCAL | CREAD); /* Ignore modem control lines, enable receiver */
    new_to.c_cflag &= (~CSIZE); /*Clear character size mask.  Values are CS5, CS6, CS7, or CS8. */
    
    new_to.c_cflag |= CS8; /* 8 data bit */
    new_to.c_cflag &= (~PARENB); /* no parity */
    new_to.c_cflag &= (~CSTOPB); /* 1 stop bit */
    
    /* none check bit */
    new_to.c_cflag &= (~CRTSCTS);
	new_to.c_iflag &= (~IXON);
    new_to.c_iflag &= (~IXOFF);
    
    new_to.c_cc[VTIME] = 0; /* minimum wait time */
    new_to.c_cc[VMIN] = 0; /* minimum of received packet length */
    
    /* set baudrate as 9600 */
    cfsetispeed(&new_to, B19200);
    cfsetospeed(&new_to, B19200);
    
    /* apply the setting right now */
    if (tcsetattr(tty_fd, TCSANOW, &new_to) != 0) {
        perror("tcsetattr error");
        exit(1);
    }
	#endif
	
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

    //char buffer[64] = {0xb, 0x0, 0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, 0xaa, 0xbb, 0xcc, 0xff}; /* raw, ok */
    
    //char buffer[64] = {0xb, 0x0, 0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, 0xaa, 0xbb, 0xcc}; /* no 0xff, can be received, but the data malformed */
    //char buffer[64] = {0xa, 0x0, 0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, 0xaa, 0xbb, 0xcc}; /* modified the length item,  can be received, but the data malformed*/
    /* from the above test, the content on first byte indicates the length about knx_ctrl and followed data, including l/npci, tpci/apci, user data.
     * 0xff may be the end-character of the data stream.
     * */
    
    //char buffer[64] = {0xa, 0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, 0xaa, 0xbb, 0xcc, 0xff}; /* no knx-ctrl , malformed */
    //char buffer[64] = {0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, 0xaa, 0xbb, 0xcc,0xff}; /* no length & knx-ctrl, ... */
    
    unsigned char buffer[64] = {
		0x14, 0x0,\
		0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, \
		'h', 'e', 'l', 'l', 'o', ',', 'w', 'o', 'r', 'l', 'd','!'};
    unsigned char buffer_r[64] = {0};
    
    int i;
    for (i = 0; i < 22; i++)
		printf("%02x ", buffer[i]);
	printf("\n");
    
	fd_set tty_set;
    bool running = true;
    int frame_length = 0, rssi_enable = 1, failed = 0;
    int maxDescriptor = tty_fd + 1, \
		actual_len = 0, finished = 0, nr = 0;
    
    while (running) {

		FD_ZERO(&tty_set);
		FD_SET(tty_fd, &tty_set);

        if (gNetworkRole) {
            printf("try to write data, result:");
            r = write(tty_fd, buffer, 21);
            printf("r = %d bytes\n", r);
            sleep(3);
        } else {
            //printf("try to read data, result:");

			if (select(maxDescriptor, &tty_set, NULL, NULL, NULL) <= 0)
				continue;

			if (FD_ISSET(tty_fd, &tty_set)) {
				
				#if 1
				/*
				 * read 1 bytes at first, and then the bytes_read 
				 * will be modified by the actual status. 
				*/
				actual_len = read(tty_fd, buffer_r, 1);
				
				if (actual_len == 0)
					continue;

				frame_length = buffer_r[0];
				if (actual_len == 1) {
					fprintf(stdout, "frame_length ?= %02x ", frame_length);
					if (frame_length == 0x0) {
						if (rssi_enable) {
							usleep(180 + 1770 + 900); /* 1770 = 590 * 3 */
							read(tty_fd, buffer_r, 1);
						}
						fprintf(stdout, "\n");
					} else {
						usleep(180 + (660 * frame_length) + 900);
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
				#endif
				
				
			} /* FD is set */
			#if 1
			if (finished) {
				//bytes_read = 0;
				//dump_buffer(buffer_r, frame_length);
				dump_buffer_appl_data(buffer_r, frame_length, DUMP_FORMAT_STRING);
				finished = 0;
			}
			#endif
			
        } /* role == 0 */
        
    } /* while (running) */

    close(tty_fd);
    return 0;
}
