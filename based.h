#ifndef BASED_H
#define BASED_H

#include <unistd.h>

#define BT_CHANNEL 8

#define NC_HIGH 0x01
#define NC_LOW 0x03
#define NC_OFF 0x00

ssize_t set_name(int sock, const char *name);
ssize_t noise_cancelling(int sock, char level);

#endif
