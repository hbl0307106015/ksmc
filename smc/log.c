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

void DieWithUserMessage(const char *msg, const char *detail)
{
	fputs(msg, stderr);
	fputs(": ", stderr);
	fputs(detail, stderr);
	fputc('\n', stderr);
	//exit(1);
}

void DieWithSystemMessage(const char *msg)
{
	perror(msg);
	//exit(1);
}

