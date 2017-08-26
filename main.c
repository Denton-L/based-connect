#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "based.h"

static void usage() {
	// TODO: print usage string
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

	if (strcmp(optarg, "high") == 0) {
		nc = NC_HIGH;
	} else if (strcmp(optarg, "low") == 0) {
		nc = NC_LOW;
	} else if (strcmp(optarg, "off") == 0) {
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
		case AO_NEVER:
		case AO_5_MIN:
		case AO_20_MIN:
		case AO_40_MIN:
		case AO_60_MIN:
		case AO_180_MIN:
			ao = parsed;
		default:
			fprintf(stderr, "Invalid auto-off argument: %s\n", arg);
			return 1;
	}

	return set_auto_off(sock, ao);
}

static int do_set_prompt_language(int sock, const char *arg) {
	enum PromptLanguage pl;

	if (strcmp(optarg, "off") == 0) {
		pl = PL_OFF;
	} else if (strcmp(optarg, "en") == 0) {
		pl = PL_EN;
	} else if (strcmp(optarg, "fr") == 0) {
		pl = PL_FR;
	} else if (strcmp(optarg, "it") == 0) {
		pl = PL_IT;
	} else if (strcmp(optarg, "de") == 0) {
		pl = PL_DE;
	} else if (strcmp(optarg, "es") == 0) {
		pl = PL_ES;
	} else if (strcmp(optarg, "pt") == 0) {
		pl = PL_PT;
	} else if (strcmp(optarg, "zh") == 0) {
		pl = PL_ZH;
	} else if (strcmp(optarg, "ko") == 0) {
		pl = PL_KO;
	} else if (strcmp(optarg, "nl") == 0) {
		pl = PL_NL;
	} else if (strcmp(optarg, "ja") == 0) {
		pl = PL_JA;
	} else if (strcmp(optarg, "sv") == 0) {
		pl = PL_SV;
	} else {
		fprintf(stderr, "Invalid prompt language argument: %s\n", arg);
		return 1;
	}

	return set_prompt_language(sock, pl);
}

int main(int argc, char *argv[]) {
	const char *short_opt = "+hn:c:o:l:";
	const struct option long_opt[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "set-name", required_argument, NULL, 'n' },
		{ "noise-cancelling", required_argument, NULL, 'c' },
		{ "auto-off", required_argument, NULL, 'o' },
		{ "prompt-language", required_argument, NULL, 'l' },
		{ 0, 0, 0, 0 }
	};
	int opt_index;
	int opt;

	const struct timeval send_timeout = { 5, 0 };
	const struct timeval recieve_timeout = { 1, 0 };
	int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	struct sockaddr_rc address = {
		AF_BLUETOOTH,
		*BDADDR_ANY,
		BOSE_CHANNEL
	};

	int status;

	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &send_timeout, sizeof(send_timeout)) < 0) {
		perror("Could not set socket send timeout");
		return 1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &recieve_timeout, sizeof(recieve_timeout)) < 0) {
		perror("Could not set socket recieve timeout");
		return 1;
	}

	// Find connection address and verify options
	opt_index = 0;
	while ((opt = getopt_long(argc, argv, short_opt, long_opt, &opt_index)) > 0) {
		switch (opt) {
			case 'h':
				usage();
				return 0;
			case '?':
				// TODO: print out usage string
				return 1;
			default:
				break;
		}
	}

	str2ba(argv[optind], &address.rc_bdaddr);
	if (connect(sock, (struct sockaddr *) &address, sizeof(address)) != 0) {
		perror("Could not connect to Bluetooth device");
		return 1;
	}

	status = init_connection(sock);

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
