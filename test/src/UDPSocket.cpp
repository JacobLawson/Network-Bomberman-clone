#include <stdafx.h>
#include "UDPSocket.h"

//Destructor closes socket
UDPSocket::~UDPSocket()
{
	closesocket(m_Socket);
}

int UDPSocket::Bind(const SocketAddress& a_inToAddress)
{
	int err = bind(m_Socket, &a_inToAddress.m_SockAddr, a_inToAddress.GetSize());
	if (err != 0)
	{
		//Functionality To Handle Errors goes here
		return err;
	}
	return NO_ERROR;
}

int UDPSocket::SendTo(const void* a_inData, int a_inLength, const SocketAddress& a_inTo)
{
	int byteSentCount = sendto(m_Socket, static_cast<const char*>(a_inData), a_inLength, 0, &a_inTo.m_SockAddr, a_inTo.GetSize());
	if (byteSentCount >= 0)
	{
		return byteSentCount;
	}
	else
	{
		//Add Functionality to handle errors more gracefully
		return -1;
	}
}

int UDPSocket::RecieveFrom(void* a_inBuffer, int a_inLength, SocketAddress& a_outFrom)
{
	int fromLength = a_outFrom.GetSize();
	int readByteCount = recvfrom(m_Socket, static_cast<char*>(a_inBuffer), a_inLength, 0, &a_outFrom.m_SockAddr, &fromLength);
	if (readByteCount > 0)
	{
		return readByteCount;
	}
	else
	{
		//Handle errors
		return -1;
	}
}