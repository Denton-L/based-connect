#ifndef BASED_H
#define BASED_H

#include <bluetooth/bluetooth.h>
#include <stddef.h>

#include "bluetooth.h"

#define BOSE_CHANNEL 8
#define MAX_NAME_LEN 0x1f
#define MAX_NUM_DEVICES 8
#define MAX_BT_PACK_LEN 0x1000
#define VER_STR_LEN 6
#define VP_MASK 0x7F

enum NoiseCancelling {
	NC_HIGH = 0x01,
	NC_LOW = 0x03,
	NC_OFF = 0x00,
	NC_DNE = 0xff
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
	PL_EN = 0x21,
	PL_FR = 0x22,
	PL_IT = 0x23,
	PL_DE = 0x24,
	PL_ES = 0x26,
	PL_PT = 0x27,
	PL_ZH = 0x28,
	PL_KO = 0x29,
	PL_PL = 0x2B,
	PL_RU = 0x2A,
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

enum SelfVoice {
	SV_OFF = 0x0,
	SV_HIGH = 0x1,
	SV_MEDIUM = 0x2,
	SV_LOW = 0x3,
};

struct Device {
	bdaddr_t address;
	enum DeviceStatus status;
	char name[MAX_NAME_LEN + 1];
};

int has_noise_cancelling(unsigned int device_id);
int init_connection(int sock);
int send_packet(int sock, const void *send, size_t send_n, uint8_t recieved[MAX_BT_PACK_LEN]);
int get_device_id(int sock, unsigned int *device_id, unsigned int *index);
int set_name(int sock, const char *name);
int set_prompt_language(int sock, enum PromptLanguage language);
int set_voice_prompts(int sock, int on);
int set_auto_off(int sock, enum AutoOff minutes);
int set_noise_cancelling(int sock, enum NoiseCancelling level);
int get_device_status(int sock, char name[MAX_NAME_LEN + 1], enum PromptLanguage *language,
		enum AutoOff *minutes, enum NoiseCancelling *level);
int set_pairing(int sock, enum Pairing pairing);
int set_self_voice(int sock, enum SelfVoice selfVoice);
int get_firmware_version(int sock, char version[VER_STR_LEN]);
int get_serial_number(int sock, char serial[0x100]);
int get_battery_level(int sock, unsigned int *level);
int get_device_info(int sock, bdaddr_t address, struct Device *device);
int get_paired_devices(int sock, bdaddr_t addresses[MAX_NUM_DEVICES], size_t *num_devices,
		enum DevicesConnected *connected);
int connect_device(int sock, bdaddr_t address);
int disconnect_device(int sock, bdaddr_t address);
int remove_device(int sock, bdaddr_t address);

#endif
