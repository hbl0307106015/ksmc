#ifndef COMMON_H
#define COMMON_H

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>

#include"mytimer.h"

#define BUFFER_SIZE 64

struct client_data {
	int sockfd;
	char buf[BUFFER_SIZE];
	struct sockaddr_in address;
	struct mytimer *ptr_timer;
};



#endif
