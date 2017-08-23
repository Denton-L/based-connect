#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "based.h"

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
	int opt_index = 0;
	int opt;

	char set_name_arg[MAX_NAME_LEN + 1] = { 0 };
	char noise_cancelling_arg = -1;
	int auto_off_arg = -1;
	char prompt_language_arg = -1;

	const struct timeval send_timeout = { 5, 0 };
	const struct timeval recieve_timeout = { 1, 0 };
	int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	struct sockaddr_rc address = {
		AF_BLUETOOTH,
		*BDADDR_ANY,
		BOSE_CHANNEL
	};

	while ((opt = getopt_long(argc, argv, short_opt, long_opt, &opt_index)) != -1) {
		switch (opt) {
			case 'h':
				// TODO: print usage string
				return 0;

			case 'n':
				if (strlen(optarg) > MAX_NAME_LEN) {
					fprintf(stderr, "Length of name exceeds %d character maximum.\n", MAX_NAME_LEN);
					return 1;
				}
				strncpy(set_name_arg, optarg, MAX_NAME_LEN);
				break;

			case 'c':
				if (strcmp(optarg, "high") == 0) {
					noise_cancelling_arg = NC_HIGH;
				} else if (strcmp(optarg, "low") == 0) {
					noise_cancelling_arg = NC_LOW;
				} else if (strcmp(optarg, "off") == 0) {
					noise_cancelling_arg = NC_OFF;
				} else {
					fprintf(stderr, "Invalid noise cancelling argument: %s\n", optarg);
					return 1;
				}
				break;

			case 'o':
				auto_off_arg = atoi(optarg);

				switch (auto_off_arg) {
					case AO_NEVER:
					case AO_5_MIN:
					case AO_20_MIN:
					case AO_40_MIN:
					case AO_60_MIN:
					case AO_180_MIN:
						break;
					default:
						fprintf(stderr, "Invalid auto-off argument: %s\n", optarg);
						fprintf(stderr, "Must be one of %d, %d, %d, %d, %d, %d\n",
								AO_NEVER, AO_5_MIN, AO_20_MIN, AO_40_MIN, AO_60_MIN, AO_180_MIN);
						return 1;
				}
				break;

			case 'l':
				if (strcmp(optarg, "off") == 0) {
					prompt_language_arg = PL_OFF;
				} else if (strcmp(optarg, "en") == 0) {
					prompt_language_arg = PL_EN;
				} else if (strcmp(optarg, "fr") == 0) {
					prompt_language_arg = PL_FR;
				} else if (strcmp(optarg, "it") == 0) {
					prompt_language_arg = PL_IT;
				} else if (strcmp(optarg, "de") == 0) {
					prompt_language_arg = PL_DE;
				} else if (strcmp(optarg, "es") == 0) {
					prompt_language_arg = PL_ES;
				} else if (strcmp(optarg, "pt") == 0) {
					prompt_language_arg = PL_PT;
				} else if (strcmp(optarg, "zh") == 0) {
					prompt_language_arg = PL_ZH;
				} else if (strcmp(optarg, "ko") == 0) {
					prompt_language_arg = PL_KO;
				} else if (strcmp(optarg, "nl") == 0) {
					prompt_language_arg = PL_NL;
				} else if (strcmp(optarg, "ja") == 0) {
					prompt_language_arg = PL_JA;
				} else if (strcmp(optarg, "sv") == 0) {
					prompt_language_arg = PL_SV;
				} else {
					fprintf(stderr, "Invalid prompt language argument: %s\n", optarg);
					return 1;
				}

				break;

			case '?':
				// TODO: print error message if opterr != 0
				return 1;

			default:
				abort();
		}
	}

	if (argc - 1 != optind) {
		fprintf(stderr, "Invalid arguments provided.\n");
		return 1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &send_timeout, sizeof(send_timeout)) < 0) {
		perror("Could not set socket send timeout");
		return 1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &recieve_timeout, sizeof(recieve_timeout)) < 0) {
		perror("Could not set socket recieve timeout");
		return 1;
	}

	str2ba(argv[optind], &address.rc_bdaddr);
	if (connect(sock, (struct sockaddr *) &address, sizeof(address)) != 0) {
		perror("Could not connect to Bluetooth device");
		return 1;
	}

	// TODO: refactor this common code together so can differentiate between > 0 and < 0 exit code
	if (set_name_arg[0]) {
		if (set_name(sock, set_name_arg) != 0) {
			goto error;
		}
	}

	if (noise_cancelling_arg >= 0) {
		if (noise_cancelling(sock, noise_cancelling_arg) != 0) {
			goto error;
		}
	}

	if (auto_off_arg >= 0) {
		if (auto_off(sock, auto_off_arg) != 0) {
			goto error;
		}
	}

	if (prompt_language_arg >= 0) {
		if (prompt_language(sock, prompt_language_arg) != 0) {
			goto error;
		}
	}

	close(sock);
	return 0;

error:
	perror("Failed to change setting");
	close(sock);
	return 1;
}
