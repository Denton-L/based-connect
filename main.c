#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "based.h"

static void usage(const char *program) {
	printf("Usage: %s [options] <address>\n"
			"\taddress: The Bluetooth address of the headphones.\n"
			"Options:\n"
			"\t-h, --help\n"
			"\t\tPrint the help message.\n"
			"\t-n <name>, --name=<name>\n"
			"\t\tChange the name of the headphones.\n"
			"\t-c <level>, --noise-cancelling=<level>\n"
			"\t\tChange the noise cancelling level.\n"
			"\t\tlevel: high, low, off\n"
			"\t-o <minutes>, --auto-off=<minutes>\n"
			"\t\tChange the auto-off time.\n"
			"\t\tminutes: never, 5, 20, 40, 50, 180\n"
			"\t-l <language>, --prompt-language=<language>\n"
			"\t\tChange the voice-prompt language.\n"
			"\t\tlanguage: off, en, fr, it, de, es, pt, zh, ko, nl, ja, sv\n"
			"\t-f, --firmware-version\n"
			"\t\tPrint the firmware version on the headphones.\n"
			"\t-b, --battery-level\n"
			"\t\tPrint the battery level of the headphones as a percent.\n", program);
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
		return 1;
	}

	return set_noise_cancelling(sock, nc);
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
		default:
			if (strcmp(arg, "never") == 0) {
				ao = AO_NEVER;
			} else {
				fprintf(stderr, "Invalid auto-off argument: %s\n", arg);
				return 1;
			}
	}

	return set_auto_off(sock, ao);
}

static int do_set_prompt_language(int sock, const char *arg) {
	enum PromptLanguage pl;

	if (strcmp(arg, "off") == 0) {
		pl = PL_OFF;
	} else if (strcmp(arg, "en") == 0) {
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
	} else if (strcmp(arg, "nl") == 0) {
		pl = PL_NL;
	} else if (strcmp(arg, "ja") == 0) {
		pl = PL_JA;
	} else if (strcmp(arg, "sv") == 0) {
		pl = PL_SV;
	} else {
		fprintf(stderr, "Invalid prompt language argument: %s\n", arg);
		return 1;
	}

	return set_prompt_language(sock, pl);
}

static int do_get_firmware_version(int sock) {
	char version[6];
	int status = get_firmware_version(sock, version);

	if (status != 0) {
		return status;
	}

	printf("%s\n", version);
	return 0;
}

static int do_get_battery_level(int sock) {
	unsigned int level;
	int status = get_battery_level(sock, &level);

	if (status != 0) {
		return status;
	}

	printf("%d\n", level);
	return 0;
}

int main(int argc, char *argv[]) {
	const char *short_opt = "hn:c:o:l:fb";
	const struct option long_opt[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "name", required_argument, NULL, 'n' },
		{ "noise-cancelling", required_argument, NULL, 'c' },
		{ "auto-off", required_argument, NULL, 'o' },
		{ "prompt-language", required_argument, NULL, 'l' },
		{ "firmware-version", no_argument, NULL, 'f' },
		{ "battery-level", no_argument, NULL, 'b' },
		{ 0, 0, 0, 0 }
	};

	const struct timeval send_timeout = { 5, 0 };
	const struct timeval recieve_timeout = { 1, 0 };
	int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &send_timeout, sizeof(send_timeout)) < 0) {
		perror("Could not set socket send timeout");
		return 1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &recieve_timeout, sizeof(recieve_timeout)) < 0) {
		perror("Could not set socket recieve timeout");
		return 1;
	}

	// Find connection address and verify options
	int opt;
	int opt_index = 0;
	while ((opt = getopt_long(argc, argv, short_opt, long_opt, &opt_index)) > 0) {
		switch (opt) {
			case 'h':
				usage(argv[0]);
				return 0;
			case '?':
				usage(argv[0]);
				return 1;
			default:
				break;
		}
	}

	if (argc - 1 != optind) {
		fprintf(stderr, argc <= optind
				? "An address argument must be given.\n"
				: "Only one address argument may be given.\n");
		usage(argv[0]);
		return 1;
	}

	struct sockaddr_rc address = {
		AF_BLUETOOTH,
		*BDADDR_ANY,
		BOSE_CHANNEL
	};
	str2ba(argv[optind], &address.rc_bdaddr);
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

			case 'c':
				status = do_set_noise_cancelling(sock, optarg);
				break;

			case 'o':
				status = do_set_auto_off(sock, optarg);
				break;

			case 'l':
				status = do_set_prompt_language(sock, optarg);
				break;

			case 'f':
				status = do_get_firmware_version(sock);
				break;

			case 'b':
				status = do_get_battery_level(sock);
				break;

			default:
				abort();
		}
	}

	if (status < 0) {
		perror("Error trying to change setting");
	}

	close(sock);
	return status;
}
