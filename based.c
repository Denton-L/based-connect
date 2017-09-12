#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "based.h"
#include "bluetooth.h"

#define ANY 0x00
#define CN_BASE_PACK_LEN 4
#define CN_BASE_CONF_LEN 5

static int masked_memcmp(const void *ptr1, const void *ptr2, size_t num, const void *mask) {
	while (num-- > 0) {
		uint8_t mask_byte = *(uint8_t *) mask++;
		uint8_t byte1 = *(uint8_t *) ptr1++ & mask_byte;
		uint8_t byte2 = *(uint8_t *) ptr2++ & mask_byte;

		if (byte1 != byte2) {
			return byte1 - byte2;
		}
	}
	return 0;
}

static int read_check(int sock, void *recieve, size_t recieve_n, const void *expected,
		const void *mask) {
	int status = read(sock, recieve, recieve_n);
	if (status != recieve_n) {
		return status ? status : 1;
	}

	return abs(mask
			? masked_memcmp(expected, recieve, recieve_n, mask)
			: memcmp(expected, recieve, recieve_n));
}

static int write_check(int sock, const void *send, size_t send_n,
		const void *expected, size_t expected_n) {
	uint8_t buffer[expected_n];

	int status = write(sock, send, send_n);
	if (status) {
		return status ? status : 1;
	}
	return read_check(sock, buffer, sizeof(buffer), expected, NULL);
}

int send_packet(int sock, const void *send, size_t send_n, uint8_t recieved[MAX_BT_PACK_LEN]) {
	int status;
	if ((status = write(sock, send, send_n)) < 0) {
		return status;
	}

	return read(sock, recieved, MAX_BT_PACK_LEN);
}

int init_connection(int sock) {
	static const uint8_t send[] = { 0x00, 0x01, 0x01, 0x00 };
	static const uint8_t expected[] = { 0x00, 0x01, 0x03, 0x05 };

	int status = write_check(sock, send, sizeof(send), expected, sizeof(expected));
	if (status < 0) {
		return status;
	}

	// Throw away the initial firmware version
	uint8_t garbage[5];
	status = read(sock, garbage, sizeof(garbage));

	if (status < 0) {
		return status;
	}

	return 0;
}

int set_name(int sock, const char *name) {
	static uint8_t send[CN_BASE_PACK_LEN + MAX_NAME_LEN] = { 0x01, 0x02, 0x02, ANY };
	static uint8_t expected[CN_BASE_CONF_LEN + MAX_NAME_LEN] = { 0x01, 0x02, 0x03, ANY, 0x00 };
	size_t length = strlen(name);

	send[3] = length;
	strncpy((char *) &send[CN_BASE_PACK_LEN], name, MAX_NAME_LEN);

	expected[3] = length + 1;
	strncpy((char *) &expected[CN_BASE_CONF_LEN], name, MAX_NAME_LEN);

	return write_check(sock, send, CN_BASE_PACK_LEN + length, expected, CN_BASE_CONF_LEN + length);
}

int set_noise_cancelling(int sock, enum NoiseCancelling level) {
	static uint8_t send[] = { 0x01, 0x06, 0x02, 0x01, ANY };
	static uint8_t expected[] = { 0x01, 0x06, 0x03, 0x02, ANY, 0x0b };
	send[4] = level;
	expected[4] = level;
	return write_check(sock, send, sizeof(send), expected, sizeof(expected));
}

int set_auto_off(int sock, enum AutoOff minutes) {
	static uint8_t send[] = { 0x01, 0x04, 0x02, 0x01, ANY };
	static uint8_t expected[] = { 0x01, 0x04, 0x03, 0x01, ANY };
	send[4] = minutes;
	expected[4] = minutes;
	return write_check(sock, send, sizeof(send), expected, sizeof(expected));
}

int set_prompt_language(int sock, enum PromptLanguage language) {
	static uint8_t send[] = { 0x01, 0x03, 0x02, 0x01, ANY };
	// TODO: ensure that this value is correct
	static uint8_t expected[] = { 0x01, 0x03, 0x03, 0x05, ANY, 0x00, 0x04, 0xc3, 0xde };
	send[4] = language;
	expected[4] = language;
	return write_check(sock, send, sizeof(send), expected, sizeof(expected));
}

int set_pairing(int sock, enum Pairing pairing) {
	static uint8_t send[] = { 0x04, 0x08, 0x05, 0x01, ANY };
	static uint8_t expected[] = { 0x04, 0x08, 0x06, 0x01, ANY };
	send[4] = pairing;
	expected[4] = pairing;
	return write_check(sock, send, sizeof(send), expected, sizeof(expected));
}

int get_firmware_version(int sock, char version[6]) {
	static const uint8_t send[] = { 0x00, 0x05, 0x01, 0x00 };
	static const uint8_t expected[] = { 0x00, 0x05, 0x03, 0x05 };

	int status = write_check(sock, send, sizeof(send), expected, sizeof(expected));
	if (status) {
		return status;
	}

	status = read(sock, version, 5);
	if (status < 0) {
		return status;
	}

	version[5] = '\0';
	return 0;
}

int get_serial_number(int sock, char serial[0x100]) {
	static const uint8_t send[] = { 0x00, 0x07, 0x01, 0x00 };
	static const uint8_t expected[] = { 0x00, 0x07, 0x03 };

	int status = write_check(sock, send, sizeof(send), expected, sizeof(expected));
	if (status) {
		return status;
	}

	uint8_t length;

	status = read(sock, &length, 1);
	if (status < 0) {
		return status;
	}

	status = read(sock, serial, length);
	if (status < 0) {
		return status;
	}
	serial[length] = '\0';

	return 0;
}

