#ifndef ATTEMPER_ENGINE_HEAD_FILE
#define ATTEMPER_ENGINE_HEAD_FILE

#pragma once

#include "NetEngineHead.h"
#include "AsynchronismEngine.h"




class CAttemperEngine : public IAttemperEngine, public IAsynchronismEngineSink, public ITCPNetworkEngineEvent
{
	
protected:
	CCriticalSection				m_CriticalLocker;					
	CAsynchronismEngine				m_AsynchronismEngine;				

	
protected:
	ITCPNetworkEngine *				m_pITCPNetworkEngine;				
	IAttemperEngineSink	*			m_pIAttemperEngineSink;				

	
protected:
	BYTE							m_cbBuffer[MAX_ASYNCHRONISM_DATA];	

	
public:
	
	CAttemperEngine();
	
	virtual ~CAttemperEngine();

	
public:
	
	virtual VOID Release() { delete this; }
	
	virtual VOID * QueryInterface(REFGUID Guid, DWORD dwQueryVer);

	
public:
	
	virtual bool StartService();
	
	virtual bool ConcludeService();

	
public:
	
	virtual bool SetNetworkEngine(IUnknownEx * pIUnknownEx);
	
	virtual bool SetAttemperEngineSink(IUnknownEx * pIUnknownEx);
public:
	
	virtual bool OnEventTCPNetworkBind(DWORD dwSocketID, DWORD dwClientAddr);
	
	virtual bool OnEventTCPNetworkShut(DWORD dwSocketID, DWORD dwClientAddr, DWORD dwActiveTime);
	
	virtual bool OnEventTCPNetworkRead(DWORD dwSocketID, TCP_Command Command, VOID * pData, WORD wDataSize);

	
public:
	
	virtual bool OnAsynchronismEngineStart();
	
	virtual bool OnAsynchronismEngineConclude();
	
	virtual bool OnAsynchronismEngineData(WORD wIdentifier, VOID * pData, WORD wDataSize);
};



#endif
