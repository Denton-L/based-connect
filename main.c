#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "based.h"
#include "bluetooth.h"
#include "util.h"

static const char *program_name;

static void usage() {
	printf("Usage: %s [options] <address>\n"
		"\t-h, --help\n"
		"\t\tPrint the help message.\n"
		"\t-n <name>, --name=<name>\n"
		"\t\tChange the name of the device.\n"
		"\t-c <level>, --noise-cancelling=<level>\n"
		"\t\tChange the noise cancelling level.\n"
		"\t\tlevel: high, low, off\n"
		"\t-o <minutes>, --auto-off=<minutes>\n"
		"\t\tChange the auto-off time.\n"
		"\t\tminutes: never, 5, 20, 40, 60, 180\n"
		"\t-l <language>, --prompt-language=<language>\n"
		"\t\tChange the voice-prompt language.\n"
		"\t\tlanguage: en, fr, it, de, es, pt, zh, ko, nl, ja, sv\n"
		"\t-v <switch>, --voice-prompts=<switch>\n"
		"\t\tChange whether voice-prompts are on or off.\n"
		"\t\tswitch: on, off\n"
		"\t-d, --device-status\n"
		"\t\tPrint the device status information. This includes its name, language,\n"
		"\t\tvoice-prompts, auto-off and noise cancelling settings.\n"
		"\t-p <status>, --pairing=<status>\n"
		"\t\tChange whether the device is pairing.\n"
		"\t\tstatus: on, off\n"
		"\t-f, --firmware-version\n"
		"\t\tPrint the firmware version on the device.\n"
		"\t-s, --serial-number\n"
		"\t\tPrint the serial number of the device.\n"
		"\t-b, --battery-level\n"
		"\t\tPrint the battery level of the device as a percent.\n"
		"\t-a, --paired-devices\n"
		"\t\tPrint the devices currently connected to the device.\n"
		"\t\t!: indicates the current device\n"
		"\t\t*: indicates other connected devices\n"
		"\t--connect-device=<address>\n"
		"\t\tAttempt to connect to the device at address.\n"
		"\t--disconnect-device=<address>\n"
		"\t\tDisconnect the device at address.\n"
		"\t--remove-device=<address>\n"
		"\t\tRemove the device at address from the pairing list.\n"
		"\t--device-id\n"
		"\t\tPrint the device id followed by the index revision.\n"
		"\t-e, --self-voice\n"
		"\t\tChange the self voice level.\n"
		"\t\tlevel: high, medium, low, off\n"
		, program_name);
}

static int do_set_name(int sock, const char *arg) {
	char name_buffer[MAX_NAME_LEN + 1] = { 0 };
	int status;

	if (strlen(arg) > MAX_NAME_LEN) {
		fprintf(stderr, "Name exceeds %d character maximum. Truncating.\n", MAX_NAME_LEN);
		status = 1;
	} else {
		strncpy(name_buffer, arg, MAX_NAME_LEN);
		status = set_name(sock, name_buffer);
	}
	return status;
}

static int do_set_prompt_language(int sock, const char *arg) {
	enum PromptLanguage pl;

	if (strcmp(arg, "en") == 0) {
		pl = PL_EN;
	} else if (strcmp(arg, "fr") == 0) {
		pl = PL_FR;
	} else if (strcmp(arg, "it") == 0) {
		pl = PL_IT;
	} else if (strcmp(arg, "de") == 0) {
		pl = PL_DE;
	} else if (strcmp(arg, "es") == 0) {
		pl = PL_ES;
	} else if (strcmp(arg, "pt") == 0) {
		pl = PL_PT;
	} else if (strcmp(arg, "zh") == 0) {
		pl = PL_ZH;
	} else if (strcmp(arg, "ko") == 0) {
		pl = PL_KO;
	} else if (strcmp(arg, "pl") == 0) {
		pl = PL_PL;
	} else if (strcmp(arg, "ru") == 0) {
		pl = PL_RU;
	}  else if (strcmp(arg, "nl") == 0) {
		pl = PL_NL;
	} else if (strcmp(arg, "ja") == 0) {
		pl = PL_JA;
	} else if (strcmp(arg, "sv") == 0) {
		pl = PL_SV;
	} else {
		fprintf(stderr, "Invalid prompt language argument: %s\n", arg);
		usage();
		return 1;
	}

	return set_prompt_language(sock, pl);
}

