#include <bluetooth/bluetooth.h>
#include <stdlib.h>

#include "bluetooth.h"

/*
 * Based on code taken from the BlueZ library.
 */

int reverse_ba2str(const bdaddr_t *ba, char *str) {
	return sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
			ba->b[0], ba->b[1], ba->b[2], ba->b[3], ba->b[4], ba->b[5]);
}

int reverse_str2ba(const char *str, bdaddr_t *ba) {
	int i;

	if (bachk(str) < 0) {
		memset(ba, 0, sizeof(*ba));
		return -1;
	}

	for (i = 0; i < 6; i++, str += 3)
		ba->b[i] = strtol(str, NULL, 16);

	return 0;
}
