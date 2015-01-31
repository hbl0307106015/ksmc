#include "knxCommon.h"
#include "knxProtocol.h"
#include "log.h"
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
	fflush(stdout);
    
    #if 0
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
    #endif
    
	fd_set tty_set;
    //bool running = true;
    int tty_fd = knx_arg->fd;
    int ret = 0, frame_length = 0, failed = 0;
    int maxDescriptor = tty_fd + 1, \
		actual_len = 0, finished = 0, nr = 0;
	speed_t spd_baud_rate = knx_arg->baud_rate;
	#if 1
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    #endif
    
    struct pkt_t *pkt = NULL;
    struct circular_queue *txq = NULL, *rxq = NULL;
    txq = knx_protocol_get_queue_tx();
	if (!txq) {
		fprintf(stderr, "%s %d get tx queue failed\n", __func__, __LINE__);
		goto out;
	}
	
    rxq = knx_protocol_get_queue_rx();
	if (!rxq) {
		fprintf(stderr, "%s %d get rx queue failed\n", __func__, __LINE__);
		goto out;
	}
	
    while (gFlagExit != true) {
	//while (1) {
           
		//fprintf(stdout, "enter poll\n");
		
		FD_ZERO(&tty_set);
		FD_SET(tty_fd, &tty_set);

        if (gNetworkRole) {
            fprintf(stdout, "try to write data, result:");
            #if 0
            ret = write(tty_fd, buffer, 21);
            fprintf(stdout, "r = %d bytes\n", ret);
            #endif
            
            /* retrieve a packet from tx queue */
			pthread_mutex_lock(&(txq->qmutex));
			pkt = (struct pkt_t *)knx_protocol_retrieve_packet(txq);
			pthread_mutex_unlock(&(txq->qmutex));
			
			if (pkt && pkt->u) {
				/* send the packet to the tty_fd if packet(s) exist */
				ret = write(tty_fd, pkt->u, pkt->length);
			}

            sleep(3);
        } else {
            //fprintf(stdout, "try to read data, result:");
            //fflush(stdout);
            
			if (select(maxDescriptor, &tty_set, NULL, NULL, &tv) <= 0) {
			//if (select(maxDescriptor, &tty_set, NULL, NULL, NULL) <= 0) {
				sleep(1);
				continue;
			}

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
							ret = read(tty_fd, buffer_r, 1);
						}
						fprintf(stdout, "\n");
					} else {
						usleep(gTxInterval);
						//usleep((unsigned int)(180 + (590 * 3 * frame_length) + 900));
						actual_len = read(tty_fd, buffer_r, frame_length);
						fprintf(stdout, "NO = %d, actual_length := %02x ", (++nr), actual_len);
						if (frame_length == actual_len) {
							finished = 1;
							pkt = knx_protocol_alloc_pkt(actual_len);
							knx_protocol_pkt_fill(pkt, buffer_r, actual_len);
							fprintf(stdout, "enqueuing a packet, len=%d\n", (int)pkt->length);
							/* put the packet to the rx queue */
							pthread_mutex_lock(&(rxq->qmutex));
							//enqueue(rxq, pkt);
							knx_protocol_store_packet(rxq, (void *)pkt);
							pthread_mutex_unlock(&(rxq->qmutex));
						} else {
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
				//fprintf(stdout, "threadknxtty received finished: ");
				//dump_buffer(buffer_r, frame_length);
				//dump_buffer_appl_data(buffer_r, frame_length, DUMP_FORMAT_STRING);
				finished = 0;
			}
			
        } /* role == 0 */
        
    } /* while (running) */

out:
    fprintf(stdout, "thread %u is going to shutdown\n", ((unsigned int)pthread_self()));
	return (void *)NULL;
}
