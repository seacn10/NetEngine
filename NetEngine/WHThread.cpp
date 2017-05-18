#include "StdAfx.h"
#include "WHThread.h"





struct tagThreadParameter
{
	bool							bSuccess;							
	HANDLE							hEventFinish;						
	CWHThread	*					pServiceThread;						
};




CWHThread::CWHThread()
{
	
	m_bRun=false;
	m_uThreadID=0;
	m_hThreadHandle=NULL;

	return;
}


CWHThread::~CWHThread()
{
	
	ConcludeThread(INFINITE);

	return;
}


bool CWHThread::IsRuning()
{
	
	if (m_hThreadHandle==NULL) return false;
	if (WaitForSingleObject(m_hThreadHandle,0)!=WAIT_TIMEOUT) return false;

	return true;
}


bool CWHThread::StartThread()
{
	
	ASSERT(IsRuning()==false);
	if (IsRuning()==true) return false;

	
	if (m_hThreadHandle!=NULL) 
	{
		
		CloseHandle(m_hThreadHandle);

		
		m_uThreadID=0;
		m_hThreadHandle=NULL;
	}

	
	tagThreadParameter ThreadParameter;
	ZeroMemory(&ThreadParameter,sizeof(ThreadParameter));

	
	ThreadParameter.bSuccess=false;
	ThreadParameter.pServiceThread=this;
	ThreadParameter.hEventFinish=CreateEvent(NULL,FALSE,FALSE,NULL);

	
	ASSERT(ThreadParameter.hEventFinish!=NULL);
	if (ThreadParameter.hEventFinish==NULL) return false;

	
	m_bRun=true;
	m_hThreadHandle=(HANDLE)::_beginthreadex(NULL,0,ThreadFunction,&ThreadParameter,0,&m_uThreadID);

	
	if (m_hThreadHandle==INVALID_HANDLE_VALUE)
	{
		CloseHandle(ThreadParameter.hEventFinish);
		return false;
	}

	
	WaitForSingleObject(ThreadParameter.hEventFinish,INFINITE);
	CloseHandle(ThreadParameter.hEventFinish);

	
	if (ThreadParameter.bSuccess==false)
	{
		ConcludeThread(INFINITE);
		return false;
	}

	return true;
}


bool CWHThread::ConcludeThread(DWORD dwMillSeconds)
{
	
	if (IsRuning()==true)
	{
		
		m_bRun=false;

		
		if (WaitForSingleObject(m_hThreadHandle,dwMillSeconds)==WAIT_TIMEOUT)
		{
			return false;
		}
	}

	
	if (m_hThreadHandle!=NULL)
	{
		
		CloseHandle(m_hThreadHandle);

		
		m_uThreadID=0;
		m_hThreadHandle=NULL;
	}

	return true;
}


LRESULT CWHThread::PostThreadMessage(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	
	ASSERT((m_uThreadID!=0)&&(m_hThreadHandle!=NULL));
	if ((m_uThreadID==0)||(m_hThreadHandle==NULL)) return false;

	
	if (::PostThreadMessage(m_uThreadID,uMessage,wParam,lParam)==FALSE)
	{
		DWORD dwLastError=GetLastError();
		return dwLastError;
	}

	return 0L;
}


unsigned __stdcall CWHThread::ThreadFunction(LPVOID pThreadData)
{
	
	srand((DWORD)time(NULL));

	
	tagThreadParameter * pThreadParameter=(tagThreadParameter *)pThreadData;
	CWHThread * pServiceThread=pThreadParameter->pServiceThread;

	
	try
	{
		pThreadParameter->bSuccess=pServiceThread->OnEventThreadStrat(); 
	} 
	catch (...)
	{
		
		ASSERT(FALSE);
		pThreadParameter->bSuccess=false;
	}

	
	bool bSuccess=pThreadParameter->bSuccess;
	ASSERT(pThreadParameter->hEventFinish!=NULL);
	if (pThreadParameter->hEventFinish!=NULL) SetEvent(pThreadParameter->hEventFinish);

	
	if (bSuccess==true)
	{
		
		while (pServiceThread->m_bRun)
		{
#ifndef _DEBUG
			
			try
			{
				if (pServiceThread->OnEventThreadRun()==false)
				{
					break;
				}
			}
			catch (...)	{ }
#else
			
			if (pServiceThread->OnEventThreadRun()==false)
			{
				break;
			}
#endif
		}

		
		try
		{ 
			pServiceThread->OnEventThreadConclude();
		} 
		catch (...)	{ ASSERT(FALSE); }
	}

	
	_endthreadex(0L);

	return 0L;
}