static int do_set_voice_prompts(int sock, const char *arg) {
	int on;

	if (strcmp(arg, "on") == 0) {
		on = 1;
	} else if (strcmp(arg, "off") == 0) {
		on = 0;
	} else {
		fprintf(stderr, "Invalid voice prompt argument: %s\n", arg);
		usage();
		return 1;
	}

	return set_voice_prompts(sock, on);
}

static int do_set_auto_off(int sock, const char *arg) {
	enum AutoOff ao;

	int parsed = atoi(arg);

	switch (parsed) {
		case AO_5_MIN:
		case AO_20_MIN:
		case AO_40_MIN:
		case AO_60_MIN:
		case AO_180_MIN:
			ao = parsed;
			break;
		default:
			if (strcmp(arg, "never") == 0) {
				ao = AO_NEVER;
			} else {
				fprintf(stderr, "Invalid auto-off argument: %s\n", arg);
				usage();
				return 1;
			}
	}

	return set_auto_off(sock, ao);
}

static int do_set_noise_cancelling(int sock, const char *arg) {
	enum NoiseCancelling nc;

	if (strcmp(arg, "high") == 0) {
		nc = NC_HIGH;
	} else if (strcmp(arg, "low") == 0) {
		nc = NC_LOW;
	} else if (strcmp(arg, "off") == 0) {
		nc = NC_OFF;
	} else {
		fprintf(stderr, "Invalid noise cancelling argument: %s\n", arg);
		usage();
		return 1;
	}

	unsigned int device_id;
	unsigned int index;
	int status = get_device_id(sock, &device_id, &index);
	if (status) {
		return status;
	}

	if (!has_noise_cancelling(device_id)) {
		fprintf(stderr, "This device does not have noise cancelling.\n");
		usage();
		return 1;
	}

	return set_noise_cancelling(sock, nc);
}

static int do_get_device_status(int sock) {
	char name[MAX_NAME_LEN + 1];
	enum PromptLanguage pl;
	enum AutoOff ao;
	enum NoiseCancelling nc;

	int status = get_device_status(sock, name, &pl, &ao, &nc);
	if (status) {
		return status;
	}

	char *print;
	printf("Name: %s\n", name);

	switch (pl & VP_MASK) {
		case PL_EN:
			print = "en";
			break;
		case PL_FR:
			print = "fr";
			break;
		case PL_IT:
			print = "it";
			break;
		case PL_DE:
			print = "de";
			break;
		case PL_ES:
			print = "es";
			break;
		case PL_PT:
			print = "pt";
			break;
		case PL_ZH:
			print = "zh";
			break;
		case PL_KO:
			print = "ko";
			break;
		case PL_NL:
			print = "nl";
			break;
		case PL_JA:
			print = "ja";
			break;
		case PL_SV:
			print = "sv";
			break;
		case PL_RU:
			print = "ru";
			break;
		case PL_PL:
			print = "pl";
			break;
		default:
			print = "unknown";
			break;
	}
	printf("Language: %s\n", print);
	printf("Voice Prompts: %s\n", pl & VP_MASK ? "on" : "off");

	printf("Auto-Off: ");
	if (ao) {
		printf("%d", ao);
	} else {
		printf("never");
	}
	printf("\n");

	if (nc != NC_DNE) {
		switch (nc) {
			case NC_HIGH:
				print = "high";
				break;
			case NC_LOW:
				print = "low";
				break;
			case NC_OFF:
				print = "off";
				break;
			default:
				return 1;
		}
		printf("Noise Cancelling: %s\n", print);
	}

	return 0;
}

static int do_set_pairing(int sock, const char *arg) {
	enum Pairing p;

	if (strcmp(arg, "on") == 0) {
		p = P_ON;
	} else if (strcmp(arg, "off") == 0) {
		p = P_OFF;
	} else {
		fprintf(stderr, "Invalid pairing argument: %s\n", arg);
		usage();
		return 1;
	}

	return set_pairing(sock, p);
}

