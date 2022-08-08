#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#define read(sock, buf, size) recv(sock, (void *) buf, (int) size, 0)
#define write(sock, buf, size) send(sock, (char *) buf, (int) size, 0)
#define bswap_16(x) _byteswap_ushort(x)
#else
#include <unistd.h>
#endif

#include "based.h"
#include "bluetooth.h"
#include "malloc.h"

#define ANY 0x00
#define CN_BASE_PACK_LEN 4

int has_noise_cancelling(unsigned int device_id) {
	switch (device_id) {
		case 0x4014:
		case 0x4020:
		case 0x400c:
			return 1;
		default:
			return 0;
	}
}

static int masked_memcmp(const void *ptr1, const void *ptr2, size_t num, const void *mask) {
	const uint8_t *p1 = (const uint8_t *) ptr1;
        const uint8_t *p2 = (const uint8_t *) ptr2;
	const uint8_t *m = (const uint8_t *) mask;	
	while (num-- > 0) {
		uint8_t mask_byte = *m++;
		uint8_t byte1 = *p1++ & mask_byte;
		uint8_t byte2 = *p2++ & mask_byte;

		if (byte1 != byte2) {
			return byte1 - byte2;
		}
	}
	return 0;
}

static int read_check(socktype_t sock, void *receive, size_t receive_n, const void *ack,
		const void *mask) {
	int status = read(sock, receive, receive_n);
	if (status != receive_n) {
		return status ? status : 1;
	}

	return abs(mask
			? masked_memcmp(ack, receive, receive_n, mask)
			: memcmp(ack, receive, receive_n));
}

static int write_check(socktype_t sock, const void *send_b, size_t send_n,
		const void *ack, size_t ack_n) {
	uint8_t *buffer = alloca(ack_n);

	int status = write(sock, send_b, send_n);
	if (status != send_n) {
		return status ? status : 1;
	}
	return read_check(sock, buffer, ack_n, ack, NULL);
}

int send_packet(socktype_t sock, const void *send_b, size_t send_n, uint8_t received[MAX_BT_PACK_LEN]) {
	int status = write(sock, send_b, send_n);
	if (status != send_n) {
		return status ? status : 1;
	}

	return read(sock, received, MAX_BT_PACK_LEN);
}

int init_connection(socktype_t sock) {
	static const uint8_t send_buf[] = { 0x00, 0x01, 0x01, 0x00 };
	static const uint8_t ack[] = { 0x00, 0x01, 0x03, 0x05 };

	int status = write_check(sock, send_buf, sizeof(send_buf), ack, sizeof(ack));
	if (status) {
		return status;
	}

	// Throw away the initial firmware version
	uint8_t garbage[5];
	status = read(sock, garbage, sizeof(garbage));

	if (status != sizeof(garbage)) {
		return status ? status : 1;
	}

	return 0;
}

int get_device_id(socktype_t sock, unsigned int *device_id, unsigned int *index) {
	static const uint8_t send_buf[] = { 0x00, 0x03, 0x01, 0x00 };
	static const uint8_t ack[] = { 0x00, 0x03, 0x03, 0x03 };

	int status = write_check(sock, send_buf, sizeof(send_buf), ack, sizeof(ack));
	if (status) {
		return status;
	}

	uint16_t device_id_halfword;
	status = read(sock, &device_id_halfword, sizeof(device_id_halfword));
	if (status != sizeof(device_id_halfword)) {
		return status ? status : 1;
	}

	*device_id = bswap_16(device_id_halfword);

	uint8_t index_byte;
	status = read(sock, &index_byte, 1);
	if (status != 1) {
		return status ? status : 1;
	}
	*index = index_byte;

	return 0;
}

static int get_name(socktype_t sock, char name[MAX_NAME_LEN + 1]) {
	static const uint8_t ack[] = { 0x01, 0x02, 0x03, ANY, 0x00 };
	static const uint8_t mask[] = { 0xff, 0xff, 0xff, 0x00, 0xff };
	uint8_t buffer[sizeof(ack)];

	int status = read_check(sock, buffer, sizeof(buffer), ack, mask);
	if (status) {
		return status;
	}

	size_t length = buffer[3] - 1;
	status = read(sock, name, length);
	if (status != length) {
		return status ? status : 1;
	}
	name[length] = '\0';

	return 0;
}

