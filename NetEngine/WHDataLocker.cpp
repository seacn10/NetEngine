#include "StdAfx.h"
#include "WHDataLocker.h"




CWHDataLocker::CWHDataLocker(CCriticalSection & CriticalSection, bool bLockAtOnce) 
	: m_CriticalSection(CriticalSection)
{
	
	m_nLockCount=0;

	
	if (bLockAtOnce==true)
	{
		Lock();
	}

	return;
}


CWHDataLocker::~CWHDataLocker()
{
	
	while (m_nLockCount>0)
	{
		UnLock();
	}

	return;
}


VOID CWHDataLocker::Lock()
{
	
	m_CriticalSection.Lock();

	
	m_nLockCount++;

	return;
}


VOID CWHDataLocker::UnLock()
{
	
	ASSERT(m_nLockCount>0);
	if (m_nLockCount==0) return;

	
	m_nLockCount--;

	
	m_CriticalSection.Unlock();

	return;
}


