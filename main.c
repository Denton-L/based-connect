#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "based.h"

int main(int argc, char *argv[]) {
	const char *short_opt = "+hn:c:v:";
	const struct option long_opt[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "set-name", required_argument, NULL, 'n' },
		{ "noise-cancelling", required_argument, NULL, 'c' },
		{ "voice-prompts", required_argument, NULL, 'v' },
		{ 0, 0, 0, 0 }
	};
	int opt_index = 0;
	int opt;

	char *set_name_arg = NULL;
	char noise_cancelling_arg = -1;
	char voice_prompts_arg = -1;

	int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	struct sockaddr_rc address = {
		AF_BLUETOOTH,
		*BDADDR_ANY,
		BT_CHANNEL
	};

	while ((opt = getopt_long(argc, argv, short_opt, long_opt, &opt_index)) != -1) {
		switch (opt) {
			case 'h':
				// TODO: print usage string
				return 0;

			case 'n':
				set_name_arg = optarg;
				break;

			case 'c':
				if (strcmp(optarg, "high") == 0) {
					noise_cancelling_arg = NC_HIGH;
				} else if (strcmp(optarg, "low") == 0) {
					noise_cancelling_arg = NC_LOW;
				} else if (strcmp(optarg, "off") == 0) {
					noise_cancelling_arg = NC_OFF;
				} else {
					printf("Invalid noise cancelling argument: %s\n", optarg);
					return 1;
				}
				break;

			case 'v':
				if (strcmp(optarg, "on") == 0) {
					voice_prompts_arg = VP_ON;
				} else if (strcmp(optarg, "off") == 0) {
					voice_prompts_arg = VP_OFF;
				} else {
					printf("Invalid voice prompts argument: %s\n", optarg);
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
		printf("The address must be provided as the final argument.\n");
		return 1;
	}

	str2ba(argv[optind], &address.rc_bdaddr);
	if (connect(sock, (struct sockaddr *) &address, sizeof(address)) != 0) {
		perror("Could not connect to Bluetooth device");
		return 1;
	}

	if (set_name_arg) {
		if (set_name(sock, set_name_arg) < 0) {
			goto error;
		}
	}

	if (noise_cancelling_arg >= 0) {
		if (noise_cancelling(sock, noise_cancelling_arg) < 0) {
			goto error;
		}
	}

	if (voice_prompts_arg >= 0) {
		if (voice_prompts(sock, voice_prompts_arg) < 0) {
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
