#include "log.h"
#include "threadKNX.h"
#include "smcQueue.h"
#include "smcCommon.h"
#include "smcNetwork.h"
#include "smcProtocol.h"

void* smc_thread_knx(void *arg)
{
	//struct thread_knx_arg *knx_arg = (struct thread_knx_arg *)arg;
	//struct peer_real_time_info *knx_client_info = knx_arg->knx_client_info;

	// tell the system what kind(s) of address we want
	struct addrinfo addrCriteria;
	memset(&addrCriteria, 0, sizeof(addrCriteria));
	addrCriteria.ai_family = AF_UNSPEC; // any address family
	addrCriteria.ai_flags = AI_PASSIVE; // accept on any address / port
	addrCriteria.ai_socktype = SOCK_DGRAM; // only datagram socket
	addrCriteria.ai_protocol = IPPROTO_UDP; // only UDP socket

	// get address(es)
	int sock = 0;
	struct addrinfo *servAddr = NULL;
	int ret = getaddrinfo(NULL, PORT_NUM_KNX, &addrCriteria, &servAddr);
	if (ret != 0) {
		DieWithUserMessage("thread knx, getaddrinfo() failed", gai_strerror(ret));
		goto out;
	}

	// create socket file descriptor
	sock = socket(servAddr->ai_family, servAddr->ai_socktype,\
		servAddr->ai_protocol);
	if (sock < 0) {
		DieWithSystemMessage("thread knx, socket() failed");
		goto out;
	}

	if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0) {
		DieWithSystemMessage("thread knx, bind() failed");
		goto out;
	}
	
	struct sockaddr_storage clntAddr; //client address
	socklen_t clntAddrLen = sizeof(clntAddr); // length of client address structure, (in-out parameter)
	unsigned char buffer[RC1180_MAX_BUFFER_SIZE] = {0}; // I/O buffer
	ssize_t numBytesRcvd = 0;
	ssize_t numBytesSent = 0;
	
	size_t cnt = 0;
	struct pollfd fds[MAX_NUM_POLL_FILES_DES];
	fds[0].fd = sock; // add socket file descriptor
	fds[0].events = POLLIN|POLLRDNORM;// register pollin event
	fds[1].fd = sock; // add socket file descriptor
	fds[1].events = POLLOUT|POLLWRNORM; // register pollout event
	
	struct protocol_data pdata;
	struct client_real_time_info *rt_info = NULL;
	unsigned char buffer_w[RC1180_MAX_BUFFER_SIZE] = {\
		0x14, 0x0,\
		0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, \
		'h', 'e', 'l', 'l', 'o', ',', 'w', 'o', 'r', 'l', 'd','!'};

	fputs("thread knx enter mainloop\n", stdout);
	do {
		// check if fds are ready ?
		if (poll(fds, MAX_NUM_POLL_FILES_DES, POLL_TIME_OUT_MS) <= 0) {
			perror("no file descriptor is available");
			goto next_turn;
		}

		// check readable availability
		if ((fds[0].revents & POLLIN)||(fds[0].revents & POLLRDNORM)) {
			fputs("pollin available\n", stdout);
			
			//read socket
			numBytesRcvd = recvfrom(sock, buffer, RC1180_MAX_BUFFER_SIZE, \
				0, (struct sockaddr *)&clntAddr, &clntAddrLen);
			if (numBytesRcvd < 0)
				DieWithSystemMessage("recvfrom() failed");
			
			//print sockaddr info
			fputs("Handling client ", stdout);
			PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
			fputc('\n', stdout);
			
			//dump received datagram back to the stdout
			fprintf(stdout, "%d ", (int)(++cnt));
			dump_buffer((unsigned char *)buffer, (size_t)numBytesRcvd);
			
			//parse buffer
			pdata.network_ctx.fd = sock;
			pdata.network_ctx.src_addr = (struct sockaddr *)&clntAddr;
			pdata.network_ctx.src_addr_len = clntAddrLen;
			pdata.buffer = buffer;
			pdata.buffer_len = numBytesRcvd;
			smc_knx_handle_protocol(&pdata);

			fputs("pollin done\n", stdout);
		}
		
		// check writable availability
		#if 0
		if ((fds[1].revents && POLLOUT)||(fds[1].revents && POLLWRNORM)) {
			
			if (real_time_info[INFO_NR_KNX].state != CLIENT_STATE_ALIVE) {
				fprintf(stdout, "knx client program is not alive\n");
				goto next_turn;
			}
			
			fputs("pollout available\n", stdout);
			//retrieve message from txqueue
			//comming soon, not implemented yet...
			
			//send to the knx client program
			rt_info = get_real_time_info(INFO_NR_KNX);
			//just for test
			rt_info->client_info.buffer = buffer_w;
			rt_info->client_info.buffer_len = 21;
			numBytesSent = smc_send_protocol(&(rt_info->client_info));
			
			if (numBytesSent < 0) {
				DieWithSystemMessage("send protocol message failed");
				set_real_time_info_state(INFO_NR_KNX, CLIENT_STATE_UNALIVE);
			}
			
			#if 0
			numBytesSent = sendto(knx_arg->sock, buffer_w, 21, \
				0, (struct sockaddr *)&clntAddr, sizeof(clntAddr));
			#endif
			if (numBytesSent < 0)
				DieWithSystemMessage("sendto() failed");
			else if (numBytesSent != 21)
				DieWithUserMessage("sento()", "sent unexpected number of bytes");
			fputs("pollout done\n", stdout);
		}
		#endif

		next_turn:
			sleep(1);
			continue;

	} while (gDoExit != true);

out:
	fprintf(stdout, "thread %u is going to shutdown\n",  ((unsigned int)pthread_self()));
	fflush(stdout);
	fflush(stderr);
	smc_deinit_queue_by_index(QUEUE_INDEX_TX_KNX);
	// free address list which allocated by getaddrinfo()
	freeaddrinfo(servAddr);
	close(sock);
	return ((void *)NULL);
}
