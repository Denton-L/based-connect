#ifdef _WIN32
#include <winsock2.h>
#include <stdio.h>
#include <stdint.h>
#else
#include <bluetooth/bluetooth.h>
#endif

#include <stdlib.h>

#include "bluetooth.h"

/*
 * Based on code taken from the BlueZ library.
 */

int reverse_ba2str(const bdaddr_t *ba, char *str) {
	return sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
			BDADDR_BYTES(*ba)[0], BDADDR_BYTES(*ba)[1], BDADDR_BYTES(*ba)[2], BDADDR_BYTES(*ba)[3], BDADDR_BYTES(*ba)[4], BDADDR_BYTES(*ba)[5]);
}

int reverse_str2ba(const char *str, bdaddr_t *ba) {
	int i;

	if (bachk(str) < 0) {
		memset(ba, 0, sizeof(*ba));
		return -1;
	}

	for (i = 0; i < 6; i++, str += 3)
		BDADDR_BYTES(*ba)[i] = strtol(str, NULL, 16);

	return 0;
}

#ifdef _WIN32
int bachk(const char *str)
{
	char tmp[18], *ptr = tmp;
	if (!str)
		return -1;
	if (strlen(str) != 17)
		return -1;
	memcpy(tmp, str, 18);
	while (*ptr) {
		*ptr = toupper(*ptr);
		if (*ptr < '0'|| (*ptr > '9' && *ptr < 'A') || *ptr > 'F')
			return -1;
		ptr++;
		*ptr = toupper(*ptr);
		if (*ptr < '0'|| (*ptr > '9' && *ptr < 'A') || *ptr > 'F')
			return -1;
		ptr++;
		*ptr = toupper(*ptr);
		if (*ptr == 0)
			break;
		if (*ptr != ':')
			return -1;
		ptr++;
	}
	return 0;
}

int str2ba(const char *str, bdaddr_t *ba)
{
	uint8_t b[6];
	const char *ptr = str;
	int i;
	for (i = 0; i < 6; i++) {
		b[i] = (uint8_t) strtol(ptr, NULL, 16);
		if (i != 5 && !(ptr = strchr(ptr, ':')))
			ptr = ":00:00:00:00:00";
		ptr++;
	}
	baswap(ba, (bdaddr_t *) b);
	return 0;
}

void baswap(bdaddr_t *dst, const bdaddr_t *src)
{
	register unsigned char *d = (unsigned char *) dst;
	register const unsigned char *s = (const unsigned char *) src;
	register int i;
	for (i = 0; i < 6; i++)
		d[i] = s[5-i];
}
#endif
