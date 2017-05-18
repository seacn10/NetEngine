#ifndef TCP_NETWORK_ENGINE_HEAD_FILE
#define TCP_NETWORK_ENGINE_HEAD_FILE

#pragma once

#include "AsynchronismEngine.h"




enum enOperationType
{
	enOperationType_Send,			
	enOperationType_Recv,			
};




class COverLappedRecv;
class COverLappedSend;
class CTCPNetworkItem;
class CTCPNetworkEngine;


typedef class CWHArray<COverLappedSend *>	COverLappedSendPtr;
typedef class CWHArray<COverLappedRecv *>	COverLappedRecvPtr;





interface ITCPNetworkItemSink
{
	
	virtual bool OnEventSocketBind(CTCPNetworkItem * pTCPNetworkItem)=NULL;
	
	virtual bool OnEventSocketShut(CTCPNetworkItem * pTCPNetworkItem)=NULL;
	
	virtual bool OnEventSocketRead(TCP_Command Command, VOID * pData, WORD wDataSize, CTCPNetworkItem * pTCPNetworkItem)=NULL;
};




class COverLapped
{
	
public:
	WSABUF							m_WSABuffer;						
	OVERLAPPED						m_OverLapped;						
	const enOperationType			m_OperationType;					

	
public:
	
	COverLapped(enOperationType OperationType);
	
	virtual ~COverLapped();

	
public:
	
	enOperationType GetOperationType() { return m_OperationType; }
};




class COverLappedSend : public COverLapped
{
	
public:
	BYTE							m_cbBuffer[SOCKET_TCP_BUFFER];			

	
public:
	
	COverLappedSend();
	
	virtual ~COverLappedSend();
};




class COverLappedRecv : public COverLapped
{
	
public:
	
	COverLappedRecv();
	
	virtual ~COverLappedRecv();
};




class CTCPNetworkItem
{
	
	friend class CTCPNetworkEngine;

	
protected:
	DWORD							m_dwClientIP;						
	DWORD							m_dwActiveTime;						

	
protected:
	WORD							m_wIndex;							
	WORD							m_wRountID;							
	WORD							m_wSurvivalTime;					
	SOCKET							m_hSocketHandle;					
	CCriticalSection				m_CriticalSection;					
	ITCPNetworkItemSink *			m_pITCPNetworkItemSink;				

	
protected:
	bool							m_bSendIng;							
	bool							m_bRecvIng;							
	bool							m_bShutDown;						
	bool							m_bAllowBatch;						
	BYTE							m_bBatchMask;						

	
protected:
	WORD							m_wRecvSize;						
	BYTE							m_cbRecvBuf[SOCKET_TCP_BUFFER*5];		

	
protected:
	DWORD							m_dwSendTickCount;					
	DWORD							m_dwRecvTickCount;					
	DWORD							m_dwSendPacketCount;				
	DWORD							m_dwRecvPacketCount;				

	
protected:
	BYTE							m_cbSendRound;						
	BYTE							m_cbRecvRound;						
	DWORD							m_dwSendXorKey;						
	DWORD							m_dwRecvXorKey;						

	
protected:
	COverLappedRecv					m_OverLappedRecv;					
	COverLappedSendPtr				m_OverLappedSendActive;				

	
protected:
	CCriticalSection				m_SendBufferSection;				
	COverLappedSendPtr				m_OverLappedSendBuffer;				

	
public:
	
	CTCPNetworkItem(WORD wIndex, ITCPNetworkItemSink * pITCPNetworkItemSink);
	
	virtual ~CTCPNetworkItem();

	
public:
	
	inline WORD GetIndex() { return m_wIndex; }
	
	inline WORD GetRountID() { return m_wRountID; }
	
	inline DWORD GetIdentifierID() { return MAKELONG(m_wIndex,m_wRountID); }

	
public:
	
	inline DWORD GetClientIP() { return m_dwClientIP; }
	
	inline DWORD GetActiveTime() { return m_dwActiveTime; }
	