int set_name(socktype_t sock, const char *name) {
	static uint8_t send_buf[CN_BASE_PACK_LEN + MAX_NAME_LEN] = { 0x01, 0x02, 0x02, ANY };
	size_t length = strlen(name);

	send_buf[3] = (uint8_t) length;
	strncpy_s((char *) &send_buf[CN_BASE_PACK_LEN], sizeof(send_buf) - CN_BASE_PACK_LEN, name, MAX_NAME_LEN);

	size_t send_size = CN_BASE_PACK_LEN + length;
	int status = write(sock, send_buf, send_size);
	if (status != send_size) {
		return status ? status : 1;
	}

	char got_name[MAX_NAME_LEN + 1];
	status = get_name(sock, got_name);
	if (status) {
		return status;
	}

	return abs(strcmp(name, got_name));
}

static int get_prompt_language(socktype_t sock, enum PromptLanguage *language) {
	// TODO: ensure that this value is correct
	// TODO: figure out what bytes 6 and 7 are for
	static const uint8_t ack[] = { 0x01, 0x03, 0x03, 0x05, ANY, 0x00, ANY, ANY, 0xde };
	static const uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff };
	uint8_t buffer[sizeof(ack)];

	int status = read_check(sock, buffer, sizeof(buffer), ack, mask);
	if (status) {
		return status;
	}

	*language = buffer[4];
	return 0;
}

int set_prompt_language(socktype_t sock, enum PromptLanguage language) {
	static uint8_t send_buf[] = { 0x01, 0x03, 0x02, 0x01, ANY };
	send_buf[4] = language;

	int status = write(sock, send_buf, sizeof(send_buf));
	if (status != sizeof(send_buf)) {
		return status ? status : 1;
	}

	enum PromptLanguage got_language;
	status = get_prompt_language(sock, &got_language);
	if (status) {
		return status;
	}

	return abs(language - got_language);
}

int set_voice_prompts(socktype_t sock, int on) {
	char name[MAX_NAME_LEN + 1];
	enum PromptLanguage pl;
	enum AutoOff ao;
	enum NoiseCancelling nc;

	int status = get_device_status(sock, name, &pl, &ao, &nc);
	if (status) {
		return status;
	}

	if (on) {
		pl |= VP_MASK;
	} else {
		pl &= ~VP_MASK;
	}

	return set_prompt_language(sock, pl);
}

static int get_auto_off(socktype_t sock, enum AutoOff *minutes) {
	static const uint8_t ack[] = { 0x01, 0x04, 0x03, 0x01, ANY };
	static const uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff, 0x00 };
	uint8_t buffer[sizeof(ack)];

	int status = read_check(sock, buffer, sizeof(buffer), ack, mask);
	if (status) {
		return status;
	}

	*minutes = buffer[4];
	return 0;
}

int set_auto_off(socktype_t sock, enum AutoOff minutes) {
	static uint8_t send_buf[] = { 0x01, 0x04, 0x02, 0x01, ANY };
	send_buf[4] = minutes;

	int status = write(sock, send_buf, sizeof(send_buf));
	if (status != sizeof(send_buf)) {
		return status ? status : 1;
	}

	enum AutoOff got_minutes;
	status = get_auto_off(sock, &got_minutes);
	if (status) {
		return status;
	}

	return abs(minutes - got_minutes);
}

static int get_noise_cancelling(socktype_t sock, enum NoiseCancelling *level) {
	static const uint8_t ack[] = { 0x01, 0x06, 0x03, 0x02, ANY, 0x0b };
	static const uint8_t mask[] = { 0xff, 0xff, 0xff, 0xff, 0x00, 0xff };
	uint8_t buffer[sizeof(ack)];

	int status = read_check(sock, buffer, sizeof(buffer), ack, mask);
	if (status) {
		return status;
	}

	*level = buffer[4];
	return 0;
}

int set_noise_cancelling(socktype_t sock, enum NoiseCancelling level) {
	static uint8_t send_buf[] = { 0x01, 0x06, 0x02, 0x01, ANY };
	send_buf[4] = level;

	int status = write(sock, send_buf, sizeof(send_buf));
	if (status != sizeof(send_buf)) {
		return status ? status : 1;
	}

	enum NoiseCancelling got_level;
	status = get_noise_cancelling(sock, &got_level);
	if (status) {
		return status;
	}

	return abs(level - got_level);
}