static int do_set_self_voice(int sock, const char *arg) {
	enum SelfVoice p;

	if (strcmp(arg, "high") == 0) {
		p = SV_HIGH;
	} else if (strcmp(arg, "medium") == 0) {
		p = SV_MEDIUM;
	} else if (strcmp(arg, "low") == 0) {
		p = SV_LOW;
	} else if (strcmp(arg, "off") == 0) {
		p = SV_OFF;
	} else {
		fprintf(stderr, "Invalid self voice argument: %s\n", arg);
		usage();
		return 1;
	}

	return set_self_voice(sock, p);
}

static int do_get_firmware_version(int sock) {
	char version[VER_STR_LEN];
	int status = get_firmware_version(sock, version);

	if (status) {
		return status;
	}

	printf("%s\n", version);
	return 0;
}

static int do_get_serial_number(int sock) {
	char serial[0x100];
	int status = get_serial_number(sock, serial);

	if (status) {
		return status;
	}

	printf("%s\n", serial);
	return 0;
}

static int do_get_battery_level(int sock) {
	unsigned int level;
	int status = get_battery_level(sock, &level);

	if (status) {
		return status;
	}

	printf("%d\n", level);
	return 0;
}

static int do_get_paired_devices(int sock) {
	bdaddr_t devices[MAX_NUM_DEVICES];
	size_t num_devices;
	enum DevicesConnected connected;

	int status = get_paired_devices(sock, devices, &num_devices, &connected);
	if (status) {
		return status;
	}

	unsigned int num_connected;
	switch (connected) {
		case DC_ONE:
			num_connected = 1;
			break;
		case DC_TWO:
			num_connected = 2;
			break;
		default:
			return 1;
	}
	printf("Devices connected: %d\n", num_connected);

	size_t i;
	for (i = 0; i < num_devices; ++i) {
		struct Device device;
		status = get_device_info(sock, devices[i], &device);
		if (status) {
			return status;
		}

		char address[18];
		reverse_ba2str(&device.address, address);

		char status_symb;
		switch (device.status) {
			case DS_THIS:
				status_symb = '!';
				break;
			case DS_CONNECTED:
				status_symb = '*';
				break;
			case DS_DISCONNECTED:
				status_symb = ' ';
				break;
			default:
				return 1;
		}

		printf("%c %s %s\n", status_symb, address, device.name);
	}

	return 0;
}

static int do_connect_device(int sock, const char *arg) {
	bdaddr_t address;
	reverse_str2ba(arg, &address);
	return connect_device(sock, address);
}

static int do_disconnect_device(int sock, const char *arg) {
	bdaddr_t address;
	reverse_str2ba(arg, &address);
	return disconnect_device(sock, address);
}

static int do_remove_device(int sock, const char *arg) {
	bdaddr_t address;
	reverse_str2ba(arg, &address);
	return remove_device(sock, address);
}

static int do_get_device_id(int sock) {
	unsigned int device_id;
	unsigned int index;
	int status = get_device_id(sock, &device_id, &index);

	if (status) {
		return status;
	}

	printf("0x%04x %d\n", device_id, index);
	return 0;
}

static int do_send_packet(int sock, const char *arg) {
	uint8_t send[sizeof(arg) / 2];
	size_t i;
	for (i = 0; arg[i * 2]; ++i) {
		if (strtobyte(&arg[i * 2], &send[i]) != 0) {
			return 1;
		}
	}

	uint8_t recieved[MAX_BT_PACK_LEN];
	int recieved_n = send_packet(sock, send, sizeof(send), recieved);
	if (recieved_n < 0) {
		return recieved_n;
	}

	for (i = 0; i < recieved_n; ++i) {
		printf("%02x ", recieved[i]);
	}
	printf("\n");
	return 0;
}

