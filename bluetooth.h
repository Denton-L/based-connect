#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#ifdef _WIN32
#define bdaddr_t BLUETOOTH_ADDRESS_STRUCT
#define BDADDR_BYTES(x) ((x).rgBytes)
#include <ws2bth.h>
#include <Bthsdpdef.h>
#include <BluetoothAPIs.h>
#else
#include <bluetooth/bluetooth.h>
#define BDADDR_BYTES(x) ((x).b)
#endif

#define BT_ADDR_LEN 6

int reverse_ba2str(const bdaddr_t *ba, char *str, size_t str_len);
int reverse_str2ba(const char *str, bdaddr_t *ba);
#ifdef _WIN32
int bachk(const char *str);
int str2ba(const char *str, bdaddr_t *ba);
void baswap(bdaddr_t *dst, const bdaddr_t *src);
#endif

#endif
