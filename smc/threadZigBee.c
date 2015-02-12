#include "log.h"
#include "threadZigBee.h"
#include "smcPeerUtility.h"
#include "smcProtocol.h"
#include "smcCommon.h"
#include "smcQueue.h"

void* smc_thread_zigbee(void *arg)
{
	// tell the system what kind(s) of address info we want
	struct addrinfo addrCriteria; // criteria for address
	memset(&addrCriteria, 0, sizeof(addrCriteria));
	addrCriteria.ai_family = AF_UNSPEC; // any address family
	addrCriteria.ai_flags = AI_PASSIVE; // accept on any address/port
	addrCriteria.ai_socktype = SOCK_DGRAM; // only datagram socket
	addrCriteria.ai_protocol = IPPROTO_UDP; // only UDP socket
	
	// get address(es)
	int sock = 0;
	struct addrinfo *servAddr = NULL; //list of server address(the result)
	int ret = getaddrinfo(NULL, PORT_NUM_ZIGBEE, &addrCriteria, &servAddr);
	if (ret != 0) {
		DieWithUserMessage("thread zigbee, getaddrinfo() failed", gai_strerror(ret));
		goto out;
	}
	
	// create socket for incoming connections
	sock = socket(servAddr->ai_family, servAddr->ai_socktype, \
		servAddr->ai_protocol);
	if (sock < 0) {
		DieWithSystemMessage("thread zigbee, socket() failed");
		goto out;
	}

	// bind to the local address
	if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0) {
		DieWithSystemMessage("thread zigbee, bind() failed");
		goto out;
	}
	
	// poll fds init
	struct pollfd fds[MAX_NUM_POLL_FILES_DES];
	fds[0].fd = sock;
	fds[0].events = POLLIN | POLLRDNORM;
	fds[1].fd = sock;
	fds[1].events = POLLOUT | POLLWRNORM;

	// queue preparation
	struct circular_queue *txq = NULL;
	txq = smc_get_queue(QUEUE_INDEX_TX_ZIGBEE);
	if (!txq) {
		DieWithUserMessage("smc_get_queue() failed", "null pointer");
		goto out;
	}

	ssize_t numBytesRcvd = 0;
	uint8_t state, buffer[BUFFER_SIZE64] = {0}; // I/O buffer
	struct pkt_t *pkt_generic = NULL;
	struct protocol_data pkt_proto;
	
	// client addr structure
	struct sockaddr clntAddr;
	memset(&clntAddr, 0, sizeof(clntAddr));
	socklen_t clntAddrLen = sizeof(clntAddr);
	
	fputs("thread zigbee enter mainloop\n", stdout);
	do {
		// check if fds are ready ?
		if (poll(fds, MAX_NUM_POLL_FILES_DES, POLL_TIME_OUT_MS) <= 0) {
			perror("no file descriptor is available");
			goto next_turn;
		}
		
		//check readability
		if ((fds[0].revents & POLLIN) || (fds[0].revents & POLLRDNORM)) {
			fputs("thread zigbee, pollin available\n", stdout);
			// data is comming
			numBytesRcvd = recvfrom(sock, buffer, BUFFER_SIZE64,\
				0, (struct sockaddr *)&clntAddr, &(clntAddrLen));
			if (numBytesRcvd < 0) {
				perror("zigbee recefrom error:");
				goto next_turn;
			} else if (numBytesRcvd == 0) {
				fputs("zigbee thread, no data received\n", stdout);
				goto next_turn;
			}
			
			//assemble protocol data structure
			pkt_proto.network_ctx.fd = sock;
			pkt_proto.network_ctx.flags = 0;
			pkt_proto.network_ctx.src_addr = ((struct sockaddr *)(&clntAddr));
			pkt_proto.network_ctx.src_addr_len = clntAddrLen;
			pkt_proto.buffer = buffer;
			pkt_proto.buffer_len = numBytesRcvd;
			
			/* handle data, e.g. the smc will 
			*  set the state to ALIVE so that we can transmit data to it later
			*/
			smc_zigbee_handle_protocol(&pkt_proto);
			fputs("thread zigbee, pollin done\n", stdout);
		}
		
		/* for testing requirement, to imitate a 
		 * command for zigbee.
		 * */
		#if 0
		uint16_t cmd = 0x2001;
		uint8_t val = 0x1;
		uint8_t buffer_test[3] = {0};
		cmd = htons(cmd);
		memcpy(buffer_test, &cmd, sizeof(uint16_t));
		memcpy(buffer_test + 2, &val, sizeof(uint8_t));
		
		struct pkt_t *tpkt = smc_pkt_alloc(sizeof(buffer_test));
		smc_pkt_fill(tpkt, buffer_test, sizeof(buffer_test));

		pthread_mutex_lock(&(txq->qmutex));
		smc_enqueue(txq, (void *)tpkt);
		pthread_mutex_unlock(&(txq->qmutex));		
		#endif
		
		//check writability
		if ((fds[1].revents & POLLOUT) || (fds[1].revents & POLLWRNORM)) {
			fputs("thread zigbee, pollout available\n", stdout);
			
			// check if zigbee client is ready or not
			state = get_peer_state(INFO_NR_ZIGBEE);
			if (state != PEER_STATE_ALIVE) {
				fputs("zigbee client state is unalive\n", stdout);
				goto next_turn;	
			}
			
			// queue is empty or not
			if (is_queue_empty(txq)) {
				fputs("zigbee txq is empty now\n", stdout);
				goto next_turn;
			}
			
			// retrieve data
			fputs("zigbee is dequeuing a packet for transmition\n", stdout);
			pthread_mutex_lock(&(txq->qmutex));
			pkt_generic = (struct pkt_t *)smc_dequeue(txq);
			pthread_mutex_unlock(&(txq->qmutex));
			
			//assemble protocol data structure
			pkt_proto.network_ctx.fd = sock;
			pkt_proto.network_ctx.flags = 0;
			pkt_proto.network_ctx.src_addr = ((struct sockaddr *)&clntAddr);
			pkt_proto.network_ctx.src_addr_len = clntAddrLen;
			pkt_proto.buffer = pkt_generic->u;
			pkt_proto.buffer_len = pkt_generic->length;
			
			if (pkt_generic->u) {
				fprintf(stdout, "sending a packet to zigbee len=%d", pkt_generic->length);
				// begin to transmit data
				ret = smc_zigbee_transmit_protocol(&pkt_proto);
				if (ret < 0) {
					fputs("zigbee is not ready or there is an exception\n", stdout);
					perror("zigbee error");
					//reset the client state to UNALIVE
					set_peer_state(INFO_NR_ZIGBEE, PEER_STATE_UNALIVE);
				}
			} else {
				fprintf(stdout, "can not send a null packet to zigbee len=%d", pkt_generic->length);
			}
			
			free(pkt_generic->u);
			free(pkt_generic);
			pkt_generic = NULL;
			fprintf(stdout, "thread zigbee, pollout done %d\n", ret);
		}
	
		next_turn:
			sleep(1);
			continue;

	} while (gDoExit != true);

out:
	fprintf(stdout, "thread %u is going to shutdown\n",  ((unsigned int)pthread_self()));
	fflush(stdout);
	fflush(stderr);
	smc_deinit_queue_by_index(QUEUE_INDEX_TX_ZIGBEE);
	// free address list which allocated by getaddrinfo()
	freeaddrinfo(servAddr);
	close(sock);
	return ((void *)NULL);
}
