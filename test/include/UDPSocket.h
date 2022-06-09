#pragma once
#ifndef  __UDP_SOCKET_H__
#define  __UDP_SOCKET_H__
#include <stdafx.h>
#include "SocketAddress.h"

//A class to wrap up the Creation of UDP Sockets
class UDPSocket
{
public:
	~UDPSocket();
	int Bind(const SocketAddress& a_inToAddress);
	int SendTo(const void* a_inData, int a_inLength, const SocketAddress& a_inTo);
	int RecieveFrom(void* a_inBuffer, int a_inLength, SocketAddress& a_outFrom);
private:
	friend class SocketUtil;
	UDPSocket(SOCKET a_inSocket) : m_Socket(a_inSocket) {}
	SOCKET m_Socket;
};
#endif // ! __UDP_SOCKET_H__
