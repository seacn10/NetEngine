// TCPClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>

#include "..\NetEngine\Packet.h"

#pragma comment(lib, "ws2_32.lib")

// Module Name: tcpclient.cpp
//
// Description:
//
//    This sample illustrates how to develop a simple TCP client application
//    that can send a simple "hello" message to a TCP server listening on port 5150.
//    This sample is implemented as a console-style application and simply prints
//    status messages a connection is made and when data is sent to the server.
//
// Compile:
//
//    cl -o tcpclient tcpclient.cpp ws2_32.lib
//
// Command Line Options:
//
//    tcpclient.exe <server IP address> 
//

typedef struct UserEnableInsure
{
	DWORD							dwUserID;
	TCHAR							szLogonPass[33];
	TCHAR							szInsurePass[33];
	TCHAR							szMachineID[33];
}UserEnableInsure;

unsigned short mappedBuffer(void* pData, WORD wDataSize)
{
	//变量定义
	BYTE *buffer = (BYTE*)pData;
	BYTE cbCheckCode = 0;

	//映射数据
	for (WORD i = sizeof(TCP_Info); i < wDataSize; i++)
	{
		cbCheckCode += buffer[i];
		buffer[i] = g_SendByteMap[buffer[i]];
	}

	//设置数据
	TCP_Info *pInfo = (TCP_Info*)pData;
	pInfo->cbCheckCode = ~cbCheckCode + 1;
	pInfo->wPacketSize = wDataSize;
	pInfo->cbDataKind = DK_MAPPED;

	return 1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA              wsaData;
	SOCKET               s;
	SOCKADDR_IN          ServerAddr;
	int                  Port = 5150;
	int                  Ret;

	if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", Ret);
		return 0;
	}
	if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
		== INVALID_SOCKET)
	{
		printf("socket failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.s_addr = inet_addr("172.20.183.247");

	printf("We are trying to connect to %s:%d...\n",
		inet_ntoa(ServerAddr.sin_addr), htons(ServerAddr.sin_port));

	if (connect(s, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr))
		== SOCKET_ERROR)
	{
		printf("connect failed with error %d\n", WSAGetLastError());
		closesocket(s);
		WSACleanup();
		return 0;
	}

	printf("Our connection succeeded.\n");

	TCHAR szMachineID[33] = { 0 };
	TCHAR szLogonPass[33] = { 0 };
	TCHAR szInsurePass[33] = { 0 };
	
	UserEnableInsure UserEnableInsure;
	UserEnableInsure.dwUserID = 12;
	_tcscpy_s(UserEnableInsure.szMachineID, 33, L"{75F5F041-BC63-4544-9F03}");
	_tcscpy_s(UserEnableInsure.szLogonPass, 33, L"szLogonPass");
	_tcscpy_s(UserEnableInsure.szInsurePass, 33, L"szInsurePass");

	WORD dwSize = sizeof(UserEnableInsure);

	TCP_Buffer tcp_buffer;
	memset(&tcp_buffer, 0, sizeof(TCP_Buffer));
	tcp_buffer.Head.CommandInfo.wMainCmdID = 3;
	tcp_buffer.Head.CommandInfo.wSubCmdID = 160;
	tcp_buffer.Head.TCPInfo.cbCheckCode = 0;
	tcp_buffer.Head.TCPInfo.cbDataKind = DK_MAPPED;
	tcp_buffer.Head.TCPInfo.wPacketSize = sizeof(tcp_buffer.Head);

	memcpy(&tcp_buffer.cbBuffer, &UserEnableInsure, dwSize);

	mappedBuffer(&tcp_buffer, dwSize);

	byte *pByte = new byte[dwSize];
	memcpy(pByte, &tcp_buffer, dwSize);

	if ((Ret = send(s, (char*)pByte, dwSize, 0)) == SOCKET_ERROR)
	{
		printf("send failed with error %d\n", WSAGetLastError());
		closesocket(s);
		WSACleanup();
		return 0;
	}

	getchar();

	printf("We are closing the connection.\n");

	closesocket(s);

	WSACleanup();

	return 0;
}

