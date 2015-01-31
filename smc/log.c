#include "smcCommon.h"
#include "log.h"

void dump_buffer(unsigned char *b, size_t len)
{
	if (len <=0)
		return;

	uint16_t i;
	
	fprintf(stdout, "dump buffer: ");
	for (i = 0; i < len; i++)
		fprintf(stdout, "%02x ", b[i]);
	fputc('\n', stdout);
}

