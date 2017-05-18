#ifndef WH_DATA_LOCKER_HEAD_FILE
#define WH_DATA_LOCKER_HEAD_FILE

#include "NetCoreHead.h"



class SERVICE_CORE_CLASS CWHDataLocker
{
	
private:
	INT								m_nLockCount;					
	CCriticalSection &				m_CriticalSection;				

	
public:
	
	CWHDataLocker(CCriticalSection & CriticalSection, bool bLockAtOnce=true);
	
	virtual ~CWHDataLocker();

	
public:
	
	VOID Lock();
	
	VOID UnLock();

	
public:
	
	inline INT GetLockCount() { return m_nLockCount; }
};



#endif
