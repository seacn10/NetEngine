#pragma once

#include "NetCoreHead.h"
#include "Module.h"
#include "Packet.h"
#include "WHDataQueue.h"
#include "Array.h"

#define MAX_ASYNCHRONISM_DATA		16384	

#define EVENT_TCP_NETWORK_ACCEPT	0x0007	
#define EVENT_TCP_NETWORK_READ		0x0008	
#define EVENT_TCP_NETWORK_SHUT		0x0009	

typedef struct UserEnableInsure
{
	DWORD							dwUserID;
	TCHAR							szLogonPass[33];
	TCHAR							szInsurePass[33];
	TCHAR							szMachineID[33];
}UserEnableInsure;

struct TCPNetworkAcceptEvent
{
	DWORD							dwSocketID;							
	DWORD							dwClientAddr;						
};

struct TCPNetworkReadEvent
{
	WORD							wDataSize;							
	DWORD							dwSocketID;							
	TCP_Command						Command;							
};

struct TCPNetworkShutEvent
{
	DWORD							dwSocketID;							
	DWORD							dwClientAddr;						
	DWORD							dwActiveTime;						
};

#ifdef _UNICODE
#define VER_IServiceModule INTERFACE_VERSION(1,1)
static const GUID IID_IServiceModule = { 0x49084dea, 0x4420, 0x4bea, 0x0080, 0x64, 0xfa, 0x37, 0xe3, 0x42, 0xf3, 0x1c };
#else
#define VER_IServiceModule INTERFACE_VERSION(1,1)
static const GUID IID_IServiceModule = { 0x05980504, 0xa2f2, 0x4b0f, 0x009b, 0x54, 0x51, 0x54, 0x1e, 0x05, 0x5c, 0xff };
#endif


interface IServiceModule : public IUnknownEx
{
	
	virtual bool StartService() = NULL;
	
	virtual bool ConcludeService() = NULL;
};

#ifdef _UNICODE
#define VER_ITCPSocketService INTERFACE_VERSION(1,1)
static const GUID IID_ITCPSocketService = { 0x0f8a5c14, 0xab92, 0x467c, 0xb6, 0x7b, 0x6d, 0x8a, 0xcf, 0x64, 0x52, 0xd7 };
#else
#define VER_ITCPSocketService INTERFACE_VERSION(1,1)
static const GUID IID_ITCPSocketService = { 0x709a4449, 0xad77, 0x4b3d, 0xb4, 0xd6, 0x8d, 0x0b, 0x28, 0x65, 0xec, 0xae };
#endif

interface ITCPSocketService : public IServiceModule
{
public:
	virtual bool SetServiceID(WORD wServiceID) = NULL;
	virtual bool SetTCPSocketEvent(IUnknownEx * pIUnknownEx) = NULL;

public:
	virtual bool CloseSocket() = NULL;
	virtual bool Connect(DWORD dwServerIP, WORD wPort) = NULL;
	virtual bool Connect(LPCTSTR szServerIP, WORD wPort) = NULL;
	virtual bool SendData(WORD wMainCmdID, WORD wSubCmdID) = NULL;
	virtual bool SendData(WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = NULL;
};

#ifdef _UNICODE
#define VER_ITCPNetworkEngine INTERFACE_VERSION(1,1)
static const GUID IID_ITCPNetworkEngine = { 0x9aa6931f, 0x417f, 0x43a7, 0x86, 0xab, 0x56, 0x10, 0xe4, 0x34, 0x1c, 0x17 };
#else
#define VER_ITCPNetworkEngine INTERFACE_VERSION(1,1)
static const GUID IID_ITCPNetworkEngine = { 0x7747f683, 0xc0da, 0x4588, 0x89, 0xcc, 0x15, 0x93, 0xac, 0xc0, 0x44, 0xc8 };
#endif


interface ITCPNetworkEngine : public IServiceModule
{
	
public:
	
	virtual WORD GetServicePort() = NULL;
	
	virtual WORD GetCurrentPort() = NULL;

	
public:
	
	virtual bool SetTCPNetworkEngineEvent(IUnknownEx * pIUnknownEx) = NULL;
	
	virtual bool SetServiceParameter(WORD wServicePort, WORD wMaxConnect, LPCTSTR  pszCompilation) = NULL;

	
public:
	
	virtual bool SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID) = NULL;
	
	virtual bool SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = NULL;
	
	virtual bool SendDataBatch(WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize, BYTE cbBatchMask) = NULL;

	
public:
	
	virtual bool CloseSocket(DWORD dwSocketID) = NULL;
	
	virtual bool ShutDownSocket(DWORD dwSocketID) = NULL;
	
	virtual bool AllowBatchSend(DWORD dwSocketID, bool bAllowBatch, BYTE cbBatchMask) = NULL;
};

#ifdef _UNICODE
#define VER_IAsynchronismEngineSink INTERFACE_VERSION(1,1)
static const GUID IID_IAsynchronismEngineSink = { 0x55215681, 0x858a, 0x46f6, 0x0084, 0xec, 0x84, 0x9e, 0xc8, 0x7d, 0x82, 0x35 };
#else
#define VER_IAsynchronismEngineSink INTERFACE_VERSION(1,1)
static const GUID IID_IAsynchronismEngineSink = { 0x2edf5c9e, 0x2cac, 0x461d, 0x00a7, 0x82, 0x2e, 0x2f, 0xe1, 0x91, 0x80, 0xf8 };
#endif


interface IAsynchronismEngineSink : public IUnknownEx
{
	
