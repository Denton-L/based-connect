#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <bluetooth/bluetooth.h>

#define BT_ADDR_LEN 6

int reverse_ba2str(const bdaddr_t *ba, char *str);
int reverse_str2ba(const char *str, bdaddr_t *ba);

#endif