int main(int argc, char *argv[]) {
	static const char *short_opt = "hn:l:v:o:c:e:dp:fsba";
	static const struct option long_opt[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "name", required_argument, NULL, 'n' },
		{ "prompt-language", required_argument, NULL, 'l' },
		{ "voice-prompts", required_argument, NULL, 'v' },
		{ "auto-off", required_argument, NULL, 'o' },
		{ "noise-cancelling", required_argument, NULL, 'c' },
		{ "device-status", no_argument, NULL, 'd' },
		{ "pairing", required_argument, NULL, 'p' },
		{ "firmware-version", no_argument, NULL, 'f' },
		{ "serial-number", no_argument, NULL, 's' },
		{ "battery-level", no_argument, NULL, 'b' },
		{ "paired-devices", no_argument, NULL, 'a' },
		{ "connect-device", required_argument, NULL, 2 },
		{ "disconnect-device", required_argument, NULL, 3 },
		{ "remove-device", required_argument, NULL, 4 },
		{ "device-id", no_argument, NULL, 5 },
		{ "self-voice", required_argument, NULL, 'e' },
		{ "send-packet", required_argument, NULL, 1 },
		{ 0, 0, 0, 0 }
	};

	static const struct timeval send_timeout = { 5, 0 };
	static const struct timeval recieve_timeout = { 1, 0 };
	int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &send_timeout, sizeof(send_timeout)) < 0) {
		perror("Could not set socket send timeout");
		return 1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &recieve_timeout, sizeof(recieve_timeout)) < 0) {
		perror("Could not set socket recieve timeout");
		return 1;
	}

	program_name = argv[0];

	// Find connection address and verify options
	int opt;
	int opt_index = 0;
	while ((opt = getopt_long(argc, argv, short_opt, long_opt, &opt_index)) > 0) {
		switch (opt) {
			case 'h':
				usage();
				return 0;
			case '?':
				usage();
				return 1;
			default:
				break;
		}
	}

	if (argc - 1 != optind) {
		fprintf(stderr, argc <= optind
				? "An address argument must be given.\n"
				: "Only one address argument may be given.\n");
		usage();
		return 1;
	}

	struct sockaddr_rc address;
	address.rc_family = AF_BLUETOOTH;
	address.rc_channel = BOSE_CHANNEL;
	if (str2ba(argv[optind], &address.rc_bdaddr) != 0) {
		fprintf(stderr, "Invalid bluetooth address: %s\n", argv[optind]);
		return 1;
	}

	if (connect(sock, (struct sockaddr *) &address, sizeof(address)) != 0) {
		perror("Could not connect to Bluetooth device");
		return 1;
	}

	int status = init_connection(sock);

	opt_index = 0;
	optind = 1;
	while ((opt = getopt_long(argc, argv, short_opt, long_opt, &opt_index)) > 0) {
		if (status) {
			break;
		}

		switch (opt) {
			case 'n':
				status = do_set_name(sock, optarg);
				break;
			case 'l':
				status = do_set_prompt_language(sock, optarg);
				break;
			case 'v':
				status = do_set_voice_prompts(sock, optarg);
				break;
			case 'o':
				status = do_set_auto_off(sock, optarg);
				break;
			case 'c':
				status = do_set_noise_cancelling(sock, optarg);
				break;
			case 'd':
				status = do_get_device_status(sock);
				break;
			case 'p':
				status = do_set_pairing(sock, optarg);
				break;
			case 'f':
				status = do_get_firmware_version(sock);
				break;
			case 's':
				status = do_get_serial_number(sock);
				break;
			case 'b':
				status = do_get_battery_level(sock);
				break;
			case 'a':
				status = do_get_paired_devices(sock);
				break;
			case 'e':
				status = do_set_self_voice(sock, optarg);
				break;
			case 2:
				status = do_connect_device(sock, optarg);
				break;
			case 3:
				status = do_disconnect_device(sock, optarg);
				break;
			case 4:
				status = do_remove_device(sock, optarg);
				break;
			case 5:
				status = do_get_device_id(sock);
				break;
			case 1:
				status = do_send_packet(sock, optarg);
				break;
			default:
				status = 1;
		}
	}

	if (status < 0) {
		perror("Error trying to change setting");
	}

	close(sock);
	return status;
}
