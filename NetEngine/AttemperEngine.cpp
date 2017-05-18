#include "StdAfx.h"
#include "AttemperEngine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




CAttemperEngine::CAttemperEngine()
{
	
	m_pITCPNetworkEngine=NULL;
	m_pIAttemperEngineSink=NULL;

	
	ZeroMemory(m_cbBuffer,sizeof(m_cbBuffer));

	return;
}


CAttemperEngine::~CAttemperEngine()
{
}

VOID * CAttemperEngine::QueryInterface(REFGUID Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(IAttemperEngine,Guid,dwQueryVer);
	QUERYINTERFACE(ITCPNetworkEngineEvent,Guid,dwQueryVer);
	QUERYINTERFACE(IAsynchronismEngineSink,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(IAttemperEngine,Guid,dwQueryVer);
	return NULL;
}

bool CAttemperEngine::StartService()
{
	
	ASSERT((m_pITCPNetworkEngine!=NULL)&&(m_pIAttemperEngineSink!=NULL));
	if ((m_pITCPNetworkEngine==NULL)||(m_pIAttemperEngineSink==NULL)) return false;

	
	IUnknownEx * pIAsynchronismEngineSink=QUERY_ME_INTERFACE(IUnknownEx);
	if (m_AsynchronismEngine.SetAsynchronismSink(pIAsynchronismEngineSink)==false)
	{
		ASSERT(FALSE);
		return false;
	}

	
	if (m_AsynchronismEngine.StartService()==false) return false;

	return true;
}


bool CAttemperEngine::ConcludeService()
{
	
	m_AsynchronismEngine.ConcludeService();

	return true;
}


bool CAttemperEngine::SetNetworkEngine(IUnknownEx * pIUnknownEx)
{
	
	if (pIUnknownEx!=NULL)
	{
		
		ASSERT(QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,ITCPNetworkEngine)!=NULL);
		m_pITCPNetworkEngine=QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,ITCPNetworkEngine);

		
		if (m_pITCPNetworkEngine==NULL) return false;
	}
	else m_pITCPNetworkEngine=NULL;

	return true;
}


bool CAttemperEngine::SetAttemperEngineSink(IUnknownEx * pIUnknownEx)
{
	
	ASSERT(QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,IAttemperEngineSink)!=NULL);
	m_pIAttemperEngineSink=QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,IAttemperEngineSink);

	
	if (m_pIAttemperEngineSink==NULL)
	{
		ASSERT(FALSE);
		return false;
	}

	return true;
}

bool CAttemperEngine::OnEventTCPNetworkBind(DWORD dwSocketID, DWORD dwClientAddr)
{
	
	CWHDataLocker ThreadLock(m_CriticalLocker);
	TCPNetworkAcceptEvent * pAcceptEvent=(TCPNetworkAcceptEvent *)m_cbBuffer;
	
	
	pAcceptEvent->dwSocketID=dwSocketID;
	pAcceptEvent->dwClientAddr=dwClientAddr;

	
	return m_AsynchronismEngine.PostAsynchronismData(EVENT_TCP_NETWORK_ACCEPT,m_cbBuffer,sizeof(TCPNetworkAcceptEvent));
}


bool CAttemperEngine::OnEventTCPNetworkShut(DWORD dwSocketID, DWORD dwClientAddr, DWORD dwActiveTime)
{
	
	CWHDataLocker ThreadLock(m_CriticalLocker);
	TCPNetworkShutEvent * pCloseEvent=(TCPNetworkShutEvent *)m_cbBuffer;
	
	
	pCloseEvent->dwSocketID=dwSocketID;
	pCloseEvent->dwClientAddr=dwClientAddr;
	pCloseEvent->dwActiveTime=dwActiveTime;

	
	return m_AsynchronismEngine.PostAsynchronismData(EVENT_TCP_NETWORK_SHUT,m_cbBuffer,sizeof(TCPNetworkShutEvent));
}


