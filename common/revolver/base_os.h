#ifndef __BASE_OS_H
#define __BASE_OS_H

#include "revolver/base_typedef.h"
#include <time.h>
#include <string>
#include <stdio.h>

#define INVALID_HANDLER		-1

#undef FD_SETSIZE
#define FD_SETSIZE      512

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mswsock.h>
#include <direct.h>

#pragma comment(lib, "ws2_32.lib")

typedef SOCKET BASE_HANDLER;

inline int set_socket_nonblocking(BASE_HANDLER socket)
{
	int mode = 1;
	::ioctlsocket(socket, FIONBIO, (u_long FAR*)&mode);

	return 0;
}

//inline int close(BASE_HANDLER socket)
//{
//	return closesocket(socket);
//}

inline int error_no()
{
	return ::WSAGetLastError();
}

inline void usleep(uint32_t us)
{	
	if(us < 1000)
		::Sleep(1);
	else
		::Sleep(us / 1000);
}

inline int access(const char *path, int mode)
{
	return _access(path, mode);
}

#define	F_OK	0
#define	X_OK	1
#define	W_OK	2
#define	R_OK	4

#define XEAGAIN			WSAEWOULDBLOCK		//无数据读写
#define XEINPROGRESS	WSAEWOULDBLOCK		//正在进行连接
#define XECONNABORTED	WSAECONNABORTED
#define XEINTR			WSAEWOULDBLOCK

#else //LINUX GCC
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>

typedef int		BASE_HANDLER;

#define XEAGAIN			EAGAIN		//无数据读写
#define XEINPROGRESS	EINPROGRESS		//正在进行连接
#define XECONNABORTED	ECONNABORTED
#define XEINTR			EINTR

inline int set_socket_nonblocking(BASE_HANDLER socket)
{
	int val = fcntl(socket, F_GETFL, 0);
	fcntl(socket, F_SETFL, val | O_NONBLOCK);
	
	return 0;
}

inline int error_no()
{
	return errno;
}

inline int closesocket(BASE_HANDLER socket)
{
	return close(socket);
}

#endif


inline void init_socket()
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	srand((uint32_t)time(NULL));
#else
	srand((uint32_t)time(NULL));
	srandom((uint32_t)time(NULL));
#endif
}

inline void destroy_socket()
{

#ifdef WIN32
	WSACleanup();
#endif
}

inline const char* get_file_name(const char* pathname)
{
	if(pathname == NULL)
		return pathname;

	int32_t size = strlen(pathname);

	char *pos = (char *)pathname + size;

#ifdef WIN32
	while (*pos != '\\' && pos != pathname)
#else
	while (*pos != '/' && pos != pathname)
#endif
		pos--;

	if(pos == pathname)
		return pathname;
	else
		return pos + 1;
}

#endif