int get_device_status(socktype_t sock, char name[MAX_NAME_LEN + 1], enum PromptLanguage *language,
		enum AutoOff *minutes, enum NoiseCancelling *level) {
	unsigned int device_id;
	unsigned int index;
	int status = get_device_id(sock, &device_id, &index);
	if (status) {
		return status;
	}

	static const uint8_t send_buf[] = { 0x01, 0x01, 0x05, 0x00 };
	status = write(sock, send_buf, sizeof(send_buf));
	if (status != sizeof(send_buf)) {
		return status ? status : 1;
	}

	static const uint8_t ack1[] = { 0x01, 0x01, 0x07, 0x00 };
	uint8_t buffer1[sizeof(ack1)];

	status = read_check(sock, buffer1, sizeof(buffer1), ack1, NULL);
	if (status) {
		return status;
	}

	status = get_name(sock, name);
	if (status) {
		return status;
	}

	status = get_prompt_language(sock, language);
	if (status) {
		return status;
	}

	status = get_auto_off(sock, minutes);
	if (status) {
		return status;
	}

	if (has_noise_cancelling(device_id)) {
		status = get_noise_cancelling(sock, level);
		if (status) {
			return status;
		}
	} else {
		*level = NC_DNE;
	}

	static const uint8_t ack2[] = { 0x01, 0x01, 0x06, 0x00 };
	uint8_t buffer2[sizeof(ack2)];
	return read_check(sock, buffer2, sizeof(buffer2), ack2, NULL);
}

int set_pairing(socktype_t sock, enum Pairing pairing) {
	static uint8_t send_buf[] = { 0x04, 0x08, 0x05, 0x01, ANY };
	static uint8_t ack[] = { 0x04, 0x08, 0x06, 0x01, ANY };
	send_buf[4] = pairing;
	ack[4] = pairing;
	return write_check(sock, send_buf, sizeof(send_buf), ack, sizeof(ack));
}

int get_firmware_version(socktype_t sock, char version[VER_STR_LEN]) {
	static const uint8_t send_buf[] = { 0x00, 0x05, 0x01, 0x00 };
	static const uint8_t ack[] = { 0x00, 0x05, 0x03, 0x05 };

	int status = write_check(sock, send_buf, sizeof(send_buf), ack, sizeof(ack));
	if (status) {
		return status;
	}

	status = read(sock, version, VER_STR_LEN - 1);
	if (status != VER_STR_LEN - 1) {
		return status ? status : 1;
	}

	version[VER_STR_LEN - 1] = '\0';
	return 0;
}

int get_serial_number(socktype_t sock, char serial[0x100]) {
	static const uint8_t send_buf[] = { 0x00, 0x07, 0x01, 0x00 };
	static const uint8_t ack[] = { 0x00, 0x07, 0x03 };

	int status = write_check(sock, send_buf, sizeof(send_buf), ack, sizeof(ack));
	if (status) {
		return status;
	}

	uint8_t length;
	status = read(sock, &length, 1);
	if (status != 1) {
		return status ? status : 1;
	}

	status = read(sock, serial, length);
	if (status != length) {
		return status ? status : 1;
	}
	serial[length] = '\0';

	return 0;
}

int get_battery_level(socktype_t sock, unsigned int *level) {
	static const uint8_t send_buf[] = { 0x02, 0x02, 0x01, 0x00 };
	static const uint8_t ack[] = { 0x02, 0x02, 0x03, 0x01 };

	int status = write_check(sock, send_buf, sizeof(send_buf), ack, sizeof(ack));
	if (status) {
		return status;
	}

	uint8_t level_byte;
	status = read(sock, &level_byte, 1);
	*level = level_byte;
	return 0;
}

