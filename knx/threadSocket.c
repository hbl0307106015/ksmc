#include "log.h"
#include "knxCommon.h"
#include "knxProtocol.h"
#include "threadSocket.h"



void handle_smc_packet(unsigned char *b, size_t len);

void* handle_socket(void *arg)
{
	int sock = -1;
    struct thread_socket_arg *socket_arg = (struct thread_socket_arg *)arg;
    
    // tell the system what kind(s) of address info we want
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC; // any address family
    addrCriteria.ai_socktype = SOCK_DGRAM; // only datagram sockets
    addrCriteria.ai_protocol = IPPROTO_UDP; // only udp protocol
    
    // get address(es)
    struct addrinfo *servAddr = NULL; // the result of below calls
    int ret = getaddrinfo(socket_arg->server, socket_arg->port, \
		&addrCriteria, &servAddr);
	if (ret != 0) {
		perror("socket thread, getaddrinfo()");
		goto out;
	}
	
	// create a datagram/UDP socket
	sock = socket(servAddr->ai_family, servAddr->ai_socktype,
		servAddr->ai_protocol);
	if (sock < 0) {
		perror("socket thread, socket()");
		goto out;
	}
	
	fputs("create thread socket successfully\n", stdout);
	
	ssize_t numBytes = 0;
	struct sockaddr_storage fromAddr;
	socklen_t fromAddrLen = sizeof(fromAddr); // source address of server
	char buffer[MAX_STRING_LENGTH + 1] = {0}; // I/O buffer
	
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
	
	struct pkt_t *pkt = NULL;
	struct pollfd fds[MAX_NUM_FILES_DES];
	fds[0].fd = sock; // add socket file descriptor
	fds[0].events = POLLIN|POLLRDNORM;// |POLLOUT|POLLWRNORM; // register pollin & pollout event
	fds[1].fd = sock; // add socket file descriptor
	fds[1].events = POLLOUT|POLLWRNORM; // register pollin & pollout event

    do {
		
		if (poll(fds, MAX_NUM_FILES_DES, POLL_TIME_OUT_MS) <= 0) {
			perror("no file descriptor is available");
			goto next_turn;
		}
		
		// check writable availability
		if ((fds[1].revents & POLLOUT)||(fds[1].revents & POLLWRNORM)) {
			fputs("pollout available\n", stdout);
			
			if (is_queue_empty(rxq)) {
				fputs("queue rx on knx is null\n", stdout);
				goto next_turn;
			}
			
			/* retrieve a packet from rx queue if exist */
			pthread_mutex_lock(&(rxq->qmutex));
			pkt = (struct pkt_t *)knx_protocol_retrieve_packet(&rxq);
			pthread_mutex_unlock(&(rxq->qmutex));
			
			if (pkt && pkt->u) {
				fprintf(stdout, "dequeued a packet, len=%d\n", (int)pkt->length);
				dump_buffer(pkt->u, pkt->length);
				
				fprintf(stdout, "sending a packet to smc\n");
				/* send packet to smart controller */
				numBytes = sendto(sock, pkt->u, pkt->length, 0, \
					servAddr->ai_addr, servAddr->ai_addrlen);
				fprintf(stdout, "%d bytes sent\n", numBytes);

				if (numBytes < 0) {
					perror("socket thread, numBytes < 0");
					pthread_exit(NULL);
				} else if ((size_t)numBytes != pkt->length) {
					perror("socket thread, numBytes != packet length");
					pthread_exit(NULL);
				}
				
				free(pkt->u);
				free(pkt);
			} else {
				fputs("warning!!! there is a null packet in queue\n", stdout);
			}

			fputs("pollout done\n", stdout);
			goto next_turn;
		}
		
		#if 0
		if ((fds[0].revents & POLLIN)||(fds[0].revents & POLLRDNORM)) {
			fputs("pollin available\n", stdout);
			fputs("pollin done\n", stdout);
		}
		#endif
		
		next_turn:
			sleep(1);
			continue;

    } while (gFlagExit != true); /* while (running) */
	
out:
	fprintf(stdout, "thread %u is going to shutdown\n",  ((unsigned int)pthread_self()));
	fflush(stdout);
	fflush(stderr);
	freeaddrinfo(servAddr);
	close(sock);
    return ((void *)NULL);
}
