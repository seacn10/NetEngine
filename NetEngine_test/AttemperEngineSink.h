#ifndef ATTEMPER_ENGINE_SINK_HEAD_FILE
#define ATTEMPER_ENGINE_SINK_HEAD_FILE

#pragma once

#include "Stdafx.h"
#include "..\NetEngine\NetEngineHead.h"

typedef CWHArray<WORD> CWordArrayTemplate;

class CAttemperEngineSink : public IAttemperEngineSink
{
	friend class CServiceUnits;

protected:
	WORD							m_wCollectItem;						
	CWordArrayTemplate				m_WaitCollectItemArray;					
	
protected:
	ITCPNetworkEngine *				m_pITCPNetworkEngine;				

public:
	CAttemperEngineSink();
	virtual ~CAttemperEngineSink();

public:
	
	virtual VOID Release() { return; }
	virtual VOID * QueryInterface(REFGUID Guid, DWORD dwQueryVer);
	
public:
	
	virtual bool OnAttemperEngineStart(IUnknownEx * pIUnknownEx);
	virtual bool OnAttemperEngineConclude(IUnknownEx * pIUnknownEx);
		
public:
	
	virtual bool OnEventTCPNetworkBind(DWORD dwClientAddr, DWORD dwSocketID);
	virtual bool OnEventTCPNetworkShut(DWORD dwClientAddr, DWORD dwActiveTime, DWORD dwSocketID);
	virtual bool OnEventTCPNetworkRead(TCP_Command Command, VOID * pData, WORD wDataSize, DWORD dwSocketID);
};

#endif