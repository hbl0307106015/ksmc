#include "main.h"
#include "utility.h"
#include "pratical.h"

// thread used to handle the packet from server
void* handler_thread(void *arg);

// thread used to receive packets and put them into the evbuffer
void* receiver_thread(void *arg);

// thread used to maintain connection between router and server
void* maintainer_thread(void *arg);

// mutex
pthread_mutex_t gSockMutex = PTHREAD_MUTEX_INITIALIZER;

// evbuffer uses producer-customer model to manage the availability
#define BUFFER_SIZE_BYTES 1024
#define PACKET_SIZE_NUM 16
pthread_mutex_t gEvbufferMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t gEmptySem, gFullSem;

int main(int argc, char *argv[])
{
	main_loop(argc, argv);
	return 0;
}

void main_loop(int argc, char *argv[])
{
	// initialize network
	if (argc < 3 || argc > 4)
		DieWithUserMessage("parameter(s)",
			"<server address/name> <echo work> [<server port/service>]");
	
	char *server = argv[1]; // first arg: server address/name
	char *echoString = argv[2]; //
	
	//third arg (optional) : server port/service
	char *service = (argc == 4) ? (argv[3]) : ("echo");
	
	//create a connected TCP socket
	int sock = SetupTCPClientSocket(server, service);
	if (sock < 0)
		DieWithUserMessage("SetupTCPClientSocket() failed",
			"unable to connect");
	
	// init semaphore
	sem_init(&gEmptySem, 0, PACKET_SIZE_NUM);
	sem_init(&gFullSem, 0, 0);
	
	// thread start at here
	// including begin to connect to server and wait comming packet
	pthread_t pid[3];
	pthread_create(&pid[1], NULL, receiver_thread, (void *)&sock);
	pthread_create(&pid[0], NULL, handler_thread, (void *)&sock);
	pthread_create(&pid[2], NULL, maintainer_thread, (void *)&sock);
	
	// not reach
}
