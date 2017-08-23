#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "based.h"

#define BYTE_MAX 0xfe
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

static int write_check(int sock, const void *send, size_t send_n,
		const void *expected, size_t expected_n) {
	uint8_t buffer[expected_n];
	int status;

	if ((status = write_get(sock, send, send_n, buffer, sizeof(buffer))) < 0) {
		return status;
	}

	return abs(memcmp(expected, buffer, sizeof(buffer)));
}

int set_name(int sock, const char *name) {
	uint8_t send[CN_BASE_PACK_LEN + BYTE_MAX] = { 0x01, 0x02, 0x02, 0x00 };
	uint8_t expected[CN_BASE_CONF_LEN + BYTE_MAX] = { 0x01, 0x02, 0x03, 0x00, 0x00 };
	size_t length = strlen(name);

	if (length > BYTE_MAX) {
		length = BYTE_MAX;
		fprintf(stderr, "Length of name too long. Truncating to %d characters.\n", BYTE_MAX);
	}

	send[3] = length;
	strncpy((char *) &send[CN_BASE_PACK_LEN], name, BYTE_MAX);

	expected[3] = length + 1;
	strncpy((char *) &expected[CN_BASE_CONF_LEN], name, BYTE_MAX);

	return write_check(sock, send, CN_BASE_PACK_LEN + length, expected, CN_BASE_PACK_LEN + length);
}

int noise_cancelling(int sock, enum NoiseCancelling level) {
	uint8_t send[] = { 0x01, 0x06, 0x02, 0x01, level };
	uint8_t expected[] = { 0x01, 0x06, 0x03, 0x02, level, 0x0b };
	return write_check(sock, send, sizeof(send), expected, sizeof(expected));
}

int auto_off(int sock, enum AutoOff minutes) {
	uint8_t send[] = { 0x01, 0x04, 0x02, 0x01, minutes };
	uint8_t expected[] = { 0x01, 0x04, 0x03, 0x01, minutes };
	return write_check(sock, send, sizeof(send), expected, sizeof(expected));
}

int prompt_language(int sock, enum PromptLanguage language) {
	uint8_t send[] = { 0x01, 0x03, 0x02, 0x01, language };
	// TODO: ensure that this value is correct
	uint8_t expected[] = { 0x01, 0x03, 0x03, 0x05, language, 0x00, 0x04, 0xc3, 0xde };
	return write_check(sock, send, sizeof(send), expected, sizeof(expected));
}
