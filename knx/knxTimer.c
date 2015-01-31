#include"knxTimer.h"

static void knx_timer_init(struct knx_timer *t, int delay);

static void knx_timer_init(struct knx_timer *t, int delay)
{
	if (t) {
		t->expire = (time(NULL) + (time_t)delay);
		fprintf(stdout, "init user_data expire=%d\n", (int)t->expire);
	}

	char str[BUFFER_SIZE64] = {0};
	sprintf(str, "I delay %d", delay);
	
	t->user_data = (void *)malloc(sizeof(struct knx_timer_data));
	if (t->user_data) {
		strncpy(((struct knx_timer_data *)(t->user_data))->buf, \
			str, strlen(str));
		fprintf(stdout, "init user_data done\n");
	}
	
	t->valid = true;
}
