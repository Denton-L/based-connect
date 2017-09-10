#include <stdint.h>

static uint8_t get_value(char c) {
	if ('0' <= c && c <= '9') {
		return c - '0';
	} else if ('A' <= c && c <= 'F') {
		return c - 'A' + 10;
	} else if ('a' <= c && c <= 'f') {
		return c - 'a' + 10;
	} else {
		return 16;
	}
}

int strtobyte(const char *str, uint8_t *byte) {
	uint8_t total = 0;
	uint8_t part;

	part = get_value(str[0]);
	if (part > 15) {
		return 1;
	}
	total += 0x10 * part;

	part = get_value(str[1]);
	if (part > 15) {
		return 1;
	}
	total += part;

	*byte = total;
	return 0;
}
