#include "log.h"
#include "threadAPP.h"
#include "smcCommon.h"
#include "smcQueue.h"
#include "smcProtocol.h"
#include "smcPeerUtility.h"

void* smc_thread_app(void *arg)
{
    // tell the system what kind(s) of address info we want
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC; // any address family
    addrCriteria.ai_flags = AI_PASSIVE; // accept on any address/port
    addrCriteria.ai_socktype = SOCK_DGRAM; // only datagram sockets
    addrCriteria.ai_protocol = IPPROTO_UDP; // only udp protocol

    // get address(es)
    int sock = 0;
    struct addrinfo *servAddr = NULL; // the result of below calls
    int ret = getaddrinfo(NULL, PORT_NUM_APP, &addrCriteria, &servAddr);
	if (ret != 0) {
		DieWithUserMessage("thread app, getaddrinfo() failed", gai_strerror(ret));
		goto out;
	}
	
	#if 0
	char htname[BUFFER_SIZE64] = {0};
	struct hostent *ent = NULL;
	gethostname(htname, BUFFER_SIZE64);
	fprintf(stdout, "hostname:%s\n", htname);
	ent = gethostbyname(htname);
	char *c = NULL;
	int i = 0, len = 0;
	for (i = 0; ent->h_addr_list[i]; i++) {
		c = inet_ntoa(*((struct in_addr *)(ent->h_addr_list[i])));
		len = strlen(c); 
		fprintf(stdout, "%s %d\t",  c, len);
	}
	fputc('\n', stdout);
	#endif
	
	// create a datagram/UDP socket
	sock = socket(servAddr->ai_family, servAddr->ai_socktype, \
		servAddr->ai_protocol);
	if (sock < 0) {
		DieWithSystemMessage("thread app, socket() failed");
		goto out;
	}

	#if 0
	#define IFACENAME "br-lan"
	struct ifreq ifr;
	size_t slen = strlen(IFACENAME);
	strncpy(ifr.ifr_name, IFACENAME, slen);
	ifr.ifr_name[slen] = '\0';
	fprintf(stdout, "interface str=%s strlen=%d\n", ifr.ifr_name, slen);
	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
		perror("ioctl, get if addr error");
		goto out;
	}
	
	struct sockaddr_in gen_addr;
	memcpy(&gen_addr, &(ifr.ifr_addr), sizeof(gen_addr));
	gIPstr = inet_ntoa(gen_addr.sin_addr);
	//inet_ntop(servAddr->ai_family, (void *)
	slen = strlen(gIPstr);
	fprintf(stdout, "%s-%d\n",  gIPstr, slen);
	#endif
	
	//smc_get_iface_ipaddr(NULL, NULL);
	
	// bind to the local address
	if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0) {
		DieWithSystemMessage("thread app, bind() failed");
		goto out;
	}

	// poll fds init
	struct pollfd fds[MAX_NUM_POLL_FILES_DES];
	fds[0].fd = sock; // add socket file descriptor
	fds[0].events = POLLIN | POLLRDNORM;// register pollin event
	fds[1].fd = sock; // add socket file descriptor
	fds[1].events = POLLOUT | POLLWRNORM; // register pollout event

	// queue for app
	struct circular_queue *rxq = NULL;
	rxq = smc_get_queue(QUEUE_INDEX_RX_GENERIC);
	if (!rxq) {
		DieWithUserMessage("smc_get_queue() failed", "null pointer");
		goto out;
	}

	ssize_t numBytesRcvd = 0, numBytesSent = 0;
	uint8_t state, buffer[BUFFER_SIZE64] = {0}; // I/O buffer

	struct pkt_t *pkt = NULL;
	struct protocol_data pdata;
	unsigned char buffer_test[BUFFER_SIZE64] = { \
		0x14, 0x0,\
		0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, \
		'h', 'e', 'l', 'l', 'o', ',', 'w', 'o', 'r', 'l', 'd','!'};

	// client address
	struct sockaddr_storage clntAddr; //client address
	socklen_t clntAddrLen = sizeof(clntAddr); // length of client address structure, (in-out parameter)
	memset(&clntAddr, 0, clntAddrLen);
	
	fputs("thread app enter mainloop\n", stdout);
	do {
		// check if fds are ready ?
		if (poll(fds, MAX_NUM_POLL_FILES_DES, POLL_TIME_OUT_MS) <= 0) {
			perror("no file descriptor is available");
			goto next_turn;
		}

		// check readable availability
		if ((fds[0].revents & POLLIN) || (fds[0].revents & POLLRDNORM)) {
			fputs("thread app pollin available\n", stdout);

			//read socket
			numBytesRcvd = recvfrom(sock, buffer, BUFFER_SIZE64, \
				0, (struct sockaddr *)&clntAddr, &clntAddrLen);
			if (numBytesRcvd < 0)
				DieWithSystemMessage("recvfrom() failed");
			
			PrintSocketAddress((struct sockaddr *)&clntAddr, stdout);
			fputc('\n', stdout);
			//assemble protocol ctx
			pdata.network_ctx.fd = sock;
			pdata.network_ctx.src_addr = (struct sockaddr *)&clntAddr;
			pdata.network_ctx.src_addr_len = clntAddrLen;
			pdata.buffer = buffer;
			pdata.buffer_len = numBytesRcvd;
			//handle protocol
			smc_app_handle_protocol(&pdata);

			fputs("thread app pollin done\n", stdout);
		}

		/* for testing requirement, to imitate a 
		 * command for app.
		 * */
		#if 0
		uint16_t cmd = 0x1000;
		static uint8_t val = 0;
		val = ((++val) % 1000);
		uint8_t buffer_test[16] = {0};
		cmd = htons(cmd);
		memcpy(buffer_test, &cmd, sizeof(uint16_t));
		memcpy(buffer_test + 2, &val, sizeof(uint8_t));
		
		struct pkt_t *tpkt = smc_pkt_alloc(sizeof(buffer_test));
		smc_pkt_fill(tpkt, buffer_test, sizeof(buffer_test));

		pthread_mutex_lock(&(rxq->qmutex));
		smc_enqueue(rxq, (void *)tpkt);
		pthread_mutex_unlock(&(rxq->qmutex));
		#endif

		// check writable availability
		if ((fds[1].revents & POLLOUT) || (fds[1].revents & POLLWRNORM)) {
			fputs("thread app  pollout available\n", stdout);

			if ((state = get_peer_state(INFO_NR_APP)) != PEER_STATE_ALIVE) {
				fputs("APP is not ready or unalive\n", stdout);
				goto next_turn;
			}
			
			if (is_queue_empty(rxq)) {
				fputs("tx queue for APP is empty now\n", stdout);
				goto next_turn;
			}

			pthread_mutex_lock(&(rxq->qmutex));
			pkt = smc_dequeue(rxq);
			pthread_mutex_unlock(&(rxq->qmutex));
			
			numBytesSent = sendto(sock, pkt->u, pkt->length, \
				0, (struct sockaddr *)&clntAddr, sizeof(clntAddr));


			if (numBytesSent < 0) {
				DieWithSystemMessage("recvfrom() failed");
				// set state of the APP peer
				set_peer_state(INFO_NR_APP, PEER_STATE_UNALIVE);
				memset(&clntAddr, 0, sizeof(clntAddr));
			}
		
			free(pkt->u);
			free(pkt);
			pkt = NULL;
			fprintf(stdout, "thread app pollout done ret=%d\n", (int)numBytesSent);
		}

		next_turn:
			sleep(1);
			continue;

	} while (gDoExit != true);

out:
	fprintf(stdout, "thread %u is going to shutdown\n",  ((unsigned int)pthread_self()));
	fflush(stdout);
	fflush(stderr);
	smc_deinit_queue_by_index(QUEUE_INDEX_RX_GENERIC);
	// free address list which allocated by getaddrinfo()
	freeaddrinfo(servAddr);
	close(sock);
	return ((void *)NULL);
}
