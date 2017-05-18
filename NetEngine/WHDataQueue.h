#ifndef WH_DATA_QUEUE_HEAD_FILE
#define WH_DATA_QUEUE_HEAD_FILE

#include "NetCoreHead.h"





struct tagDataHead
{
	WORD							wDataSize;							
	WORD							wIdentifier;						
};


struct tagBurthenInfo
{
	DWORD							dwDataSize;							
	DWORD							dwBufferSize;						
	DWORD							dwDataPacketCount;					
};


struct tagDataBuffer
{
	WORD							wDataSize;							
	LPVOID							pDataBuffer;						
};




class SERVICE_CORE_CLASS CWHDataQueue
{
	
protected:
	DWORD							m_dwInsertPos;						
	DWORD							m_dwTerminalPos;					
	DWORD							m_dwDataQueryPos;					

	
protected:
	DWORD							m_dwDataSize;						
	DWORD							m_dwDataPacketCount;				

	
protected:
	DWORD							m_dwBufferSize;						
	LPBYTE							m_pDataQueueBuffer;					

	
public:
	
	CWHDataQueue();
	
	virtual ~CWHDataQueue();

	
public:
	
	VOID GetBurthenInfo(tagBurthenInfo & BurthenInfo);
	
	DWORD GetDataPacketCount() { return m_dwDataPacketCount; }

	
public:
	
	bool InsertData(WORD wIdentifier, VOID * pBuffer, WORD wDataSize);
	
	bool InsertData(WORD wIdentifier, tagDataBuffer DataBuffer[], WORD wDataCount);

	
public:
	
	VOID RemoveData(bool bFreeMemroy);
	
	bool DistillData(tagDataHead & DataHead, VOID * pBuffer, WORD wBufferSize);

	
private:
	
	bool RectifyBuffer(DWORD dwNeedSize);
};



#endif
