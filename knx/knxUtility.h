#ifndef __KNX_UTILITY_H__
#define __KNX_UTILITY_H__

#include "knxCommon.h"

bool is_valid_length(ssize_t ret, ssize_t len_byte);

bool is_valid_knx_buffer(ssize_t ret);

ssize_t knxRead(int fd, char *b, size_t l);

ssize_t knxReadLengthByte(int fd, char *b);

void knxHandleUARTBuffer(int fd);

#endif /* __KNX_UTILITY_H__ */
