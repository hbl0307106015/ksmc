#ifndef __PRATICAL_H__
#define __PRATICAL_H__

#include "smcCommon.h"

void DieWithUserMessage(const char *msg, const char *detail);
void DieWithSystemMessage(const char *msg);

void PrintSocketAddress(const struct sockaddr *addr, FILE *s);

#endif /* __PRATICAL_H__ */
