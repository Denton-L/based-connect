#include <bluetooth/bluetooth.h>

#include "bluetooth.h"

/*
 * Based on code taken from the BlueZ library.
 */

int reverse_ba2str(const bdaddr_t *ba, char *str) {
	return sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
			ba->b[0], ba->b[1], ba->b[2], ba->b[3], ba->b[4], ba->b[5]);
}
