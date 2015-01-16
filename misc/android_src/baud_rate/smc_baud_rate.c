#include <stdio.h>
#include <unistd.h>
#include <termios.h>

int main(int argc, char *argv[])
{
    FILE *tty_fd = NULL;
    struct termios to;

    tty_fd = fopen("/dev/ttyUSB0", "r+");

    memset(&to, 0, sizeof(to));
    tcgetattr(fileno(tty_fd), &to);

    speed_t sp = cfgetispeed(&to);

    printf("baud rate output is B9600: %d\n", sp|B9600);
    printf("data bit: %d\n", to.c_cflag & CS8);
    printf("stop bit: %d\n", to.c_cflag & CSTOPB);
    printf("parity bit: %d\n", to.cflag & PARENB);
    printf("flow control xon: %d\n", to.c_iflag & IXON);
    printf("flow control xoff: %d\n", to.c_iflag & IXOFF);

    int r = 0;
    char buffer[10] = {0x0, 0x1, 0x0, 0x2, 0x0, 0x61, 0x16, 0xaa, 0xbb, 0xcc};
    while (1) {

        r = fwrite(buffer, sizeof(char), strlen(buffer), tty_fd);
        printf("r = %d bytes", r);
        sleep(2);
    }

    return 0;
}
