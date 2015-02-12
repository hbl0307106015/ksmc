#ifndef __LOG_H__
#define __LOG_H__

#include "smcCommon.h"

void dump_buffer(unsigned char *b, size_t len);
void DieWithUserMessage(const char *msg, const char *detail);
void DieWithSystemMessage(const char *msg);

#endif /* __LOG_H__ */
