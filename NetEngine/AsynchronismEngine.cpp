#include "StdAfx.h"
#include "AsynchronismEngine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




CAsynchronismThread::CAsynchronismThread()
{
	
	m_hCompletionPort=NULL;
	m_pIAsynchronismEngineSink=NULL;

	
	ZeroMemory(m_cbBuffer,sizeof(m_cbBuffer));

	return;
}


CAsynchronismThread::~CAsynchronismThread()
{
}


VOID CAsynchronismThread::SetCompletionPort(HANDLE hCompletionPort) 
{ 
	
	m_hCompletionPort=hCompletionPort; 

	return;
}


VOID CAsynchronismThread::SetAsynchronismEngineSink(IAsynchronismEngineSink * pIAsynchronismEngineSink)
{
	
	m_pIAsynchronismEngineSink=pIAsynchronismEngineSink;

	return;
}


bool CAsynchronismThread::OnEventThreadRun()
{
	
	ASSERT(m_hCompletionPort!=NULL);
	ASSERT(m_pIAsynchronismEngineSink!=NULL);

	
	DWORD dwThancferred=0;
	OVERLAPPED * pOverLapped=NULL;
	CAsynchronismEngine * pAsynchronismEngine=NULL;

	
	if (GetQueuedCompletionStatus(m_hCompletionPort,&dwThancferred,(PULONG_PTR)&pAsynchronismEngine,&pOverLapped,INFINITE))
	{
		
		if (pAsynchronismEngine==NULL) return false;

		
		CWHDataLocker ThreadLock(pAsynchronismEngine->m_CriticalSection);

		
		tagDataHead DataHead;
		pAsynchronismEngine->m_DataQueue.DistillData(DataHead,m_cbBuffer,sizeof(m_cbBuffer));

		
		ThreadLock.UnLock();

		
		try
		{
			m_pIAsynchronismEngineSink->OnAsynchronismEngineData(DataHead.wIdentifier,m_cbBuffer,DataHead.wDataSize);
		}
		catch (...)
		{
			
			TCHAR szDescribe[256]=TEXT("");
			_sntprintf(szDescribe,CountArray(szDescribe),TEXT("CAsynchronismEngine::OnAsynchronismEngineData [ wIdentifier=%d wDataSize=%ld ]"),
				DataHead.wIdentifier,DataHead.wDataSize);

			

		}

		return true;
	}

	return false;
}


bool CAsynchronismThread::OnEventThreadStrat()
{
	
	ASSERT(m_pIAsynchronismEngineSink!=NULL);
	bool bSuccess=m_pIAsynchronismEngineSink->OnAsynchronismEngineStart();

	
	CAsynchronismEngine * pAsynchronismEngine=CONTAINING_RECORD(this,CAsynchronismEngine,m_AsynchronismThread);
	pAsynchronismEngine->m_bService=true;

	return bSuccess;
}


bool CAsynchronismThread::OnEventThreadConclude()
{
	
	CAsynchronismEngine * pAsynchronismEngine=CONTAINING_RECORD(this,CAsynchronismEngine,m_AsynchronismThread);
	pAsynchronismEngine->m_bService=false;

	
	ASSERT(m_pIAsynchronismEngineSink!=NULL);
	bool bSuccess=m_pIAsynchronismEngineSink->OnAsynchronismEngineConclude();

	return bSuccess;
}




CAsynchronismEngine::CAsynchronismEngine()
{
	
	m_bService=false;
	m_hCompletionPort=NULL;
	m_pIAsynchronismEngineSink=NULL;

	return;
}


CAsynchronismEngine::~CAsynchronismEngine()
{
	
	ConcludeService();

	return;
}


VOID * CAsynchronismEngine::QueryInterface(REFGUID Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(IServiceModule,Guid,dwQueryVer);
	QUERYINTERFACE(IAsynchronismEngine,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(IAsynchronismEngine,Guid,dwQueryVer);
	return NULL;
}


bool CAsynchronismEngine::GetBurthenInfo(tagBurthenInfo & BurthenInfo)
{
	CWHDataLocker lock(m_CriticalSection);
	m_DataQueue.GetBurthenInfo(BurthenInfo);

	return true;
}



bool CAsynchronismEngine::SetAsynchronismSink(IUnknownEx * pIUnknownEx)
{
	
	ASSERT(m_bService==false);
	if (m_bService==true) return false;

	
	m_pIAsynchronismEngineSink=QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,IAsynchronismEngineSink);

	
	if (m_pIAsynchronismEngineSink==NULL)
	{
		ASSERT(FALSE);
		return false;
	}

	return true;
}


bool CAsynchronismEngine::PostAsynchronismData(WORD wIdentifier, VOID * pData, WORD wDataSize)
{
	
	ASSERT((m_bService==true)&&(m_hCompletionPort!=NULL));
	if ((m_hCompletionPort==NULL)||(m_bService==false)) return false;

	
	CWHDataLocker ThreadLock(m_CriticalSection);
	if (m_DataQueue.InsertData(wIdentifier,pData,wDataSize)==false)
	{
		ASSERT(FALSE);
		return false;
	}

	
	PostQueuedCompletionStatus(m_hCompletionPort,wDataSize,(ULONG_PTR)this,NULL);

	return true;
}


bool CAsynchronismEngine::PostAsynchronismData(WORD wIdentifier, tagDataBuffer DataBuffer[], WORD wDataCount)
{
	ASSERT(wDataCount > 0);
	if (wDataCount == 0) return false;

	
	CWHDataLocker lcok(m_CriticalSection);
	if (m_DataQueue.InsertData(wIdentifier, DataBuffer, wDataCount) == false) return false;

	for (WORD i = 0; i < wDataCount; i++)
	{
		PostQueuedCompletionStatus(m_hCompletionPort,0,(ULONG_PTR)this,NULL);
	}

	return true;
}


bool CAsynchronismEngine::StartService()
{
	
	ASSERT(m_bService==false);
	if (m_bService==true) return false;

	
	m_hCompletionPort=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,1);
	if (m_hCompletionPort==NULL) return false;

	
	m_AsynchronismThread.SetCompletionPort(m_hCompletionPort);
	m_AsynchronismThread.SetAsynchronismEngineSink(m_pIAsynchronismEngineSink);

	
	if (m_AsynchronismThread.StartThread()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	return true;
}


bool CAsynchronismEngine::ConcludeService()
{
	
	m_bService=false;

	
	if (m_hCompletionPort!=NULL) 
	{
		PostQueuedCompletionStatus(m_hCompletionPort,0,NULL,NULL);
	}

	
	m_AsynchronismThread.ConcludeThread(INFINITE);

	
	if (m_hCompletionPort!=NULL) CloseHandle(m_hCompletionPort);

	
	m_hCompletionPort=NULL;
	m_pIAsynchronismEngineSink=NULL;

	
	m_AsynchronismThread.SetCompletionPort(NULL);
	m_AsynchronismThread.SetAsynchronismEngineSink(NULL);

	
	m_DataQueue.RemoveData(false);

	return true;
}




DECLARE_CREATE_MODULE(AsynchronismEngine);


