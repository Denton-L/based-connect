#ifndef BASED_H
#define BASED_H

#include <unistd.h>

#define BT_CHANNEL 8

ssize_t change_name(int sock, const char *name);

#endif
