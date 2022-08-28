// Minimal abstraction layer over WinSock2 / BSD sockets

#ifndef _SOCKET_H
#define _SOCKET_H

#ifdef _WIN32

#include <winsock2.h>
#include <stdint.h>
#define psockerror(x) do { \
		wchar_t *s = NULL; \
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&s, 0, NULL); \
		fprintf(stderr, "%s: %S\n", x, s); \
		LocalFree(s); \
	} while (0)
#define close(x) closesocket(x)
typedef SOCKET socktype_t;
#define AF_BLUETOOTH AF_BTH
#define BTPROTO_RFCOMM BTHPROTO_RFCOMM

#else

#include <bluetooth/bluetooth.h>
#define psockerror(x) perror(x)
typedef int socktype_t;

#endif

#endif /* _SOCKET_H */
