#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "based.h"

#define BYTE_MAX 0xFF
#define CN_BASE_PACK_LEN 3
#define NC_PACK_LEN 5

ssize_t set_name(int sock, const char *name) {
	char packet[CN_BASE_PACK_LEN + BYTE_MAX] = { 0x01, 0x02, 0x02 };
	size_t length = strlen(name);
	size_t packet_len;

	if (length > BYTE_MAX) {
		length = BYTE_MAX;
		printf("Length of name too long. Truncating to %d characters.\n", BYTE_MAX);
	}

	packet_len = CN_BASE_PACK_LEN + 1 + length;

	packet[CN_BASE_PACK_LEN] = (char) length;
	strncpy(&packet[CN_BASE_PACK_LEN + 1], name, BYTE_MAX);

	return write(sock, packet, packet_len);
}

ssize_t noise_cancelling(int sock, char level) {
	char packet[NC_PACK_LEN] = { 0x01, 0x06, 0x02, 0x01, level };
	return write(sock, packet, NC_PACK_LEN);
}