	inline DWORD GetSendTickCount() { return m_dwSendTickCount; }
	
	inline DWORD GetRecvTickCount() { return m_dwRecvTickCount; }
	
	inline DWORD GetSendPacketCount() { return m_dwSendPacketCount; }
	
	inline DWORD GetRecvPacketCount() { return m_dwRecvPacketCount; }
	
	inline CCriticalSection & GetCriticalSection() { return m_CriticalSection; }

	
public:
	
	inline bool IsAllowBatch() { return m_bAllowBatch; }
	
	inline bool IsAllowSendData() { return m_dwRecvPacketCount>0L; }
	
	inline bool IsValidSocket() { return (m_hSocketHandle!=INVALID_SOCKET); }
	
	inline BYTE GetBatchMask() { return m_bBatchMask; }

	
public:
	
	DWORD Attach(SOCKET hSocket, DWORD dwClientIP);
	
	DWORD ResumeData();

	
public:
	
	bool SendData(WORD wMainCmdID, WORD wSubCmdID, WORD wRountID);
	
	bool SendData(VOID * pData, WORD wDataSize, WORD wMainCmdID, WORD wSubCmdID, WORD wRountID);
	
	bool RecvData();
	
	bool CloseSocket(WORD wRountID);

	
public:
	
	bool ShutDownSocket(WORD wRountID);
	
	bool AllowBatchSend(WORD wRountID, bool bAllowBatch,BYTE cbBatchMask);

	
public:
	
	bool OnSendCompleted(COverLappedSend * pOverLappedSend, DWORD dwThancferred);
	
	bool OnRecvCompleted(COverLappedRecv * pOverLappedRecv, DWORD dwThancferred);
	
	bool OnCloseCompleted();

	
private:
	
	WORD EncryptBuffer(BYTE pcbDataBuffer[], WORD wDataSize, WORD wBufferSize);
	
	WORD CrevasseBuffer(BYTE pcbDataBuffer[], WORD wDataSize);

	 
	unsigned short mappedBuffer(void* pData, WORD wDataSize);
    
    
	unsigned short unMappedBuffer(void* pData, WORD wDataSize);

	
private:
	
	inline WORD SeedRandMap(WORD wSeed);
	
	inline BYTE MapSendByte(BYTE cbData);
	
	inline BYTE MapRecvByte(BYTE cbData);

	
private:
	
	inline bool SendVerdict(WORD wRountID);
	
	inline COverLappedSend * GetSendOverLapped(WORD wPacketSize);
};




class CTCPNetworkThreadReadWrite : public CWHThread
{
	
protected:
	HANDLE							m_hCompletionPort;					

	
public:
	
	CTCPNetworkThreadReadWrite();
	
	virtual ~CTCPNetworkThreadReadWrite();

	
public:
	
	bool InitThread(HANDLE hCompletionPort);

	
private:
	
	virtual bool OnEventThreadRun();
};




class CTCPNetworkThreadAccept : public CWHThread
{
	
protected:
	SOCKET							m_hListenSocket;					
	HANDLE							m_hCompletionPort;					
	CTCPNetworkEngine *				m_pTCPNetworkEngine;				

	
public:
	
	CTCPNetworkThreadAccept();
	
	virtual ~CTCPNetworkThreadAccept();

	
public:
	
	bool InitThread(HANDLE hCompletionPort, SOCKET hListenSocket, CTCPNetworkEngine * pNetworkEngine);

	
private:
	
	virtual bool OnEventThreadRun();
};




class CTCPNetworkThreadDetect : public CWHThread
{
	
protected:
	DWORD							m_dwPileTime;						
	DWORD							m_dwDetectTime;						
	CTCPNetworkEngine *				m_pTCPNetworkEngine;				

	
public:
	
	CTCPNetworkThreadDetect();
	
	virtual ~CTCPNetworkThreadDetect();

	
public:
	
	bool InitThread(CTCPNetworkEngine * pNetworkEngine, DWORD dwDetectTime);

	
private:
	
	virtual bool OnEventThreadRun();
};




