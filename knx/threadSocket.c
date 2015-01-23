#include "knxCommon.h"
#include "knxProtocol.h"
#include "log.h"
#include "threadSocket.h"

#define MAX_NUM_FILES_DES 2
#define POLL_TIME_OUT_MS 500

static inline bool is_sock_addr_equal()
{
	return true;
}

void* handle_socket(void *arg)
{
    struct thread_socket_arg *socket_arg = (struct thread_socket_arg *)arg;
    
    #if 0
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
    #endif
    
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
	
	struct pollfd fds[MAX_NUM_FILES_DES];
	fds[0].fd = sock; // add socket file descriptor
	fds[0].events = POLLIN|POLLRDNORM;//|POLLOUT|POLLWRNORM; //register pollin & pollout event
	fds[1].fd = sock; // add socket file descriptor
	fds[1].events = POLLOUT|POLLWRNORM; //register pollin & pollout event

    do {
		
		if (poll(fds, MAX_NUM_FILES_DES, POLL_TIME_OUT_MS) <= 0) {
			perror("no data available");
			sleep(1);
			continue;
		}
		
		if ((fds[1].revents && POLLOUT)||(fds[1].revents && POLLWRNORM)) {
			fputs("pollout available\n", stdout);
			numBytes = sendto(sock, echoString, eStrLen, 0, \
				servAddr->ai_addr, servAddr->ai_addrlen);

			if (numBytes < 0) {
				perror("socket thread, numBytes < 0");
				//pthread_exit(NULL);
			}
			else if (numBytes != eStrLen) {
				perror("socket thread, numBytes != eStrLen");
				//pthread_exit(NULL);
			}
			sleep(1);
			fputs("pollout done\n", stdout);
		}

		fputs("it has been sent a message, waiting for response\n", stdout);

		// received a response
		if ((fds[0].revents && POLLIN) || (fds[0].revents && POLLRDNORM)) {
			fputs("pollin available\n", stdout);
			numBytes = recvfrom(sock, buffer, MAX_STRING_LENGTH, 0, \
				(struct sockaddr *)&fromAddr, &fromAddrLen);
			if (numBytes < 0) {
				perror("socket thread, numBytes recv < 0");
				//pthread_exit(NULL);
			}
			else if (numBytes != eStrLen) {
				perror("socket thread, numBytes recv != sStrlen");
				//pthread_exit(NULL);
			}
				// verfication 
			if (!is_sock_addr_equal(servAddr->ai_addr, (struct sockaddr *)&fromAddr))
				perror("socket thread, addr is not equal with the previous server");
		
			buffer[eStrLen] = '\0'; // Null terminate received data
			fprintf(stdout, "received:%s.\n", buffer); // Print the data buffer
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