int get_device_info(socktype_t sock, bdaddr_t address, struct Device *device) {
	static uint8_t send_buf[10] = { 0x04, 0x05, 0x01, BT_ADDR_LEN };
	static const uint8_t ack[] = { 0x04, 0x05, 0x03 };

	memcpy(&send_buf[4], &BDADDR_BYTES(address), BT_ADDR_LEN);

	int status = write_check(sock, send_buf, sizeof(send_buf), ack, sizeof(ack));
	if (status) {
		return status;
	}

	uint8_t length;
	status = read(sock, &length, 1);
	if (status != 1) {
		return status ? status : 1;
	}

	status = read(sock, &BDADDR_BYTES(device->address), BT_ADDR_LEN);
	if (status != BT_ADDR_LEN) {
		return status ? status : 1;
	}
	length -= BT_ADDR_LEN;

	status = memcmp(&BDADDR_BYTES(address), &BDADDR_BYTES(device->address), BT_ADDR_LEN);
	if (status) {
		return abs(status);
	}

	uint8_t status_byte;
	status = read(sock, &status_byte, 1);
	if (status != 1) {
		return status ? status : 1;
	}
	length -= 1;

	device->status = status_byte;

	// TODO: figure out what the first byte of garbage is for
	uint8_t garbage[2];
	status = read(sock, &garbage, sizeof(garbage));
	if (status != sizeof(garbage)) {
		return status ? status : 1;
	}
	length -= sizeof(garbage);

	status = read(sock, device->name, length);
	if (status != length) {
		return status ? status : 1;
	}
	device->name[length] = '\0';

	return 0;
}

int get_paired_devices(socktype_t sock, bdaddr_t addresses[MAX_NUM_DEVICES], size_t *num_devices,
		enum DevicesConnected *connected) {
	static const uint8_t send_buf[] = { 0x04, 0x04, 0x01, 0x00 };
	static const uint8_t ack[] = { 0x04, 0x04, 0x03 };

	int status = write_check(sock, send_buf, sizeof(send_buf), ack, sizeof(ack));
	if (status) {
		return status;
	}

	uint8_t num_devices_byte;
	status = read(sock, &num_devices_byte, 1);
	if (status != 1) {
		return status ? status : 1;
	}

	// num_devices_byte = (num_devices_byte - 1) / BT_ADDR_LEN;
	// equivalent statements but more efficient
	num_devices_byte /= BT_ADDR_LEN;

	*num_devices = num_devices_byte;

	uint8_t num_connected_byte;
	status = read(sock, &num_connected_byte, 1);
	if (status != 1) {
		return status ? status : 1;
	}
	*connected = num_connected_byte;

	size_t i;
	for (i = 0; i < num_devices_byte; ++i) {
		status = read(sock, &BDADDR_BYTES(addresses[i]), BT_ADDR_LEN);
		if (status != BT_ADDR_LEN) {
			return status ? status : 1;
		}
	}

	return 0;
}

int connect_device(socktype_t sock, bdaddr_t address) {
	static uint8_t send_buf[11] = { 0x04, 0x01, 0x05, BT_ADDR_LEN + 1, 0x00 };
	static uint8_t ack[10] = { 0x04, 0x01, 0x07, BT_ADDR_LEN };
	memcpy(&send_buf[5], &BDADDR_BYTES(address), BT_ADDR_LEN);
	memcpy(&ack[4], &BDADDR_BYTES(address), BT_ADDR_LEN);
	return write_check(sock, send_buf, sizeof(send_buf), ack, sizeof(ack));
}

int disconnect_device(socktype_t sock, bdaddr_t address) {
	static uint8_t send_buf[10] = { 0x04, 0x02, 0x05, BT_ADDR_LEN };
	static uint8_t ack[10] = { 0x04, 0x02, 0x07, BT_ADDR_LEN };
	memcpy(&send_buf[4], &BDADDR_BYTES(address), BT_ADDR_LEN);
	memcpy(&ack[4], &BDADDR_BYTES(address), BT_ADDR_LEN);
	return write_check(sock, send_buf, sizeof(send_buf), ack, sizeof(ack));
}

int remove_device(socktype_t sock, bdaddr_t address) {
	static uint8_t send_buf[10] = { 0x04, 0x03, 0x05, BT_ADDR_LEN };
	static uint8_t ack[10] = { 0x04, 0x03, 0x06, BT_ADDR_LEN };
	memcpy(&send_buf[4], &BDADDR_BYTES(address), BT_ADDR_LEN);
	memcpy(&ack[4], &BDADDR_BYTES(address), BT_ADDR_LEN);
	return write_check(sock, send_buf, sizeof(send_buf), ack, sizeof(ack));
}
