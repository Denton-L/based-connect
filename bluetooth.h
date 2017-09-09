#ifndef UTIL_H
#define UTIL_H

#include <bluetooth/bluetooth.h>

#define BT_ADDR_LEN 6

int reverse_ba2str(const bdaddr_t *ba, char *str);

#endif
