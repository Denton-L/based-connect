#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "based.h"

#define CN_BASE_PACK_LEN 4
#define CN_BASE_CONF_LEN 5

static int write_get(int sock, const void *send, size_t send_n, void *recv, size_t recv_n) {
	int status;

	if ((status = write(sock, send, send_n)) < 0) {
		return status;
	}

	if ((status = read(sock, recv, recv_n)) < 0) {
		return status;
	}

	return 0;
}

static int masked_memcmp(const void *ptr1, const void *ptr2, size_t num, const void *mask) {
	for (; num-- ; ++ptr1, ++ptr2, ++mask ) {
		uint8_t m = *(uint8_t *) mask;
		uint8_t b1 = *(uint8_t *) ptr1 & m;
		uint8_t b2 = *(uint8_t *) ptr2 & m;

		if (b1 != b2) {
			return b1 - b2;
		}
	}
	return 0;
}

static int read_check(int sock, const void *send, size_t send_n,
		void *buffer, size_t buffer_n, const void *expected, const void *check_mask) {
	int status;

	if ((status = write_get(sock, send, send_n, buffer, buffer_n)) < 0) {
		return status;
	}

	return abs(check_mask == NULL
			? memcmp(expected, buffer, buffer_n)
			: masked_memcmp(expected, buffer, buffer_n, check_mask));
}

static int write_check(int sock, const void *send, size_t send_n,
		const void *expected, size_t expected_n, const void *check_mask) {
	uint8_t buffer[expected_n];

	return read_check(sock, send, send_n, buffer, sizeof(buffer), expected, check_mask);
}

int init_connection(int sock) {
	uint8_t send[] = { 0x00, 0x01, 0x01, 0x00 };
	uint8_t expected[] = { 0x00, 0x01, 0x03, 0x05, 0x00, '.', 0x00, '.', 0x00 };
	uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00 };

	return write_check(sock, send, sizeof(send), expected, sizeof(expected), mask);
}

int set_name(int sock, const char *name) {
	uint8_t send[CN_BASE_PACK_LEN + MAX_NAME_LEN] = { 0x01, 0x02, 0x02, 0x00 };
	uint8_t expected[CN_BASE_CONF_LEN + MAX_NAME_LEN] = { 0x01, 0x02, 0x03, 0x00, 0x00 };
	size_t length = strlen(name);

	send[3] = length;
	strncpy((char *) &send[CN_BASE_PACK_LEN], name, MAX_NAME_LEN);

	expected[3] = length + 1;
	strncpy((char *) &expected[CN_BASE_CONF_LEN], name, MAX_NAME_LEN);

	return write_check(sock, send, CN_BASE_PACK_LEN + length,
			expected, CN_BASE_CONF_LEN + length, NULL);
}

int set_noise_cancelling(int sock, enum NoiseCancelling level) {
	uint8_t send[] = { 0x01, 0x06, 0x02, 0x01, level };
	uint8_t expected[] = { 0x01, 0x06, 0x03, 0x02, level, 0x0b };
	return write_check(sock, send, sizeof(send), expected, sizeof(expected), NULL);
}

int set_auto_off(int sock, enum AutoOff minutes) {
	uint8_t send[] = { 0x01, 0x04, 0x02, 0x01, minutes };
	uint8_t expected[] = { 0x01, 0x04, 0x03, 0x01, minutes };
	return write_check(sock, send, sizeof(send), expected, sizeof(expected), NULL);
}

int set_prompt_language(int sock, enum PromptLanguage language) {
	uint8_t send[] = { 0x01, 0x03, 0x02, 0x01, language };
	// TODO: ensure that this value is correct
	uint8_t expected[] = { 0x01, 0x03, 0x03, 0x05, language, 0x00, 0x04, 0xc3, 0xde };
	return write_check(sock, send, sizeof(send), expected, sizeof(expected), NULL);
}

int get_firmware_version(int sock, char version[6]) {
	uint8_t send[] = { 0x00, 0x05, 0x01, 0x00 };
	uint8_t expected[] = { 0x00, 0x05, 0x03, 0x05, 0x00, '.', 0x00, '.', 0x00 };
	uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00 };
	uint8_t buffer[sizeof(expected)];

	int status = read_check(sock, send, sizeof(send), buffer, sizeof(buffer), expected, mask);
	if (status != 0) {
		return status;
	}

	memcpy(version, &buffer[4], 5);
	version[5] = '\0';
	return 0;
}

int get_battery_level(int sock, unsigned int *level) {
	uint8_t send[] = { 0x02, 0x02, 0x01, 0x00 };
	uint8_t expected[] = { 0x02, 0x02, 0x03, 0x01, 0x00 };
	uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff, 0x00 };
	uint8_t buffer[sizeof(expected)];

	int status = read_check(sock, send, sizeof(send), buffer, sizeof(buffer), expected, mask);
	if (status != 0) {
		return status;
	}

	*level = buffer[4];
	return 0;
}
