#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <getopt.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BYTE_MAX 0xFF
#define CN_BASE_PACK_LEN 3

// TODO: make this dynamically read from commandline
static char dest[18] = "04:52:C7:5B:8B:47";

int change_name(int sock, char *name) {
	char packet[CN_BASE_PACK_LEN + BYTE_MAX] = { 0x01, 0x02, 0x02 };
	size_t length = strlen(name);

	if (length > BYTE_MAX) {
		length = BYTE_MAX;
		printf("Length of name too long. Truncating to %d characters.\n", BYTE_MAX);
	}

	packet[CN_BASE_PACK_LEN] = (char) length;
	strncpy(&packet[CN_BASE_PACK_LEN + 1], name, BYTE_MAX);

	return write(sock, packet, CN_BASE_PACK_LEN + 1 + length);
}

int main(int argc, char *argv[]) {
// TODO: implement command arg parsing
//	struct option command_options[] = {
//		{ "set-name", no_argument, NULL, 1 }
//	};

	int status;
	int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	struct sockaddr_rc address = {
		AF_BLUETOOTH,
		*BDADDR_ANY,
		8 // TODO: think about refactoring this out into a constant
	};
	str2ba(dest, &address.rc_bdaddr);

	status = connect(sock, (struct sockaddr *) &address, sizeof(address));

	change_name(sock, "this is a test");

	close(sock);
	return 0;
}
