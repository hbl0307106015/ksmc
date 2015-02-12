#include "log.h"
#include "knxCommon.h"

void dump_buffer(unsigned char *b, size_t len)
{
	if (len <=0)
		return;

	uint16_t i;
	
	fprintf(stdout, "buffer: ");
	for (i = 0; i < len; i++)
		fprintf(stdout, "%02x ", b[i]);
	fprintf(stdout, "\n");
}

void dump_buffer_appl_data(unsigned char *b, size_t len, uint8_t flag)
{
	if (len < LENGTH_BEFORE_APPL_DATA)
		return;
		
	fprintf(stdout, "application data: ");
	
	if (flag == DUMP_FORMAT_HEX) {
		dump_buffer(b, len);
		return;
	}
	
	uint16_t actual_len = get_user_data_length(len);

	char *p = (char *)malloc(actual_len + 1);
	
	if (!p) {
		perror("out of memory");
		exit(1);
	}
	
	memcpy(p, b + LENGTH_BEFORE_APPL_DATA, actual_len);
	p[actual_len] = '\0';
	fprintf(stdout, "%s\n", p);
	
	free(p);
}