	virtual bool OnAsynchronismEngineStart() = NULL;
	
	virtual bool OnAsynchronismEngineConclude() = NULL;
	
	virtual bool OnAsynchronismEngineData(WORD wIdentifier, VOID * pData, WORD wDataSize) = NULL;
};

#ifdef _UNICODE
#define VER_ITCPNetworkEngineEvent INTERFACE_VERSION(1,1)
static const GUID IID_ITCPNetworkEngineEvent = { 0x9759ffb3, 0x5bec, 0x4199, 0x0091, 0xef, 0x49, 0x5b, 0xca, 0xdc, 0x00, 0x98 };
#else
#define VER_ITCPNetworkEngineEvent INTERFACE_VERSION(1,1)
static const GUID IID_ITCPNetworkEngineEvent = { 0xb7e6da53, 0xfca5, 0x4d90, 0x0085, 0x48, 0xfe, 0x05, 0xf6, 0xb4, 0xc0, 0xef };
#endif


interface ITCPNetworkEngineEvent : public IUnknownEx
{
	
public:
	
	virtual bool OnEventTCPNetworkBind(DWORD dwSocketID, DWORD dwClientAddr) = NULL;
	
	virtual bool OnEventTCPNetworkShut(DWORD dwSocketID, DWORD dwClientAddr, DWORD dwActiveTime) = NULL;
	
	virtual bool OnEventTCPNetworkRead(DWORD dwSocketID, TCP_Command Command, VOID * pData, WORD wDataSize) = NULL;
};

#ifdef _UNICODE
#define VER_IAsynchronismEngine INTERFACE_VERSION(1,1)
static const GUID IID_IAsynchronismEngine = { 0xe03ad33d, 0xb285, 0x48ea, 0x86, 0x70, 0x0a, 0x95, 0x55, 0x92, 0x07, 0xe1 };
#else
#define VER_IAsynchronismEngine INTERFACE_VERSION(1,1)
static const GUID IID_IAsynchronismEngine = { 0xc7a13074, 0x75c5, 0x4b8e, 0xb5, 0x4b, 0xee, 0x0e, 0xec, 0xfe, 0xb9, 0xeb };
#endif


interface IAsynchronismEngine : public IServiceModule
{
	
public:
	
	virtual bool GetBurthenInfo(tagBurthenInfo & BurthenInfo) = NULL;
	
	virtual bool SetAsynchronismSink(IUnknownEx * pIUnknownEx) = NULL;

	
public:
	
	virtual bool PostAsynchronismData(WORD wIdentifier, VOID * pData, WORD wDataSize) = NULL;
	
	virtual bool PostAsynchronismData(WORD wIdentifier, tagDataBuffer DataBuffer[], WORD wDataCount) = NULL;
};

#ifdef _UNICODE
#define VER_IAttemperEngine INTERFACE_VERSION(1,1)
static const GUID IID_IAttemperEngine = { 0x4d5d2424, 0x40fd, 0x4747, 0x86, 0xd8, 0x8f, 0xca, 0x6b, 0x96, 0xea, 0x0b };
#else
#define VER_IAttemperEngine INTERFACE_VERSION(1,1)
static const GUID IID_IAttemperEngine={0x0b070b2c,0x9d72,0x42d2,0xa5,0x70,0xba,0x2c,0xbf,0x6f,0xbb,0x1c};
#endif


interface IAttemperEngine : public IServiceModule
{
	
public:
	
	virtual bool SetNetworkEngine(IUnknownEx * pIUnknownEx) = NULL;
	
	virtual bool SetAttemperEngineSink(IUnknownEx * pIUnknownEx) = NULL;
};

#ifdef _UNICODE
#define VER_IAttemperEngineSink INTERFACE_VERSION(1,1)
static const GUID IID_IAttemperEngineSink = { 0x133d1f30, 0x54ce, 0x4360, 0x0084, 0x50, 0x87, 0x29, 0xe0, 0x95, 0xaa, 0xbb };
#else
#define VER_IAttemperEngineSink INTERFACE_VERSION(1,1)
static const GUID IID_IAttemperEngineSink = { 0x831b9001, 0x4450, 0x45dd, 0x0091, 0x37, 0x0d, 0x26, 0x16, 0xe3, 0x75, 0x32 };
#endif


interface IAttemperEngineSink : public IUnknownEx
{
	
public:
	
	virtual bool OnAttemperEngineStart(IUnknownEx * pIUnknownEx) = NULL;
	
	virtual bool OnAttemperEngineConclude(IUnknownEx * pIUnknownEx) = NULL;
	
public:
	
	virtual bool OnEventTCPNetworkBind(DWORD dwClientAddr, DWORD dwSocketID) = NULL;
	
	virtual bool OnEventTCPNetworkShut(DWORD dwClientAddr, DWORD dwActiveTime, DWORD dwSocketID) = NULL;
	
	virtual bool OnEventTCPNetworkRead(TCP_Command Command, VOID * pData, WORD wDataSize, DWORD dwSocketID) = NULL;
};

DECLARE_MODULE_HELPER(TCPSocketService, NET_ENGINE_DLL_NAME, "CreateTCPSocketService")
DECLARE_MODULE_HELPER(TCPNetworkEngine, NET_ENGINE_DLL_NAME, "CreateTCPNetworkEngine")
DECLARE_MODULE_HELPER(AsynchronismEngine, NET_ENGINE_DLL_NAME, "CreateAsynchronismEngine")
DECLARE_MODULE_HELPER(AttemperEngine, NET_ENGINE_DLL_NAME, "CreateAttemperEngine")
