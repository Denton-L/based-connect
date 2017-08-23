#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "based.h"

#define BYTE_MAX 0xFF
#define CN_BASE_PACK_LEN 3

int change_name(int sock, const char *name) {
	char packet[CN_BASE_PACK_LEN + BYTE_MAX] = { 0x01, 0x02, 0x02 };
	size_t length = strlen(name);
	size_t packet_len;
	ssize_t written;

	if (length > BYTE_MAX) {
		length = BYTE_MAX;
		printf("Length of name too long. Truncating to %d characters.\n", BYTE_MAX);
	}

	packet_len = CN_BASE_PACK_LEN + 1 + length;

	packet[CN_BASE_PACK_LEN] = (char) length;
	strncpy(&packet[CN_BASE_PACK_LEN + 1], name, BYTE_MAX);

	written = write(sock, packet, packet_len);

	return written == packet_len ? 0 : written;
}