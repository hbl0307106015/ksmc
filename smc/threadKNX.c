#include "log.h"
#include "threadKNX.h"
#include "pratical.h"
#include "smcCommon.h"
#include "smcNetwork.h"
#include "smcProtocol.h"

void* smc_thread_knx(void *arg)
{
	struct thread_knx_arg *knx_arg = (struct thread_knx_arg *)arg;
	struct client_real_time_info *real_time_info = knx_arg->real_time_info;

	struct sockaddr_storage clntAddr; //client address
	socklen_t clntAddrLen = sizeof(clntAddr); // length of client address structure, (in-out parameter)
	unsigned char buffer[RC1180_MAX_BUFFER_SIZE] = {0}; // I/O buffer
	ssize_t numBytesRcvd = 0;
	ssize_t numBytesSent = 0;
	
	size_t cnt = 0;
	struct pollfd fds[MAX_NUM_POLL_FILES_DES];
	fds[0].fd = knx_arg->sock; // add socket file descriptor
	fds[0].events = POLLIN|POLLRDNORM;// |POLLOUT|POLLWRNORM; // register pollin & pollout event
	fds[1].fd = knx_arg->sock; // add socket file descriptor
	fds[1].events = POLLOUT|POLLWRNORM; // register pollin & pollout event
	
	struct protocol_data pdata;
	struct client_real_time_info *rt_info = NULL;
	unsigned char buffer_w[RC1180_MAX_BUFFER_SIZE] = {\
		0x14, 0x0,\
		0x0, 0x1, 0x0, 0x2, 0x61, 0x0, 0x0, \
		'h', 'e', 'l', 'l', 'o', ',', 'w', 'o', 'r', 'l', 'd','!'};

	do {
		
		if (poll(fds, MAX_NUM_POLL_FILES_DES, POLL_TIME_OUT_MS) <= 0) {
			perror("no file descriptor is available");
			goto next_turn;
		}

		// check readable availability
		if ((fds[0].revents && POLLIN)||(fds[0].revents && POLLRDNORM)) {
			fputs("pollin available\n", stdout);
			
			//read socket
			numBytesRcvd = recvfrom(knx_arg->sock, buffer, RC1180_MAX_BUFFER_SIZE, \
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
			//smc_knx_parse(buffer, numBytesRcvd);
			pdata.fd = knx_arg->sock;
			pdata.src_addr = (struct sockaddr *)&clntAddr;
			pdata.src_addr_len = clntAddrLen;
			pdata.buffer = buffer;
			pdata.buffer_len = numBytesRcvd;
			smc_knx_handle_protocol(&pdata);

			fputs("pollin done\n", stdout);
		}
		
		// check writable availability
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

next_turn:
		sleep(1);

	} while (gDoExit != true);
	
	return (void *)NULL;
}
