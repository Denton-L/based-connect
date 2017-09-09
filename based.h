#ifndef BASED_H
#define BASED_H

#include <bluetooth/bluetooth.h>
#include <stddef.h>

#include "bluetooth.h"

#define BOSE_CHANNEL 8
#define MAX_NAME_LEN 0x1f
#define MAX_NUM_DEVICES 8

enum NoiseCancelling {
	NC_HIGH = 0x01,
	NC_LOW = 0x03,
	NC_OFF = 0x00
};

enum AutoOff {
	AO_NEVER = 0,
	AO_5_MIN = 5,
	AO_20_MIN = 20,
	AO_40_MIN = 40,
	AO_60_MIN = 60,
	AO_180_MIN = 180
};

enum PromptLanguage {
	PL_OFF = 0x01,
	PL_EN = 0x21,
	PL_FR = 0x22,
	PL_IT = 0x23,
	PL_DE = 0x24,
	PL_ES = 0x26,
	PL_PT = 0x27,
	PL_ZH = 0x28,
	PL_KO = 0x29,
	PL_NL = 0x2e,
	PL_JA = 0x2f,
	PL_SV = 0x32
};

enum Pairing {
	P_ON = 0x01,
	P_OFF = 0x00
};

enum DeviceStatus {
	DS_THIS = 0x03,
	DS_CONNECTED = 0x01,
	DS_DISCONNECTED = 0x00
};

enum DevicesConnected {
	DC_ONE = 0x01,
	DC_TWO = 0x03
};

struct Device {
	bdaddr_t address;
	enum DeviceStatus status;
	char name[MAX_NAME_LEN + 1];
};

int init_connection(int sock);
int set_name(int sock, const char *name);
int set_noise_cancelling(int sock, enum NoiseCancelling level);
int set_auto_off(int sock, enum AutoOff minutes);
int set_prompt_language(int sock, enum PromptLanguage language);
int set_pairing(int sock, enum Pairing pairing);
int get_firmware_version(int sock, char version[6]);
int get_serial_number(int sock, char serial[0x100]);
int get_battery_level(int sock, unsigned int *level);
int get_devices(int sock, bdaddr_t addresses[MAX_NUM_DEVICES], size_t *num_devices,
		enum DevicesConnected *connected);
int get_device_info(int sock, bdaddr_t address, struct Device *device);
int connect_device(int sock, bdaddr_t address);
int disconnect_device(int sock, bdaddr_t address);
int remove_device(int sock, bdaddr_t address);

#endif
