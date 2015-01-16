#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int r = 0;
    //FILE *tty_fd = NULL;
    int tty_fd = NULL;
    struct termios to;

    //tty_fd = fopen("/dev/ttyUSB0", "r+");
    //tty_fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);
    char *dev = argv[1];
    int role = atoi(argv[2]);
    printf("target device (%s)\n", argv[1]);
    tty_fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY);

    memset(&to, 0, sizeof(to));
    //tcgetattr(fileno(tty_fd), &to);
    tcgetattr(tty_fd, &to);

    speed_t sp_o = cfgetospeed(&to);
    speed_t sp_i = cfgetispeed(&to);

    printf("output baud rate output is B9600: %d\n", sp_o & B9600);
    printf("input baud rate input is B9600: %d\n", sp_i & B9600);
    printf("data bit: %d\n", (to.c_cflag) & CS8);
    printf("stop bit: %d\n", (to.c_cflag) & CSTOPB);
    to.c_cflag |= (CSTOPB);
    printf("parity 1 bit: %d\n", (to.c_cflag) & PARENB);
    printf("parity 2 bit: %d\n", (to.c_cflag) & PARODD);
    to.c_cflag &= (~PARENB);
    printf("RTS/CTS: %d\n", (to.c_cflag) & CRTSCTS);
    printf("flow control xon: %d\n", (to.c_iflag) & IXON);
    printf("flow control xoff: %d\n", (to.c_iflag) & IXOFF);
    to.c_iflag &= (~IXON);
    //if (tcsetattr(fileno(tty_fd), TCSANOW, &to) != 0) {
    if (tcsetattr(tty_fd, TCSANOW, &to) != 0) {
        perror("tcsetattr error");
        return -1;
    }

    memset(&to, 0, sizeof(to));
    //tcgetattr(fileno(tty_fd), &to);
    tcgetattr(tty_fd, &to);
    speed_t sp_o2 = cfgetospeed(&to);
    speed_t sp_i2 = cfgetispeed(&to);
    printf("output baud rate output is B9600: %d\n", sp_o2 & B9600);
    printf("input baud rate input is B9600: %d\n", sp_i2 & B9600);
    printf("data bit: %d\n", (to.c_cflag) & CS8);
    printf("stop bit: %d\n", (to.c_cflag) & CSTOPB);
    printf("parity 1 bit: %d\n", (to.c_cflag) & PARENB);
    printf("parity 2 bit: %d\n", (to.c_cflag) & PARODD);
    printf("RTS/CTS: %d\n", (to.c_cflag) & CRTSCTS);
    printf("flow control xon: %d\n", (to.c_iflag) & IXON);
    printf("flow control xoff: %d\n", (to.c_iflag) & IXOFF);

    //char buffer[64] = {0xb, 0x0, 0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, 0xaa, 0xbb, 0xcc, 0xff}; /* raw, ok */
    
    //char buffer[64] = {0xb, 0x0, 0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, 0xaa, 0xbb, 0xcc}; /* no 0xff, can be received, but the data malformed */
    //char buffer[64] = {0xa, 0x0, 0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, 0xaa, 0xbb, 0xcc}; /* modified the length item,  can be received, but the data malformed*/
    /* from the above test, the content on first byte indicates the length about knx_ctrl and followed data, including l/npci, tpci/apci, user data.
     * 0xff may be the end-character of the data stream.
     * */
    
    //char buffer[64] = {0xa, 0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, 0xaa, 0xbb, 0xcc, 0xff}; /* no knx-ctrl , malformed */
    //char buffer[64] = {0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, 0xaa, 0xbb, 0xcc,0xff}; /* no length & knx-ctrl, ... */
    
    char buffer[64] = {
		0x14, 0x0,\
		0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, \
		'h', 'e', 'l', 'l', 'o', ',', 'w', 'o', 'r', 'l', 'd','!', 0xff};
    char buffer_r[64] = {0};
    
    int i;
    for (i = 0; i < 22; i++)
		printf("%02x ", buffer[i]);
	printf("\n");
    
    while (1) {

        if (role) {
            printf("try to write data, result:");
            //r = fwrite(buffer, sizeof(char), 10, fileno(tty_fd));
            r = write(tty_fd, buffer, 21);
            printf("r = %d bytes\n", r);
            sleep(3);
        } else {
            printf("try to read data, result:");
            r = read(tty_fd, buffer_r, 64);
            printf("r = %d bytes (%s)\n", r, buffer_r);
            sleep(1);
        }
    }

    //fclose(fileno(tty_fd));
    close(tty_fd);
    return 0;
}
