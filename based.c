#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "based.h"

#define BYTE_MAX 0xFE
#define CN_BASE_PACK_LEN 4
#define CN_BASE_CONF_LEN 5
#define NC_PACK_LEN 5
#define VP_PACK_LEN 5
#define AO_PACK_LEN 5
#define PL_PACK_LEN 5

ssize_t set_name(int sock, const char *name) {
	char packet[CN_BASE_PACK_LEN + BYTE_MAX] = { 0x01, 0x02, 0x02, 0x00 };
	char conf_packet[CN_BASE_CONF_LEN + BYTE_MAX] = { 0x01, 0x02, 0x03, 0x00, 0x00 };
	char conf_buffer[CN_BASE_CONF_LEN + BYTE_MAX];
	size_t length = strlen(name);
	int status;

	if (length > BYTE_MAX) {
		length = BYTE_MAX;
		fprintf(stderr, "Length of name too long. Truncating to %d characters.\n", BYTE_MAX);
	}

	packet[3] = length;
	strncpy(&packet[CN_BASE_PACK_LEN], name, BYTE_MAX);

	conf_packet[3] = length + 1;
	strncpy(&conf_packet[CN_BASE_CONF_LEN], name, BYTE_MAX);

	status = write(sock, packet, CN_BASE_PACK_LEN + length);
	if (status < 0) {
		return status;
	}

	status = read(sock, conf_buffer, CN_BASE_CONF_LEN + length);
	if (status < 0) {
		return status;
	}

	return memcmp(conf_packet, conf_buffer, CN_BASE_PACK_LEN + length);
}

ssize_t noise_cancelling(int sock, enum NoiseCancelling level) {
	char packet[NC_PACK_LEN] = { 0x01, 0x06, 0x02, 0x01, level };
	return write(sock, packet, NC_PACK_LEN);
}

ssize_t voice_prompts(int sock, enum VoicePrompt setting) {
	char packet[VP_PACK_LEN] = { 0x01, 0x03, 0x02, 0x01, setting };
	return write(sock, packet, VP_PACK_LEN);
}

ssize_t auto_off(int sock, enum AutoOff minutes) {
	char packet[AO_PACK_LEN] = { 0x01, 0x04, 0x02, 0x01, minutes };
	return write(sock, packet, AO_PACK_LEN);
}

ssize_t prompt_language(int sock, enum PromptLanguage language) {
	char packet[AO_PACK_LEN] = { 0x01, 0x03, 0x02, 0x01, language };
	return write(sock, packet, PL_PACK_LEN);
}
