#ifndef ASYNCHRONISM_ENGINE_HEAD_FILE
#define ASYNCHRONISM_ENGINE_HEAD_FILE

#pragma once

#include "NetEngineHead.h"

class CAsynchronismEngine;

class CAsynchronismThread : public CWHThread
{
	
protected:
	HANDLE							m_hCompletionPort;					
	IAsynchronismEngineSink	*		m_pIAsynchronismEngineSink;			
private:
	BYTE							m_cbBuffer[MAX_ASYNCHRONISM_DATA];	

public:
	
	CAsynchronismThread();
	
	virtual ~CAsynchronismThread();

	
public:
	
	VOID SetCompletionPort(HANDLE hCompletionPort);
	
	VOID SetAsynchronismEngineSink(IAsynchronismEngineSink * pIAsynchronismEngineSink);

	
public:
	
	virtual bool OnEventThreadRun();
	
	virtual bool OnEventThreadStrat();
	
	virtual bool OnEventThreadConclude();
};
class CAsynchronismEngine : public IAsynchronismEngine
{
	
	friend class CAsynchronismThread;

	
protected:
	bool							m_bService;							
	HANDLE							m_hCompletionPort;					
	IAsynchronismEngineSink *		m_pIAsynchronismEngineSink;			

	
protected:
	CWHDataQueue					m_DataQueue;						
	CCriticalSection				m_CriticalSection;					
	CAsynchronismThread				m_AsynchronismThread;				

	
public:
	
	CAsynchronismEngine();
	
	virtual ~CAsynchronismEngine();

	
public:
	
	virtual VOID Release() { delete this; }
	virtual VOID * QueryInterface(REFGUID Guid, DWORD dwQueryVer);

public:
	
	virtual bool StartService();
	
	virtual bool ConcludeService();

public:
	
	virtual bool GetBurthenInfo(tagBurthenInfo & BurthenInfo);
	
	virtual bool SetAsynchronismSink(IUnknownEx * pIUnknownEx);
	
	virtual bool PostAsynchronismData(WORD wIdentifier, VOID * pData, WORD wDataSize);
	
	virtual bool PostAsynchronismData(WORD wIdentifier, tagDataBuffer DataBuffer[], WORD wDataCount);
};



#endif
