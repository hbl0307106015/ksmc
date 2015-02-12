#ifndef __LOG_H__
#define __LOG_H__

#include "knxCommon.h"
#include "knxProtocol.h"

enum {
	DUMP_FORMAT_STRING = 0,
	DUMP_FORMAT_HEX = 1,
};

/*
 * dump_buffer
 * @b	: buffer
 * @len	: length of buffer
 * */
void dump_buffer(unsigned char *b, size_t len);

/*
 * dump_buffer_appl_data
 * @b	: buffer
 * @len	: length of buffer
 * @flag: format of the dump function, hex or string ?
 * */
void dump_buffer_appl_data(unsigned char *b, size_t len, uint8_t flag);

#endif /* __LOG_H__ */
