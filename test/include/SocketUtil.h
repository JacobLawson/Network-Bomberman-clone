#pragma once
#ifndef __SOCKET_UTIL_H__
#define __SOCKET_UTIL_H__
#include "SocketAddress.h"

enum SocketAddressFamily
{
	INET = AF_INET,
	INET6 = AF_INET6
};

class SocketUtil
{
public:
	static bool Init();
	static void CleanUp();

	static UDPSocket* CreateUDPSocket(SocketAddressFamily a_inFamily);
	//static TCPSocket* CreateTCPSocket(SocketAddressFamily inFamily);
};

#endif // !__SOCKET_UTIL_H__
