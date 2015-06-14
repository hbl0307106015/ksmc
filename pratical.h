#ifndef __PRATICAL_H__
#define __PRATICAL_H__

#include "utility.h"

void
DieWithUserMessage(const char *msg, const char *detail);

void
DieWithSystemMessage(const char *msg);

void
PrintSocketAddress(const struct sockaddr *addr, FILE *s);

int
SetupTCPClientSocket(const char *host, const char *service);

#endif /* __PRATICAL_H__ */
