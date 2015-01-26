#include"mytimer.h"

void mytimer_init(struct mytimer *t, int delay)
{
	if (t) {
		t->expire = (time(NULL) + (time_t)delay);
		fprintf(stdout, "init user_data expire=%d\n", (int)t->expire);
	}
	
	
	char str[BUFFER_SIZE] = {0};
	sprintf(str, "I delay %d", delay);
	
	t->user_data = (struct client_data *)malloc(sizeof(struct client_data));
	
	if (t->user_data) {
		strncpy(t->user_data->buf, str, strlen(str));
		fprintf(stdout, "init user_data done\n");
	}
}
