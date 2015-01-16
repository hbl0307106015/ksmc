#ifndef __LOG_H__
#define __LOG_H__

/*
 * dump_buffer
 * @b	: buffer
 * @len	: length of buffer
 * */
static void dump_buffer(unsigned char *b, size_t len);

/*
 * dump_buffer_appl_data
 * @b	: buffer
 * @len	: length of buffer
 * @flag: format of the dump function, hex or string ?
 * */
static void dump_buffer_appl_data(unsigned char *b, size_t len, uint8_t flag);

#endif /* __LOG_H__ */
