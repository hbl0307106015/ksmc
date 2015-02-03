#include "pratical.h"
#include "threadAPP.h"
#include "smcNetwork.h"
#include "smcCommon.h"
#include "smcProtocol.h"

struct network_ctx app_network_manager[MAX_APP_NUM];


void* smc_thread_app(void *arg)
{
	struct thread_app_arg *app_arg = (struct thread_app_arg *)arg;

	struct sockaddr_storage clntAddr; //client address
	socklen_t clntAddrLen = sizeof(clntAddr); // length of client address structure, (in-out parameter)
	memset(&clntAddr, 0, clntAddrLen);
	unsigned char buffer[BUFFER_SIZE64] = {0}; // I/O buffer
	ssize_t numBytesRcvd = 0;
	ssize_t numBytesSent = 0;
	
	size_t cnt = 0;
	struct pollfd fds[MAX_NUM_POLL_FILES_DES];
	fds[0].fd = app_arg->sock; // add socket file descriptor
	fds[0].events = POLLIN|POLLRDNORM;// register pollin event
	fds[1].fd = app_arg->sock; // add socket file descriptor
	fds[1].events = POLLOUT|POLLWRNORM; // register pollout event

	struct protocol_data pdata;
	unsigned char buffer_test[BUFFER_SIZE64] = { \
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
			numBytesRcvd = recvfrom(app_arg->sock, buffer, BUFFER_SIZE64, \
				0, (struct sockaddr *)&clntAddr, &clntAddrLen);
			if (numBytesRcvd < 0)
				DieWithSystemMessage("recvfrom() failed");
			
			//assemble protocol ctx
			pdata.fd = app_arg->sock;
			pdata.src_addr = (struct sockaddr *)&clntAddr;
			pdata.src_addr_len = clntAddrLen;
			pdata.buffer = buffer;
			pdata.buffer_len = numBytesRcvd;
			//handle protocol
			smc_app_handle_protocol(&pdata);

			fputs("pollin done\n", stdout);
		}
		
		// check writable availability
		if ((fds[1].revents && POLLOUT)||(fds[1].revents && POLLWRNORM)) {
			fputs("pollout available\n", stdout);
			
			numBytesSent = sendto(app_arg->sock, buffer_test, 21, \
				0, (struct sockaddr *)&clntAddr, sizeof(clntAddr));

			if (numBytesSent < 0)
				DieWithSystemMessage("recvfrom() failed");
				
			fputs("pollout done\n", stdout);
		}

		next_turn:
			sleep(1);
	} while(gDoExit != true);

	return (void *)NULL;
}
