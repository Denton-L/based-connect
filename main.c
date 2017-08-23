#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include "based.h"

int main(int argc, char *argv[]) {
	const char *short_opt = "+hn:";
	const struct option long_opt[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "set-name", required_argument, NULL, 'n' },
		{ 0, 0, 0, 0 }
	};
	int opt_index = 0;
	int opt;

	char *change_name_arg = NULL;

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
				change_name_arg = optarg;
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
		printf("Could not connect to Bluetooth device. (%d)\n", errno);
		return 1;
	}

	if (change_name_arg) {
		if (change_name(sock, change_name_arg) < 0) {
			printf("Failed to change name. (%d)\n", errno);
			goto error;
		}
	}

	close(sock);
	return 0;

error:
	close(sock);
	return 1;
}
