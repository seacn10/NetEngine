#ifndef WH_THREAD_HEAD_FILE
#define WH_THREAD_HEAD_FILE

#pragma once

#include "NetCoreHead.h"




class SERVICE_CORE_CLASS CWHThread
{
	
private:
	volatile bool					m_bRun;								

	
private:
	UINT							m_uThreadID;						
	HANDLE							m_hThreadHandle;					

	
protected:
	
	CWHThread();
	
	virtual ~CWHThread();

	
public:
	
	virtual bool IsRuning();
	
	virtual bool StartThread();
	
	virtual bool ConcludeThread(DWORD dwMillSeconds);

	
public:
	
	UINT GetThreadID() { return m_uThreadID; }
	
	HANDLE GetThreadHandle() { return m_hThreadHandle; }
	
	LRESULT PostThreadMessage(UINT uMessage, WPARAM wParam, LPARAM lParam);

	
protected:
	
	virtual bool OnEventThreadRun() { return true; }
	
	virtual bool OnEventThreadStrat() { return true; }
	
	virtual bool OnEventThreadConclude() { return true; }

	
private:
	
	static unsigned __stdcall ThreadFunction(LPVOID pThreadData);
};



#endif
