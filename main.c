#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <getopt.h>
#include <sys/socket.h>
#include <unistd.h>

#include "based.h"

// TODO: make this dynamically read from commandline
static char dest[18] = "04:52:C7:5B:8B:47";

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
		BT_CHANNEL
	};
	str2ba(dest, &address.rc_bdaddr);

	status = connect(sock, (struct sockaddr *) &address, sizeof(address));

	change_name(sock, "this is a test");

	close(sock);
	return 0;
}