bool CAttemperEngine::OnEventTCPNetworkRead(DWORD dwSocketID, TCP_Command Command, VOID * pData, WORD wDataSize)
{
	
	ASSERT((wDataSize+sizeof(TCPNetworkReadEvent))<=MAX_ASYNCHRONISM_DATA);
	if ((wDataSize+sizeof(TCPNetworkReadEvent))>MAX_ASYNCHRONISM_DATA) return false;

	
	CWHDataLocker ThreadLock(m_CriticalLocker);
	TCPNetworkReadEvent * pReadEvent=(TCPNetworkReadEvent *)m_cbBuffer;
	
	
	pReadEvent->Command=Command;
	pReadEvent->wDataSize=wDataSize;
	pReadEvent->dwSocketID=dwSocketID;

	
	if (wDataSize>0)
	{
		ASSERT(pData!=NULL);
		CopyMemory(m_cbBuffer+sizeof(TCPNetworkReadEvent),pData,wDataSize);
	}

	
	return m_AsynchronismEngine.PostAsynchronismData(EVENT_TCP_NETWORK_READ,m_cbBuffer,sizeof(TCPNetworkReadEvent)+wDataSize);
}


bool CAttemperEngine::OnAsynchronismEngineStart()
{
	
	ASSERT(m_pIAttemperEngineSink!=NULL);
	if (m_pIAttemperEngineSink==NULL) return false;

	
	if (m_pIAttemperEngineSink->OnAttemperEngineStart(QUERY_ME_INTERFACE(IUnknownEx))==false)
	{
		ASSERT(FALSE);
		return false;
	}

	return true;
}


bool CAttemperEngine::OnAsynchronismEngineConclude()
{
	
	ASSERT(m_pIAttemperEngineSink!=NULL);
	if (m_pIAttemperEngineSink==NULL) return false;

	
	if (m_pIAttemperEngineSink->OnAttemperEngineConclude(QUERY_ME_INTERFACE(IUnknownEx))==false)
	{
		ASSERT(FALSE);
		return false;
	}

	return true;
}


bool CAttemperEngine::OnAsynchronismEngineData(WORD wIdentifier, VOID * pData, WORD wDataSize)
{
	
	ASSERT(m_pITCPNetworkEngine!=NULL);
	ASSERT(m_pIAttemperEngineSink!=NULL);

	
	switch (wIdentifier)
	{
	case EVENT_TCP_NETWORK_ACCEPT:	
		{
			
			ASSERT(wDataSize==sizeof(TCPNetworkAcceptEvent));
			if (wDataSize!=sizeof(TCPNetworkAcceptEvent)) return false;

			
			bool bSuccess=false;
			TCPNetworkAcceptEvent * pAcceptEvent=(TCPNetworkAcceptEvent *)pData;

			try
			{ 
				bSuccess=m_pIAttemperEngineSink->OnEventTCPNetworkBind(pAcceptEvent->dwClientAddr,pAcceptEvent->dwSocketID);
			}
			catch (...)	{ }

			
			if (bSuccess==false) m_pITCPNetworkEngine->CloseSocket(pAcceptEvent->dwSocketID);

			return true;
		}
	case EVENT_TCP_NETWORK_READ:	
		{
			
			TCPNetworkReadEvent * pReadEvent=(TCPNetworkReadEvent *)pData;

			
			ASSERT(wDataSize>=sizeof(TCPNetworkReadEvent));
			ASSERT(wDataSize==(sizeof(TCPNetworkReadEvent)+pReadEvent->wDataSize));

			
			if (wDataSize<sizeof(TCPNetworkReadEvent))
			{
				m_pITCPNetworkEngine->CloseSocket(pReadEvent->dwSocketID);
				return false;
			}

			
			if (wDataSize!=(sizeof(TCPNetworkReadEvent)+pReadEvent->wDataSize))
			{
				m_pITCPNetworkEngine->CloseSocket(pReadEvent->dwSocketID);
				return false;
			}

			
			bool bSuccess=false;
			try
			{ 
				bSuccess=m_pIAttemperEngineSink->OnEventTCPNetworkRead(pReadEvent->Command,pReadEvent+1,pReadEvent->wDataSize,pReadEvent->dwSocketID);
			}
			catch (...)	{ }

			
			if (bSuccess==false) m_pITCPNetworkEngine->CloseSocket(pReadEvent->dwSocketID);

			return true;
		}
	case EVENT_TCP_NETWORK_SHUT:	
		{
			
			ASSERT(wDataSize==sizeof(TCPNetworkShutEvent));
			if (wDataSize!=sizeof(TCPNetworkShutEvent)) return false;

			
			TCPNetworkShutEvent * pCloseEvent=(TCPNetworkShutEvent *)pData;
			m_pIAttemperEngineSink->OnEventTCPNetworkShut(pCloseEvent->dwClientAddr,pCloseEvent->dwActiveTime,pCloseEvent->dwSocketID);

			return true;
		}
	}

	
	return true; 
}




DECLARE_CREATE_MODULE(AttemperEngine);