typedef CWHArray<CTCPNetworkItem *> CTCPNetworkItemPtrArray;
typedef CWHArray<CTCPNetworkThreadReadWrite *> CTCPNetworkThreadRWPtrArray;


class CTCPNetworkEngine : public ITCPNetworkEngine, public IAsynchronismEngineSink, public ITCPNetworkItemSink
{
	friend class CTCPNetworkThreadDetect;
	friend class CTCPNetworkThreadAccept;
	friend class CTCPNetworkThreadReadWrite;

	
protected:
	bool							m_bValidate;						
	bool							m_bNormalRun;						

	
protected:
	bool							m_bService;							
	BYTE							m_cbBuffer[MAX_ASYNCHRONISM_DATA];	

	
protected:
	WORD							m_wMaxConnect;						
	WORD							m_wServicePort;						
	DWORD							m_dwDetectTime;						

	
protected:
	SOCKET							m_hServerSocket;					
	HANDLE							m_hCompletionPort;					
	ITCPNetworkEngineEvent *		m_pITCPNetworkEngineEvent;			

	
protected:
	CCriticalSection				m_ItemLocked;						
	CTCPNetworkItemPtrArray			m_NetworkItemBuffer;				
	CTCPNetworkItemPtrArray			m_NetworkItemActive;				
	CTCPNetworkItemPtrArray			m_NetworkItemStorage;				
	CTCPNetworkItemPtrArray			m_TempNetworkItemArray;				

	
protected:
	CCriticalSection				m_BufferLocked;						
	CAsynchronismEngine				m_AsynchronismEngine;				
	CTCPNetworkThreadDetect			m_SocketDetectThread;				
	CTCPNetworkThreadAccept			m_SocketAcceptThread;				
	CTCPNetworkThreadRWPtrArray		m_SocketRWThreadArray;				

	
public:
	
	CTCPNetworkEngine();
	
	virtual ~CTCPNetworkEngine();

	
public:
	
	virtual VOID Release() { delete this; }
	
	virtual VOID * QueryInterface(REFGUID Guid, DWORD dwQueryVer);

	
public:
	
	virtual bool StartService();
	
	virtual bool ConcludeService();

	
public:
	
	virtual WORD GetServicePort();
	
	virtual WORD GetCurrentPort();

	
public:
	
	virtual bool SetTCPNetworkEngineEvent(IUnknownEx * pIUnknownEx);
	
	virtual bool SetServiceParameter(WORD wServicePort, WORD wMaxConnect, LPCTSTR  pszCompilation);
	
	
public:
	
	virtual bool SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID);
	
	virtual bool SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize);
	
	virtual bool SendDataBatch(WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize, BYTE cbBatchMask);

	
public:
	
	virtual bool CloseSocket(DWORD dwSocketID);
	
	virtual bool ShutDownSocket(DWORD dwSocketID);
	
	virtual bool AllowBatchSend(DWORD dwSocketID, bool bAllowBatch, BYTE cbBatchMask);

	
public:
	
	virtual bool OnAsynchronismEngineStart() { return true; }
	
	virtual bool OnAsynchronismEngineConclude() { return true; }
	
	virtual bool OnAsynchronismEngineData(WORD wIdentifier, VOID * pData, WORD wDataSize);

	
public:
	
	virtual bool OnEventSocketBind(CTCPNetworkItem * pTCPNetworkItem);
	
	virtual bool OnEventSocketShut(CTCPNetworkItem * pTCPNetworkItem);
	
	virtual bool OnEventSocketRead(TCP_Command Command, VOID * pData, WORD wDataSize, CTCPNetworkItem * pTCPNetworkItem);

	
private:
	
	bool DetectSocket();
	
	bool WebAttestation();

	
protected:
	
	CTCPNetworkItem * ActiveNetworkItem();
	
	CTCPNetworkItem * GetNetworkItem(WORD wIndex);
	
	bool FreeNetworkItem(CTCPNetworkItem * pTCPNetworkItem);
};



#endif