int get_battery_level(int sock, unsigned int *level) {
	static const uint8_t send[] = { 0x02, 0x02, 0x01, 0x00 };
	static const uint8_t expected[] = { 0x02, 0x02, 0x03, 0x01 };

	int status = write_check(sock, send, sizeof(send), expected, sizeof(expected));
	if (status) {
		return status;
	}

	uint8_t level_byte;
	status = read(sock, &level_byte, 1);
	*level = level_byte;
	return 0;
}

int get_paired_devices(int sock, bdaddr_t addresses[MAX_NUM_DEVICES], size_t *num_devices,
		enum DevicesConnected *connected) {
	static const uint8_t send[] = { 0x04, 0x04, 0x01, 0x00 };
	static const uint8_t expected[] = { 0x04, 0x04, 0x03 };

	int status = write_check(sock, send, sizeof(send), expected, sizeof(expected));
	if (status) {
		return status;
	}

	uint8_t num_devices_byte;
	status = read(sock, &num_devices_byte, 1);
	if (status < 0) {
		return status;
	}

	// num_devices_byte = (num_devices_byte - 1) / BT_ADDR_LEN;
	// equivalent statements but more efficient
	num_devices_byte /= BT_ADDR_LEN;

	*num_devices = num_devices_byte;

	uint8_t num_connected_byte;
	status = read(sock, &num_connected_byte, 1);
	if (status < 0) {
		return status;
	}
	*connected = num_connected_byte;

	size_t i;
	for (i = 0; i < num_devices_byte; ++i) {
		status = read(sock, &addresses[i].b, BT_ADDR_LEN);
		if (status < 0) {
			return status;
		}
	}

	return 0;
}

int get_device_info(int sock, bdaddr_t address, struct Device *device) {
	static uint8_t send[10] = { 0x04, 0x05, 0x01, BT_ADDR_LEN };
	static const uint8_t expected[] = { 0x04, 0x05, 0x03 };

	memcpy(&send[4], &address.b, BT_ADDR_LEN);

	int status = write_check(sock, send, sizeof(send), expected, sizeof(expected));
	if (status) {
		return status;
	}

	uint8_t length;
	status = read(sock, &length, 1);
	if (status < 0) {
		return status;
	}

	status = read(sock, &device->address.b, BT_ADDR_LEN);
	if (status < 0) {
		return status;
	}
	length -= BT_ADDR_LEN;

	status = memcmp(&address.b, &device->address.b, BT_ADDR_LEN);
	if (status) {
		return abs(status);
	}

	uint8_t status_byte;
	status = read(sock, &status_byte, 1);
	if (status < 0) {
		return status;
	}
	length -= 1;

	device->status = status_byte;

	// TODO: figure out what the first byte of garbage is for
	uint8_t garbage[2];
	status = read(sock, &garbage, sizeof(garbage));
	if (status < 0) {
		return status;
	}
	length -= sizeof(garbage);

	status = read(sock, device->name, length);
	if (status < 0) {
		return status;
	}
	device->name[length] = '\0';

	return 0;
}

int connect_device(int sock, bdaddr_t address) {
	static uint8_t send[11] = { 0x04, 0x01, 0x05, BT_ADDR_LEN + 1, 0x00 };
	static uint8_t expected[10] = { 0x04, 0x01, 0x07, BT_ADDR_LEN };
	memcpy(&send[5], &address.b, BT_ADDR_LEN);
	memcpy(&expected[4], &address.b, BT_ADDR_LEN);
	return write_check(sock, send, sizeof(send), expected, sizeof(expected));
}

int disconnect_device(int sock, bdaddr_t address) {
	static uint8_t send[10] = { 0x04, 0x02, 0x05, BT_ADDR_LEN };
	static uint8_t expected[10] = { 0x04, 0x02, 0x07, BT_ADDR_LEN };
	memcpy(&send[4], &address.b, BT_ADDR_LEN);
	memcpy(&expected[4], &address.b, BT_ADDR_LEN);
	return write_check(sock, send, sizeof(send), expected, sizeof(expected));
}

int remove_device(int sock, bdaddr_t address) {
	static uint8_t send[10] = { 0x04, 0x03, 0x05, BT_ADDR_LEN };
	static uint8_t expected[10] = { 0x04, 0x03, 0x06, BT_ADDR_LEN };
	memcpy(&send[4], &address.b, BT_ADDR_LEN);
	memcpy(&expected[4], &address.b, BT_ADDR_LEN);
	return write_check(sock, send, sizeof(send), expected, sizeof(expected));
}

int get_device_id(int sock, unsigned int *device_id, unsigned int *index) {
	static const uint8_t send[] = { 0x00, 0x03, 0x01, 0x00 };
	static const uint8_t expected[] = { 0x00, 0x03, 0x03, 0x03 };

	int status = write_check(sock, send, sizeof(send), expected, sizeof(expected));
	if (status) {
		return status;
	}

	uint16_t device_id_halfword;
	status = read(sock, &device_id_halfword, 2);
	if (status < 0) {
		return status;
	}
	// reverse endianness
	device_id_halfword = (device_id_halfword >> 8) | (device_id_halfword << 8);
	*device_id = device_id_halfword;

	uint8_t index_byte;
	status = read(sock, &index_byte, 1);
	if (status < 0) {
		return status;
	}
	*index = index_byte;

	return 0;
}
