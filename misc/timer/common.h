#ifndef COMMON_H
#define COMMON_H

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>

#define BUFFER_SIZE 64

struct client_data {
	sockaddr_in address;
	int sockfd;
	char buf[BUFFER_SIZE];
	struct mytimer *my_timer;
};



#endif
