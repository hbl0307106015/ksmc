#include "log.h"
#include "knxCommon.h"
#include "knxProtocol.h"
#include "smcProtocol.h"
#include "threadSocket.h"

#define MAX_NUM_FILES_DES 2
#define POLL_TIME_OUT_MS 500



void* handle_socket(void *arg)
{
    struct thread_socket_arg *socket_arg = (struct thread_socket_arg *)arg;
    
    /*
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
    */
    
    // tell the system what kind(s) of address info we want
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC; // any address family
    addrCriteria.ai_socktype = SOCK_DGRAM; // only datagram sockets
    addrCriteria.ai_protocol = IPPROTO_UDP; // only udp protocol
    
    // get address(es)
    struct addrinfo *servAddr = NULL; // the result of below calls
    int ret = getaddrinfo(socket_arg->server, socket_arg->port, 
		&addrCriteria, &servAddr);
	if (ret != 0) {
		perror("socket thread, getaddrinfo()");
		goto out;
	}
	
	// create a datagram/UDP socket
	int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
		servAddr->ai_protocol);
	if (sock < 0) {
		perror("socket thread, socket()");
		goto out;
	}
	
	fputs("create thread socket successfully\n", stdout);
	
	// send the string to the server
	char echoString[6] = {'h','e','l','l','o','\0'};
	ssize_t eStrLen = strlen(echoString);
	ssize_t numBytes = 0;
	
	struct sockaddr_storage fromAddr;
	socklen_t fromAddrLen = sizeof(fromAddr); // source address of server
	char buffer[MAX_STRING_LENGTH + 1] = {0}; // I/O buffer

	#if 0
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0) {
		perror("fcntl fet flags error");
		goto out;
	}

	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
		perror("fcntl fet flags error");
		goto out;
	}
	#endif
	
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
	
	/* put a discovery request packet to the rxq queue
	 * the connection for knx and smc is begin with a discovery 
	 * request from knx.
	*/
	struct pkt_t *discovery_req = NULL;
	discovery_resp = (struct pkt_t *)malloc(sizeof(struct pkt_t));
	// assemble a discovery packet
	knx_protocol_assemble_discovery_req(&discovery_req);
	knx_protocol_store_packet(rxq, &discovery_req); //put the packet to rxq queue
	
	struct pkt_t *pkt = NULL;
	struct pollfd fds[MAX_NUM_FILES_DES];
	fds[0].fd = sock; // add socket file descriptor
	fds[0].events = POLLIN|POLLRDNORM;// |POLLOUT|POLLWRNORM; // register pollin & pollout event
	fds[1].fd = sock; // add socket file descriptor
	fds[1].events = POLLOUT|POLLWRNORM; // register pollin & pollout event

    do {
		
		if (poll(fds, MAX_NUM_FILES_DES, POLL_TIME_OUT_MS) <= 0) {
			perror("no file descriptor is available");
			sleep(1);
			continue;
		}
		
		// check writable availability
		if ((fds[1].revents && POLLOUT)||(fds[1].revents && POLLWRNORM)) {
			fputs("pollout available\n", stdout);
			
			/* retrieve a packet from rx queue if exist */
			pthread_mutex_lock(&(rxq->qmutex));
			pkt = (struct pkt_t *)knx_protocol_retrieve_packet(rxq);
			pthread_mutex_unlock(&(rxq->qmutex));
			
			if (pkt) {
				fprintf(stdout, "dequeuing a packet, len=%d\n", (int)pkt->length);
				dump_buffer(pkt->u, pkt->length);
				/* send packet to smart controller */
				numBytes = sendto(sock, pkt->u, pkt->length, 0, \
					servAddr->ai_addr, servAddr->ai_addrlen);
				/*
				numBytes = sendto(sock, echoString, eStrLen, 0, \
					servAddr->ai_addr, servAddr->ai_addrlen);
				*/
				
				if (numBytes < 0) {
					perror("socket thread, numBytes < 0");
					//pthread_exit(NULL);
				} else if ((size_t)numBytes != pkt->length) {
					perror("socket thread, numBytes != eStrLen");
					//pthread_exit(NULL);
				} else if(pkt->type & PACKET_RESPONSE_NEEDED) {
					fputs("init pkt dada for timer\n", stdout);
					// copy pkt structure to user data
					struct pkt_t *udata = knx_protocol_alloc_pkt(sizeof(uint16_t));
					memcpy(udata, pkt, sizeof(pkt));
					memcpy(udata->u, pkt->u, pkt->length);
					struct knx_timer *discovery_timer = knx_timer_alloc(sizeof(struct knx_timer));
					// cb_func
					discovery_timer->valid = true;
					discovery_timer->expire = 3; //3 second for timeout
					discovery_timer->user_data = udata;
					discovery_timer->cb_func = timeout_cb_func_discovery_req;
					
					// add timer for it
					fputs("add pkt dada to timer\n", stdout);
					timer_heap_add(discovery_timer);
				}
				
				free(pkt->u);
				free(pkt);
			} else {
				fputs("no packet in queue\n", stdout);
			}
			sleep(1);
			fputs("pollout done\n", stdout);
		}

		//fputs("it has been sent a message, waiting for response\n", stdout);

		// whether received a response
		if ((fds[0].revents && POLLIN) || (fds[0].revents && POLLRDNORM)) {
			fputs("pollin available\n", stdout);
			/* receive a packet from socket */
			numBytes = recvfrom(sock, buffer, RC1180_MAX_BUFFER_SIZE, 0, \
				(struct sockaddr *)&fromAddr, &fromAddrLen);
			if (numBytes < 0) {
				perror("socket thread, numBytes recv < 0");
				//pthread_exit(NULL);
			}
			
			if (!is_sock_addr_equal(servAddr->ai_addr, (struct sockaddr *)&fromAddr))
				perror("socket thread, addr is not equal with the previous server");
			
			/* handle the packet accroding to the packet type */
			//handle_smc_packet((unsigned char *)buffer, numBytes);
			smc_knx_handle_protocol();
			
			/*
			numBytes = recvfrom(sock, buffer, MAX_STRING_LENGTH, 0, \
				(struct sockaddr *)&fromAddr, &fromAddrLen);
			if (numBytes < 0) {
				perror("socket thread, numBytes recv < 0");
				//pthread_exit(NULL);
			} else if (numBytes != eStrLen) {
				perror("socket thread, numBytes recv != sStrlen");
				//pthread_exit(NULL);
			}
			// verfication 
			if (!is_sock_addr_equal(servAddr->ai_addr, (struct sockaddr *)&fromAddr))
				perror("socket thread, addr is not equal with the previous server");
		
			buffer[eStrLen] = '\0'; // Null terminate received data
			fprintf(stdout, "received:%s.\n", buffer); // Print the data buffer
			*/
			fputs("pollin done\n", stdout);
			sleep(1);
		}
    } while (gFlagExit != true); /* while (running) */
	
out:
	fprintf(stdout, "thread %u is going to shutdown\n",  ((unsigned int)pthread_self()));
	fflush(stdout);
	fflush(stderr);
	freeaddrinfo(servAddr);
	close(sock);
    return ((void *)NULL);
}
