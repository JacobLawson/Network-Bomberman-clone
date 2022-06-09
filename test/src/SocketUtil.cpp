#include <stdafx.h>
#include "SocketUtil.h"
#include "UDPSocket.h"

bool SocketUtil::Init()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		return false;
	}
	return true;
}

void SocketUtil::CleanUp()
{
	WSACleanup();
}

UDPSocket* SocketUtil::CreateUDPSocket(SocketAddressFamily a_inFamily)
{
	SOCKET s = socket(a_inFamily, SOCK_DGRAM, IPPROTO_UDP);
	if (s != INVALID_SOCKET)
	{
		return new UDPSocket(s);
	}
	else
	{
		return nullptr;
	}
}
