#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <termios.h>
#include <fcntl.h>


/* Macro */
#define GET_OFFSET_RECV_C(b) (b + 0)
#define GET_OFFSET_RECV_HEADER(b) (b + 1)
#define GET_OFFSET_RECV_RF_INFO(b) (b + 2)
#define GET_OFFSET_RECV_KNX_CTRL(b) (b + 9)
//get the offset of TX serial number of Domain Address
#define GET_OFFSET_RECV_TXSN_OR_DA(b) (b + 3)
#define GET_OFFSET_RECV_APPL_HEADER(b) (b + 10)
#define GET_OFFSET_RECV_SRC(b) GET_OFFSET_RECV_APPL_HEADER(b)
#define GET_OFFSET_RECV_APPL_DATA (b + 17)

#define LENGTH_BEFORE_APPL_DATA 17

enum {
	FORMAT_STRING = 0,
	FORMAT_HEX = 1,
};


/* Function */

/*
 * dump_buffer
 * @b	: buffer
 * @len	: length of buffer
 * */
static void dump_buffer(unsigned char *b, size_t len);

/*
 * dump_buffer_appl_data
 * @b	: buffer
 * @len	: length of buffer
 * @flag: format of the dump function, hex or string ?
 * */
static void dump_buffer_appl_data(unsigned char *b, size_t len, uint8_t flag);

int main(int argc, char *argv[])
{
    int r = 0;
    int tty_fd = 0;
    struct termios to, new_to;

	if (argc != 3) {
		printf("Usage: <program> <tty device> <role:0 or 1>\n"
			"\texample: ./smc /dev/ttyUSB0 1\n");
		exit(1);
	}
	
    char *dev = argv[1];
    int role = atoi(argv[2]);
    printf("target device (%s) (%s)\n", dev, role ? ("sender") : ("receiver"));
    
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
    printf("output baud rate output is B9600: %d\n", sp_o & B9600);
    printf("input baud rate input is B9600: %d\n", sp_i & B9600);
    printf("data bit: %d\n", (to.c_cflag) & CS8);
    printf("stop bit: %d\n", (to.c_cflag) & CSTOPB);
    printf("parity 1 bit: %d\n", (to.c_cflag) & PARENB);
    printf("parity 2 bit: %d\n", (to.c_cflag) & PARODD);
    #if defined(_BSD_SOURCE) || defined(_SVID_SOURCE)
    printf("RTS/CTS: %d\n", (to.c_cflag) & CRTSCTS);
    #endif /* defined(_BSD_SOURCE) || defined(_SVID_SOURCE) */
    printf("flow control xon: %d\n", (to.c_iflag) & IXON);
    printf("flow control xoff: %d\n", (to.c_iflag) & IXOFF);
    printf("if enable receiver: %d\n", (to.c_cflag) & CREAD);
	printf("vtime & VMIN %d %d\n", to.c_cc[VTIME], to.c_cc[VMIN]);

    #if 0
	/* config new termios struct and apply it */
    memset(&new_to, 0, sizeof(new_to));
    new_to.c_cflag |= (CLOCAL | CREAD); /* Ignore modem control lines, enable receiver */
    new_to.c_cflag &= (~CSIZE); /*Clear character size mask.  Values are CS5, CS6, CS7, or CS8. */
    
    new_to.c_cflag |= CS8; /* 8 data bit */
    new_to.c_cflag &= (~PARENB); /* no parity */
    new_to.c_cflag &= (~CSTOPB); /* 1 stop bit */
    
    /* none check bit */
    #if defined(_BSD_SOURCE) || defined(_SVID_SOURCE)
    new_to.c_cflag &= (~CRTSCTS);
    #endif /* defined(_BSD_SOURCE) || defined(_SVID_SOURCE) */
	new_to.c_iflag &= (~IXON);
    new_to.c_iflag &= (~IXOFF);
    
    new_to.c_cc[VTIME] = 0; /* minimum wait time */
    new_to.c_cc[VMIN] = 0; /* minimum of received packet length */
    
    /* set baudrate as 9600 */
    cfsetispeed(&new_to, B9600);
    cfsetospeed(&new_to, B9600);
    
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

        if (role) {
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
							usleep((unsigned int)(180 + 1770 + 900)); /* 1770 = 590 * 3 */
							read(tty_fd, buffer_r, 1);
						}
						fprintf(stdout, "\n");
					} else {
						//usleep((unsigned int)(180 + (660 * frame_length) + 900));
						usleep((unsigned int)(180 + (590 * 3 * frame_length) + 900));
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
				
				
				#if 0
				r = read(tty_fd, buffer_r, 1); /* read the first byte which indicate the length of the data frame */
				//usleep(900);
				printf("r = %d content 1st %02x\n", r, buffer_r[0]);
				
				if (!r)
					continue;

				frame_len = buffer_r[0];
				printf("len = %02x ", frame_len);
				
				if (frame_len == 0) {// for the case '00 27'
					r = read(tty_fd, buffer_r, 1); // clear the data frame buffer
					if (r)
						printf("r = %d data = %02x", r, buffer_r[0]);
				} else {
					usleep(180 + (1800 * (frame_len)) + 900); // wait delay time
					r = read(tty_fd, buffer_r, frame_len);
					printf("r = %d data =", r);
					for (i = 0; i < r; i++)
						printf("%02x ", buffer_r[i]);
				}
				printf("\n");
				//sleep(1);
				#endif
				
				
			} /* FD is set */
			#if 1
			if (finished) {
				//bytes_read = 0;
				//dump_buffer(buffer_r, frame_length);
				dump_buffer_appl_data(buffer_r, frame_length, FORMAT_STRING);
				finished = 0;
			}
			#endif
				
			
        } /* role == 0 */
        
    } /* while (running) */

    close(tty_fd);
    return 0;
}

static void dump_buffer(unsigned char *b, size_t len)
{
	if (len <=0)
		return;

	uint32_t i;
	
	fprintf(stdout, "buffer: ");
	for (i = 0; i < len; i++)
		fprintf(stdout, "%02x ", b[i]);
	fprintf(stdout, "\n");
}

static void dump_buffer_appl_data(unsigned char *b, size_t len, uint8_t flag)
{
	if (len < LENGTH_BEFORE_APPL_DATA)
		return;
		
	fprintf(stdout, "application data: ");
	
	if (flag == FORMAT_HEX) {
		dump_buffer(b, len);
        return;
    }
	
	uint32_t actual_len = len - LENGTH_BEFORE_APPL_DATA - 1;
	char *p = (char *)malloc(actual_len + 1);
	
	if (!p) {
		perror("out of memory");
		exit(1);
	}
	
	memcpy(p, b + LENGTH_BEFORE_APPL_DATA, actual_len);
	p[actual_len] = '\0';
	fprintf(stdout, "%s\n", p);
	
	free(p);
}
