#pragma once
#ifndef __SOCKET_ADDRESS_H__
#define __SOCKET_ADDRESS_H__
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdint.h>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

class SocketAddress
{
public:
	SocketAddress() {}
	//Constructor to create an INET socket IPV4
	SocketAddress(uint32_t a_inAddress, uint16_t a_inPort)
	{
		sockaddr_in* pSockAddrIn = GetAsSockAddrIn();
		pSockAddrIn->sin_family = AF_INET;
		pSockAddrIn->sin_addr.S_un.S_addr = htonl(a_inAddress);
		pSockAddrIn->sin_port = htons(a_inPort);
	}
	//Constructor with IP address as a string
	SocketAddress(std::string a_inAddress, uint16_t a_inPort)
	{
		sockaddr_in* pSockAddrIn = GetAsSockAddrIn();
		pSockAddrIn->sin_family = AF_INET;
		pSockAddrIn->sin_port = htons(a_inPort);
		InetPton(AF_INET, a_inAddress.c_str(), &pSockAddrIn->sin_addr);
	}
	//Construct form existing socket address
	SocketAddress(const sockaddr& a_inSockAddr)
	{
		memcpy(&m_SockAddr, &a_inSockAddr, sizeof(sockaddr));
	}
	//Return the size of the underlying sockaddr function
	size_t GetSize() const { return sizeof(sockaddr); }

	std::string ToString()
	{
		sockaddr_in* pSockIn = GetAsSockAddrIn();
		std::string str("Recieved packet from: ");
		str += inet_ntoa(pSockIn->sin_addr);
		str += " ";
		str += ntohs(pSockIn->sin_port);
		return str;
	}
private:
	friend class UDPSocket;
	//The Socket address variable
	sockaddr m_SockAddr;
	//Private function to map the sockaddr member variable to a sockaddr_in
	sockaddr_in* GetAsSockAddrIn()
	{
		return reinterpret_cast<sockaddr_in*>(&m_SockAddr);
	}
};

#endif