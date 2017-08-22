#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define CN_P1_LEN 3
#define CN_P2_LEN 4

// TODO: make this dynamically read from commandline
static char dest[18] = "04:52:C7:5B:8B:47";

int change_name(char *name) {
	char packet1[CN_P1_LEN + 0xFF] = { 0x01, 0x02, 0x02 };
	char packet2[CN_P2_LEN] = { 0x01, 0x01, 0x05, 0x00 };
	size_t length = strlen(name);

	int status;
	int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	struct sockaddr_rc address = {
		AF_BLUETOOTH,
		*BDADDR_ANY,
		8 // TODO: think about refactoring this out into a constant
	};
	str2ba(dest, &address.rc_bdaddr);

	if (length > 0xFF) {
		length = 0xFF;
	}

	packet1[CN_P1_LEN] = (char) length;
	strncpy(&packet1[CN_P1_LEN + 1], name, 0xFF);

	status = connect(sock, (struct sockaddr *) &address, sizeof(address));

	printf("Status after connect: %d\n", status);
	if (status == 0) {
		status = write(sock, packet1, CN_P1_LEN + 1 + length);
		printf("Status after packet 1: %d\n", status);
		// status = write(sock, packet2, CN_P2_LEN);
		// printf("Status after packet 2: %d\n", status);
	}

	close(sock);
	return status;
}

int main(int argc, char *argv[]) {
// TODO: implement command arg parsing
//	struct option command_options[] = {
//		{ "set-name", no_argument, NULL, 1 }
//	};


	change_name("this is a test");
	return 0;
}
