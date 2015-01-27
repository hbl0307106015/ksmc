#include "common.h"
#include "circularQueue.h"

int main(void)
{
	/* init */
	struct circular_queue* queue = circular_queue_init();
	
	/* enqueue & dequeue
	* empty test
	* full test
	* dequeue
	* */
	struct data *user_data[MAX_QUEUE_SIZE] = {NULL};
	int i;
	for (i = 0; i < MAX_QUEUE_SIZE + 1; i++) {
		fprintf(stdout, "turn %d\t", i);
		user_data[i] = malloc(sizeof(struct data));
		if (!user_data[i]) {
			fprintf(stdout, "%s %d, memory out\n", __func__, __LINE__);
			exit(0);
		}
		
		fprintf(stdout, "queue tag %d, front=%d, rear=%d, size=%d\n",\
			(int)(queue->tag), queue->front, queue->rear, get_queue_size(queue));
		user_data[i]->val = (39 + i);
		enqueue(queue, user_data[i]);
		circular_queue_show(queue);
	}
	fprintf(stdout, "enqueue done, queue tag %d\n\n", (int)(queue->tag));
	
	struct data *p = NULL;
	for (i = 0; i < MAX_QUEUE_SIZE + 1; i++) {

		circular_queue_show(queue);
		
		p = dequeue(queue);
		if (!p) {
			fprintf(stdout, "%s %d, dequeue failed\n", __func__, __LINE__);
			continue;
		}
		fprintf(stdout, "turn %d, queue tag %d, val %d, front=%d, rear=%d size=%d\n",\
			i, queue->tag, p->val, queue->front, queue->rear, get_queue_size(queue));
		free(p);
		p = NULL;
	}
	fprintf(stdout, "dequeue done, queue tag %d\n", (int)(queue->tag));
	
	// free memory check
	for (i = 0; i < MAX_QUEUE_SIZE; i++)
		if (user_data[i]) {
			fprintf(stdout, "user_data[%d]->val=%d\n",\
				i, user_data[i]->val);
	}
	
	return 0;
}
