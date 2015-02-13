#include "log.h"
#include "knxCommon.h"
#include "threadKnxTty.h"

void* handle_knx_tty(void *arg)
{
    /*
     * below is the example for transmit frame format, 1st byte is length(not including itseldf).
	 * char buffer[64] = {0xb, 0x0, 0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, 0xaa, 0xbb, 0xcc, 0xff};
     */
    struct thread_knx_arg *knx_arg = (struct thread_knx_arg *)arg;
    unsigned char buffer[RC1180_MAX_BUFFER_SIZE] = {
		0x14, 0x0,\
		0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, \
		'h', 'e', 'l', 'l', 'o', ',', 'w', 'o', 'r', 'l', 'd','!'};
    unsigned char buffer_r[RC1180_MAX_BUFFER_SIZE] = {0};
    
    int i;
    for (i = 0; i < 22; i++)
		fprintf(stdout, "%02x ", buffer[i]);
	fprintf(stdout, "\n");
    
	fd_set tty_set;
    int tty_fd = knx_arg->fd;
    int ret = 0, frame_length = 0, failed = 0;
    int maxDescriptor = tty_fd + 1, \
		actual_len = 0, finished = 0, nr = 0;
	speed_t spd_baud_rate = knx_arg->baud_rate;

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

	#if 0
    struct circular_queue *txq = NULL, *rxq = NULL;
    txq = knx_get_queue(QUEUE_INDEX_KNX_TX);
	if (!txq) {
		fprintf(stderr, "%s %d get tx queue failed\n", __func__, __LINE__);
		goto out;
	}

    rxq = knx_get_queue(QUEUE_INDEX_KNX_RX);
	if (!rxq) {
		fprintf(stderr, "%s %d get rx queue failed\n", __func__, __LINE__);
		goto out;
	}
	#endif
	
	struct pollfd fds[MAX_NUM_FILES_DES];
	fds[0].fd = tty_fd; // add uart knx file descriptor
	fds[0].events = POLLIN|POLLRDNORM; // register pollin event
	fds[1].fd = tty_fd; // add uart knx file descriptor
	fds[1].events = POLLOUT|POLLWRNORM; // register pollout event
	
	gTxInterval = transfer_wait_time(spd_baud_rate, frame_length);
	
     do {
		
		if (poll(fds, MAX_NUM_FILES_DES, POLL_TIME_OUT_MS) <= 0) {
			perror("no file descriptor is available");
			goto next_turn;
		}
		
		if ((fds[0].revents & POLLIN) || (fds[0].revents & POLLRDNORM)) {
			fputs("pollin available\n", stdout);
			knxHandleUARTBuffer(tty_fd);
			fputs("pollin done\n", stdout);
		}
        
		next_turn:
			sleep(1);
			continue;
    } while (gFlagExit != true); // while (running)

out:
    fprintf(stdout, "thread %u is going to shutdown\n", ((unsigned int)pthread_self()));
	return (void *)NULL;
}
