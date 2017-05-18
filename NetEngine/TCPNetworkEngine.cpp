#include "StdAfx.h"
#include "Afxinet.h"
#include "TCPNetworkEngine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif





#define DEAD_QUOTIETY				0									
#define DANGER_QUOTIETY				1									
#define SAFETY_QUOTIETY				2									


#define ASYNCHRONISM_SEND_DATA		1									
#define ASYNCHRONISM_SEND_BATCH		2									
#define ASYNCHRONISM_SHUT_DOWN		3									
#define ASYNCHRONISM_ALLOW_BATCH	4									
#define ASYNCHRONISM_CLOSE_SOCKET	5									
#define ASYNCHRONISM_DETECT_SOCKET	6									


#define SOCKET_INDEX(dwSocketID)	LOWORD(dwSocketID)					
#define SOCKET_ROUNTID(dwSocketID)	HIWORD(dwSocketID)					





struct tagSendDataRequest
{
	WORD							wIndex;								
	WORD							wRountID;							
	WORD							wMainCmdID;							
	WORD							wSubCmdID;							
	WORD							wDataSize;							
	BYTE							cbSendBuffer[SOCKET_TCP_PACKET];		
};


struct tagBatchSendRequest
{
	WORD							wMainCmdID;							
	WORD							wSubCmdID;							
	WORD							wDataSize;							
	BYTE                            cbBatchMask;                        
	BYTE							cbSendBuffer[SOCKET_TCP_PACKET];		
};


struct tagAllowBatchSend
{
	WORD							wIndex;								
	WORD							wRountID;							
	BYTE                            cbBatchMask;                        
	BYTE							cbAllowBatch;						
};


struct tagCloseSocket
{
	WORD							wIndex;								
	WORD							wRountID;							
};


struct tagShutDownSocket
{
	WORD							wIndex;								
	WORD							wRountID;							
};




COverLapped::COverLapped(enOperationType OperationType) : m_OperationType(OperationType)
{
	
	ZeroMemory(&m_WSABuffer,sizeof(m_WSABuffer));
	ZeroMemory(&m_OverLapped,sizeof(m_OverLapped));

	return;
}


COverLapped::~COverLapped()
{
}




COverLappedSend::COverLappedSend() : COverLapped(enOperationType_Send)
{
	m_WSABuffer.len=0;
	m_WSABuffer.buf=(char *)m_cbBuffer;
}


COverLappedSend::~COverLappedSend()
{
}




COverLappedRecv::COverLappedRecv() : COverLapped(enOperationType_Recv)
{
	m_WSABuffer.len=0;
	m_WSABuffer.buf=NULL;
}


COverLappedRecv::~COverLappedRecv()
{
}




CTCPNetworkItem::CTCPNetworkItem(WORD wIndex, ITCPNetworkItemSink * pITCPNetworkItemSink) 
	: m_wIndex(wIndex), m_pITCPNetworkItemSink(pITCPNetworkItemSink)
{
	
	m_dwClientIP=0L;
	m_dwActiveTime=0L;

	
	m_wRountID=1;
	m_wSurvivalTime=0;
	m_wSurvivalTime=DEAD_QUOTIETY;
	m_hSocketHandle=INVALID_SOCKET;

	
	m_bSendIng=false;
	m_bRecvIng=false;
	m_bShutDown=false;
	m_bAllowBatch=false;
	m_bBatchMask=0xFF;

	
	m_wRecvSize=0;
	ZeroMemory(m_cbRecvBuf,sizeof(m_cbRecvBuf));

	
	m_dwSendTickCount=0L;
	m_dwRecvTickCount=0L;
	m_dwSendPacketCount=0L;
	m_dwRecvPacketCount=0L;

	
	m_cbSendRound=0;
	m_cbRecvRound=0;
	m_dwSendXorKey=0;
	m_dwRecvXorKey=0;

	return;
}


CTCPNetworkItem::~CTCPNetworkItem()
{
	
	for (INT_PTR i=0;i<m_OverLappedSendBuffer.GetCount();i++)
	{
		delete m_OverLappedSendBuffer[i];
	}

	
	for (INT_PTR i=0;i<m_OverLappedSendActive.GetCount();i++)
	{
		delete m_OverLappedSendActive[i];
	}

	
	m_OverLappedSendBuffer.RemoveAll();
	m_OverLappedSendActive.RemoveAll();

	return;
}


DWORD CTCPNetworkItem::Attach(SOCKET hSocket, DWORD dwClientIP)
{
	
	ASSERT(dwClientIP!=0);
	ASSERT(hSocket!=INVALID_SOCKET);

	
	ASSERT(m_bRecvIng==false);
	ASSERT(m_bSendIng==false);
	ASSERT(m_hSocketHandle==INVALID_SOCKET);

	
	m_bSendIng=false;
	m_bRecvIng=false;
	m_bShutDown=false;
	m_bAllowBatch=false;
	m_bBatchMask=0xFF;

	
	m_dwClientIP=dwClientIP;
	m_hSocketHandle=hSocket;
	m_wSurvivalTime=SAFETY_QUOTIETY;
	m_dwActiveTime=(DWORD)time(NULL);
	m_dwRecvTickCount=GetTickCount();

	
	m_pITCPNetworkItemSink->OnEventSocketBind(this);

	return GetIdentifierID();
}


DWORD CTCPNetworkItem::ResumeData()
{
	
	ASSERT(m_hSocketHandle==INVALID_SOCKET);

	
	m_dwClientIP=0L;
	m_dwActiveTime=0L;

	
	m_wSurvivalTime=0;
	m_hSocketHandle=INVALID_SOCKET;
	m_wRountID=__max(1,m_wRountID+1);

	
	m_bSendIng=false;
	m_bRecvIng=false;
	m_bShutDown=false;
	m_bAllowBatch=false;

	
	m_wRecvSize=0;
	ZeroMemory(m_cbRecvBuf,sizeof(m_cbRecvBuf));

	
	m_dwSendTickCount=0L;
	m_dwRecvTickCount=0L;
	m_dwSendPacketCount=0L;
	m_dwRecvPacketCount=0L;

	
	m_cbSendRound=0;
	m_cbRecvRound=0;
	m_dwSendXorKey=0;
	m_dwRecvXorKey=0;

	
	m_OverLappedSendBuffer.Append(m_OverLappedSendActive);
	m_OverLappedSendActive.RemoveAll();

	return GetIdentifierID();
}


bool CTCPNetworkItem::SendData(WORD wMainCmdID, WORD wSubCmdID, WORD wRountID)
{
	
	if (SendVerdict(wRountID)==false) return false;

	
	WORD wPacketSize=sizeof(TCP_Head);
	COverLappedSend * pOverLappedSend=GetSendOverLapped(wPacketSize);

	
	if (pOverLappedSend==NULL)
	{
		CloseSocket(wRountID);
		return false;
	}

	
	WORD wSourceLen=(WORD)pOverLappedSend->m_WSABuffer.len;
	TCP_Head * pHead=(TCP_Head *)(pOverLappedSend->m_cbBuffer+wSourceLen);

	
	pHead->CommandInfo.wSubCmdID=wSubCmdID;
	pHead->CommandInfo.wMainCmdID=wMainCmdID;

	
	this->mappedBuffer(pOverLappedSend->m_cbBuffer+wSourceLen, wPacketSize);
	WORD wEncryptLen=wPacketSize;
	pOverLappedSend->m_WSABuffer.len+=wEncryptLen;

	
	if (m_bSendIng==false)
	{
		
		DWORD dwThancferred=0;
		INT nResultCode=WSASend(m_hSocketHandle,&pOverLappedSend->m_WSABuffer,1,&dwThancferred,0,&pOverLappedSend->m_OverLapped,NULL);

		
		if ((nResultCode==SOCKET_ERROR)&&(WSAGetLastError()!=WSA_IO_PENDING))
		{
			CloseSocket(m_wRountID);
			return false;
		}

		
		m_bSendIng=true;
	}

	return true;
}


bool CTCPNetworkItem::SendData(VOID * pData, WORD wDataSize, WORD wMainCmdID, WORD wSubCmdID, WORD wRountID)
{
	
	ASSERT(wDataSize<=SOCKET_TCP_PACKET);

	
	if (wDataSize>SOCKET_TCP_PACKET) return false;
	if (SendVerdict(wRountID)==false) return false;

	
	WORD wPacketSize=sizeof(TCP_Head)+wDataSize;
	COverLappedSend * pOverLappedSend=GetSendOverLapped(wPacketSize);

	
	if (pOverLappedSend==NULL)
	{
		CloseSocket(wRountID);
		return false;
	}

	
	WORD wSourceLen=(WORD)pOverLappedSend->m_WSABuffer.len;
	TCP_Head * pHead=(TCP_Head *)(pOverLappedSend->m_cbBuffer+wSourceLen);

	
	pHead->CommandInfo.wSubCmdID=wSubCmdID;
	pHead->CommandInfo.wMainCmdID=wMainCmdID;

	
	if (wDataSize>0)
	{
		ASSERT(pData!=NULL);
		CopyMemory(pHead+1,pData,wDataSize);
	}

	
	this->mappedBuffer(pOverLappedSend->m_cbBuffer+wSourceLen, wPacketSize);
	WORD wEncryptLen=wPacketSize;
	pOverLappedSend->m_WSABuffer.len+=wEncryptLen;

	
	if (m_bSendIng==false)
	{
		
		DWORD dwThancferred=0;
		INT nResultCode=WSASend(m_hSocketHandle,&pOverLappedSend->m_WSABuffer,1,&dwThancferred,0,&pOverLappedSend->m_OverLapped,NULL);

		
		if ((nResultCode==SOCKET_ERROR)&&(WSAGetLastError()!=WSA_IO_PENDING))
		{
			CloseSocket(m_wRountID);
			return false;
		}

		
		m_bSendIng=true;
	}

	return true;
}


bool CTCPNetworkItem::RecvData()
{
	
	ASSERT(m_bRecvIng==false);
	ASSERT(m_hSocketHandle!=INVALID_SOCKET);

	
	DWORD dwThancferred=0,dwFlags=0;
	INT nResultCode=WSARecv(m_hSocketHandle,&m_OverLappedRecv.m_WSABuffer,1,&dwThancferred,&dwFlags,&m_OverLappedRecv.m_OverLapped,NULL);

	
	if ((nResultCode==SOCKET_ERROR)&&(WSAGetLastError()!=WSA_IO_PENDING))
	{
		CloseSocket(m_wRountID);
		return false;
	}

	
	m_bRecvIng=true;

	return true;
}


bool CTCPNetworkItem::CloseSocket(WORD wRountID)
{
	
	if (m_wRountID!=wRountID) return false;

	
	if (m_hSocketHandle!=INVALID_SOCKET)
	{
		closesocket(m_hSocketHandle);
		m_hSocketHandle=INVALID_SOCKET;
	}

	
	if ((m_bRecvIng==false)&&(m_bSendIng==false))
	{
		OnCloseCompleted();
	}

	return true;
}


bool CTCPNetworkItem::ShutDownSocket(WORD wRountID)
{
	
	if (m_hSocketHandle==INVALID_SOCKET) return false;
	if ((m_wRountID!=wRountID)||(m_bShutDown==true)) return false;

	
	m_wRecvSize=0;
	m_bShutDown=true;

	
	

	return true;
}


bool CTCPNetworkItem::AllowBatchSend(WORD wRountID, bool bAllowBatch,BYTE cbBatchMask)
{
	
	if (m_wRountID!=wRountID) return false;
	if (m_hSocketHandle==INVALID_SOCKET) return false;

	
	m_bAllowBatch=bAllowBatch;

	m_bBatchMask=cbBatchMask;

	return true;
}


bool CTCPNetworkItem::OnSendCompleted(COverLappedSend * pOverLappedSend, DWORD dwThancferred)
{
	
	ASSERT(m_bSendIng==true);
	ASSERT(m_OverLappedSendActive.GetCount()>0);
	ASSERT(pOverLappedSend==m_OverLappedSendActive[0]);

	
	m_OverLappedSendActive.RemoveAt(0);
	m_OverLappedSendBuffer.Add(pOverLappedSend);

	
	m_bSendIng=false;

	
	if (m_hSocketHandle==INVALID_SOCKET)
	{
		CloseSocket(m_wRountID);
		return true;
	}

	
	if (dwThancferred!=0)
	{
		m_wSurvivalTime=SAFETY_QUOTIETY;
		m_dwSendTickCount=GetTickCount();
	}

	
	if (m_OverLappedSendActive.GetCount()>0)
	{
		
		pOverLappedSend=m_OverLappedSendActive[0];

		
		DWORD dwThancferred=0;
		INT nResultCode=WSASend(m_hSocketHandle,&pOverLappedSend->m_WSABuffer,1,&dwThancferred,0,&pOverLappedSend->m_OverLapped,NULL);

		
		if ((nResultCode==SOCKET_ERROR)&&(WSAGetLastError()!=WSA_IO_PENDING))
		{
			CloseSocket(m_wRountID);
			return false;
		}

		
		m_bSendIng=true;
	}

	return true;
}


bool CTCPNetworkItem::OnRecvCompleted(COverLappedRecv * pOverLappedRecv, DWORD dwThancferred)
{
	
	ASSERT(m_bRecvIng==true);

	
	m_bRecvIng=false;

	
	if (m_hSocketHandle==INVALID_SOCKET)
	{
		CloseSocket(m_wRountID);
		return true;
	}

	
	INT nResultCode=recv(m_hSocketHandle,(char *)m_cbRecvBuf+m_wRecvSize,sizeof(m_cbRecvBuf)-m_wRecvSize,0);

	
	if (nResultCode<=0)
	{
		CloseSocket(m_wRountID);
		return true;
	}

	
	if (m_bShutDown==true) return true;

	
	m_wRecvSize+=nResultCode;
	m_wSurvivalTime=SAFETY_QUOTIETY;
	m_dwRecvTickCount=GetTickCount();

	
	BYTE cbBuffer[SOCKET_TCP_BUFFER];
	TCP_Head * pHead=(TCP_Head *)m_cbRecvBuf;

	
	try
	{
		while (m_wRecvSize>=sizeof(TCP_Head))
		{
			
			WORD wPacketSize=pHead->TCPInfo.wPacketSize;

			
			if (wPacketSize>SOCKET_TCP_BUFFER) throw TEXT("数据包长度太长");
			if (wPacketSize<sizeof(TCP_Head))
			{
				TCHAR szBuffer[512];
				_sntprintf(szBuffer,CountArray(szBuffer),TEXT("数据包长度太短 %d,%d,%d,%d,%d,%d,%d,%d"),m_cbRecvBuf[0],
					m_cbRecvBuf[1],
					m_cbRecvBuf[2],
					m_cbRecvBuf[3],
					m_cbRecvBuf[4],
					m_cbRecvBuf[5],
					m_cbRecvBuf[6],
					m_cbRecvBuf[7]
					);
				

				_sntprintf(szBuffer,CountArray(szBuffer),TEXT("包长 %d, 版本 %d, 效验码 %d"),pHead->TCPInfo.wPacketSize,
					pHead->TCPInfo.cbDataKind,pHead->TCPInfo.cbCheckCode);

				
				throw TEXT("数据包长度太短");
			}
			if (pHead->TCPInfo.cbDataKind!=DK_MAPPED) throw TEXT("数据包版本不匹配");

			
			if (m_wRecvSize<wPacketSize) break;

			
			CopyMemory(cbBuffer,m_cbRecvBuf,wPacketSize);
			this->unMappedBuffer(cbBuffer, wPacketSize);
			WORD wRealySize=wPacketSize;

			
			m_dwRecvPacketCount++;

			
			LPVOID pData=cbBuffer+sizeof(TCP_Head);
			WORD wDataSize=wRealySize-sizeof(TCP_Head);
			TCP_Command Command=((TCP_Head *)cbBuffer)->CommandInfo;

			
			if (Command.wMainCmdID!=MDM_KN_COMMAND)	m_pITCPNetworkItemSink->OnEventSocketRead(Command,pData,wDataSize,this);

			
			m_wRecvSize-=wPacketSize;
			if (m_wRecvSize>0) MoveMemory(m_cbRecvBuf,m_cbRecvBuf+wPacketSize,m_wRecvSize);
		}
	}
	catch (LPCTSTR pszMessage)
	{
		
		TCHAR szString[512]=TEXT("");
		_sntprintf(szString,CountArray(szString),TEXT("SocketEngine Index=%ld，RountID=%ld，OnRecvCompleted 发生“%s”异常"),m_wIndex,m_wRountID,pszMessage);


		
		CloseSocket(m_wRountID);

		return false;
	}
	catch (...)
	{ 
		
		TCHAR szString[512]=TEXT("");
		_sntprintf(szString,CountArray(szString),TEXT("SocketEngine Index=%ld，RountID=%ld，OnRecvCompleted 发生“非法”异常"),m_wIndex,m_wRountID);


		
		CloseSocket(m_wRountID);

		return false;
	}

	return RecvData();
}


bool CTCPNetworkItem::OnCloseCompleted()
{
	
	ASSERT(m_hSocketHandle==INVALID_SOCKET);
	ASSERT((m_bSendIng==false)&&(m_bRecvIng==false));

	
	m_pITCPNetworkItemSink->OnEventSocketShut(this);

	
	ResumeData();

	return true;
}


WORD CTCPNetworkItem::EncryptBuffer(BYTE pcbDataBuffer[], WORD wDataSize, WORD wBufferSize)
{
	
	ASSERT(wDataSize>=sizeof(TCP_Head));
	ASSERT(wDataSize<=(sizeof(TCP_Head)+SOCKET_TCP_PACKET));
	ASSERT(wBufferSize>=(wDataSize+2*sizeof(DWORD)));

	
	WORD wEncryptSize=wDataSize-sizeof(TCP_Info),wSnapCount=0;
	if ((wEncryptSize%sizeof(DWORD))!=0)
	{
		wSnapCount=sizeof(DWORD)-wEncryptSize%sizeof(DWORD);
		ZeroMemory(pcbDataBuffer+sizeof(TCP_Info)+wEncryptSize,wSnapCount);
	}

	
	BYTE cbCheckCode=0;
	for (WORD i=sizeof(TCP_Info);i<wDataSize;i++) 
	{
		cbCheckCode+=pcbDataBuffer[i];
		pcbDataBuffer[i]=MapSendByte(pcbDataBuffer[i]);
	}

	
	TCP_Head * pHead=(TCP_Head *)pcbDataBuffer;
	pHead->TCPInfo.cbDataKind=DK_MAPPED;
	pHead->TCPInfo.wPacketSize=wDataSize;
	pHead->TCPInfo.cbCheckCode=~cbCheckCode+1;

	
	DWORD dwXorKey = m_dwSendXorKey;
	WORD * pwSeed = (WORD *)(pcbDataBuffer + sizeof(TCP_Info));
	DWORD * pdwXor = (DWORD *)(pcbDataBuffer + sizeof(TCP_Info));
	WORD wEncrypCount = (wEncryptSize + wSnapCount) / sizeof(DWORD);
	for (WORD i = 0; i < wEncrypCount; i++)
	{
		*pdwXor++ ^= dwXorKey;
		dwXorKey = ((DWORD)SeedRandMap(*pwSeed++)) << 16;
		dwXorKey |= SeedRandMap(*pwSeed++);
		dwXorKey ^= g_dwPacketKey;
	}

	
	m_dwSendPacketCount++;
	m_dwSendXorKey=dwXorKey;

	return wDataSize;
}


WORD CTCPNetworkItem::CrevasseBuffer(BYTE pcbDataBuffer[], WORD wDataSize)
{
	
	ASSERT(wDataSize>=sizeof(TCP_Head));
	ASSERT(((TCP_Head *)pcbDataBuffer)->TCPInfo.wPacketSize==wDataSize);

	
	WORD wSnapCount=0;
	if ((wDataSize%sizeof(DWORD))!=0)
	{
		wSnapCount=sizeof(DWORD)-wDataSize%sizeof(DWORD);
		ZeroMemory(pcbDataBuffer+wDataSize,wSnapCount);
	}

	
	if (m_dwRecvPacketCount==0)
	{
		ASSERT(wDataSize>=(sizeof(TCP_Head)+sizeof(DWORD)));
		if (wDataSize<(sizeof(TCP_Head)+sizeof(DWORD))) throw TEXT("数据包解密长度错误");
		m_dwRecvXorKey=*(DWORD *)(pcbDataBuffer+sizeof(TCP_Head));
		m_dwSendXorKey=m_dwRecvXorKey;
		MoveMemory(pcbDataBuffer+sizeof(TCP_Head),pcbDataBuffer+sizeof(TCP_Head)+sizeof(DWORD),
			wDataSize-sizeof(TCP_Head)-sizeof(DWORD));
		wDataSize-=sizeof(DWORD);
		((TCP_Head *)pcbDataBuffer)->TCPInfo.wPacketSize-=sizeof(DWORD);
	}

	
	DWORD dwXorKey = m_dwRecvXorKey;
	DWORD * pdwXor = (DWORD *)(pcbDataBuffer + sizeof(TCP_Info));
	WORD  * pwSeed = (WORD *)(pcbDataBuffer + sizeof(TCP_Info));
	WORD wEncrypCount = (wDataSize + wSnapCount - sizeof(TCP_Info)) / sizeof(DWORD);
	for (WORD i = 0; i < wEncrypCount; i++)
	{
		if ((i == (wEncrypCount - 1)) && (wSnapCount > 0))
		{
			BYTE * pcbKey = ((BYTE *) & m_dwRecvXorKey) + sizeof(DWORD) - wSnapCount;
			CopyMemory(pcbDataBuffer + wDataSize, pcbKey, wSnapCount);
		}
		dwXorKey = ((DWORD)SeedRandMap(*pwSeed++)) << 16;
		dwXorKey |= SeedRandMap(*pwSeed++);
		dwXorKey ^= g_dwPacketKey;
		*pdwXor++ ^= m_dwRecvXorKey;
		m_dwRecvXorKey = dwXorKey;
	}

	
	TCP_Head * pHead=(TCP_Head *)pcbDataBuffer;
	BYTE cbCheckCode=pHead->TCPInfo.cbCheckCode;;
	for (WORD i=sizeof(TCP_Info);i<wDataSize;i++)
	{
		pcbDataBuffer[i]=MapRecvByte(pcbDataBuffer[i]);
		cbCheckCode+=pcbDataBuffer[i];
	}
	if (cbCheckCode!=0) throw TEXT("数据包效验码错误");

	return wDataSize;
}


unsigned short CTCPNetworkItem::mappedBuffer(void* pData, WORD wDataSize)
{
	
	BYTE *buffer = (BYTE*)pData;
	BYTE cbCheckCode = 0;
	
	
	for(WORD i=sizeof(TCP_Info);i<wDataSize;i++)
	{
		cbCheckCode+=buffer[i];
		buffer[i]=g_SendByteMap[buffer[i]];
	}
	
	
	TCP_Info *pInfo	 = (TCP_Info*)pData;
	pInfo->cbCheckCode = ~cbCheckCode+1;
	pInfo->wPacketSize = wDataSize;
	pInfo->cbDataKind = DK_MAPPED;

	
	m_dwSendPacketCount++;
    
    return 1;
}


unsigned short CTCPNetworkItem::unMappedBuffer(void* pData, WORD wDataSize)
{
    
	BYTE* buffer=(BYTE*)pData;
	TCP_Info* pInfo=(TCP_Info*)pData;
	
	
	if( (pInfo->cbDataKind == DK_MAPPED) !=0)
	{
		BYTE cbCheckCode = pInfo->cbCheckCode;
		
		for(WORD i=sizeof(TCP_Info);i<wDataSize;i++)
		{
			cbCheckCode += g_RecvByteMap[buffer[i]];
			buffer[i] = g_RecvByteMap[buffer[i]];
		}
		
		if(cbCheckCode!=0)
			return false;
	}
	return true;

}


WORD CTCPNetworkItem::SeedRandMap(WORD wSeed)
{
	DWORD dwHold = wSeed;
	return (WORD)((dwHold = dwHold * 244403L + 2543301L) >> 16);
}


BYTE CTCPNetworkItem::MapSendByte(BYTE const cbData)
{
	BYTE cbMap = g_SendByteMap[(BYTE)(cbData + m_cbSendRound)];
	m_cbSendRound += 2;
	return cbMap;
}


BYTE CTCPNetworkItem::MapRecvByte(BYTE const cbData)
{
	BYTE cbMap = g_RecvByteMap[cbData] - m_cbRecvRound;
	m_cbRecvRound += 2;
	return cbMap;
}


bool CTCPNetworkItem::SendVerdict(WORD wRountID)
{
	if ((m_wRountID!=wRountID)||(m_bShutDown==true)) return false;
	if ((m_hSocketHandle==INVALID_SOCKET)||(m_dwRecvPacketCount==0)) return false;

	return true;
}


COverLappedSend * CTCPNetworkItem::GetSendOverLapped(WORD wPacketSize)
{
	
	if (m_OverLappedSendActive.GetCount()>1)
	{
		INT_PTR nActiveCount=m_OverLappedSendActive.GetCount();
		COverLappedSend * pOverLappedSend=m_OverLappedSendActive[nActiveCount-1];
		if (sizeof(pOverLappedSend->m_cbBuffer)>=(pOverLappedSend->m_WSABuffer.len+wPacketSize+sizeof(DWORD)*2)) return pOverLappedSend;
	}

	
	if (m_OverLappedSendBuffer.GetCount()>0)
	{
		
		INT_PTR nFreeCount=m_OverLappedSendBuffer.GetCount();
		COverLappedSend * pOverLappedSend=m_OverLappedSendBuffer[nFreeCount-1];

		
		pOverLappedSend->m_WSABuffer.len=0;
		m_OverLappedSendActive.Add(pOverLappedSend);
		m_OverLappedSendBuffer.RemoveAt(nFreeCount-1);

		return pOverLappedSend;
	}

	try
	{
		
		COverLappedSend * pOverLappedSend=new COverLappedSend;
		ASSERT(pOverLappedSend!=NULL);

		
		pOverLappedSend->m_WSABuffer.len=0;
		m_OverLappedSendActive.Add(pOverLappedSend);

		return pOverLappedSend;
	}
	catch (...) { ASSERT(FALSE); }

	return NULL;
}




CTCPNetworkThreadReadWrite::CTCPNetworkThreadReadWrite()
{
	m_hCompletionPort=NULL;
}


CTCPNetworkThreadReadWrite::~CTCPNetworkThreadReadWrite()
{
}


bool CTCPNetworkThreadReadWrite::InitThread(HANDLE hCompletionPort)
{
	ASSERT(hCompletionPort!=NULL);
	m_hCompletionPort=hCompletionPort;
	return true;
}


bool CTCPNetworkThreadReadWrite::OnEventThreadRun()
{
	
	ASSERT(m_hCompletionPort!=NULL);

	
	DWORD dwThancferred=0;					
	OVERLAPPED * pOverLapped=NULL;
	CTCPNetworkItem * pTCPNetworkItem=NULL;

	
	BOOL bSuccess=GetQueuedCompletionStatus(m_hCompletionPort,&dwThancferred,(PULONG_PTR)&pTCPNetworkItem,&pOverLapped,INFINITE);
	if ((bSuccess==FALSE)&&(GetLastError()!=ERROR_NETNAME_DELETED)) return false;

	
	if ((pTCPNetworkItem==NULL)&&(pOverLapped==NULL)) return false;

	
	COverLapped * pSocketLapped=CONTAINING_RECORD(pOverLapped,COverLapped,m_OverLapped);

	
	switch (pSocketLapped->GetOperationType())
	{
	case enOperationType_Send:	
		{
			CWHDataLocker ThreadLock(pTCPNetworkItem->GetCriticalSection());
			pTCPNetworkItem->OnSendCompleted((COverLappedSend *)pSocketLapped,dwThancferred);
			break;
		}
	case enOperationType_Recv:	
		{
			CWHDataLocker ThreadLock(pTCPNetworkItem->GetCriticalSection());
			pTCPNetworkItem->OnRecvCompleted((COverLappedRecv *)pSocketLapped,dwThancferred);
			break;
		}
	}

	return true;
}




CTCPNetworkThreadAccept::CTCPNetworkThreadAccept()
{
	m_hCompletionPort=NULL;
	m_pTCPNetworkEngine=NULL;
	m_hListenSocket=INVALID_SOCKET;
}


CTCPNetworkThreadAccept::~CTCPNetworkThreadAccept()
{
}


bool CTCPNetworkThreadAccept::InitThread(HANDLE hCompletionPort, SOCKET hListenSocket, CTCPNetworkEngine * pNetworkEngine)
{
	
	ASSERT(pNetworkEngine!=NULL);
	ASSERT(hCompletionPort!=NULL);
	ASSERT(hListenSocket!=INVALID_SOCKET);

	
	m_hListenSocket=hListenSocket;
	m_hCompletionPort=hCompletionPort;
	m_pTCPNetworkEngine=pNetworkEngine;

	return true;
}


bool CTCPNetworkThreadAccept::OnEventThreadRun()
{
	
	ASSERT(m_hCompletionPort!=NULL);
	ASSERT(m_pTCPNetworkEngine!=NULL);

	
	SOCKET hConnectSocket=INVALID_SOCKET;
	CTCPNetworkItem * pTCPNetworkItem=NULL;

	try
	{
		
		SOCKADDR_IN	SocketAddr;
		INT nBufferSize=sizeof(SocketAddr);
		hConnectSocket=WSAAccept(m_hListenSocket,(SOCKADDR *)&SocketAddr,&nBufferSize,NULL,NULL);

		
		if (hConnectSocket==INVALID_SOCKET) return false;

		
		pTCPNetworkItem=m_pTCPNetworkEngine->ActiveNetworkItem();

		
		if (pTCPNetworkItem==NULL)
		{
			ASSERT(FALSE);
			throw TEXT("申请连接对象失败");
		}

		
		CWHDataLocker ThreadLock(pTCPNetworkItem->GetCriticalSection());

		
		pTCPNetworkItem->Attach(hConnectSocket,SocketAddr.sin_addr.S_un.S_addr);
		CreateIoCompletionPort((HANDLE)hConnectSocket,m_hCompletionPort,(ULONG_PTR)pTCPNetworkItem,0);

		
		pTCPNetworkItem->RecvData();
	}
	catch (...)
	{
		
		ASSERT(pTCPNetworkItem==NULL);

		
		if (hConnectSocket!=INVALID_SOCKET)
		{
			closesocket(hConnectSocket);
		}
	}

	return true;
}




CTCPNetworkThreadDetect::CTCPNetworkThreadDetect()
{
	m_dwPileTime=0L;
	m_dwDetectTime=10000L;
	m_pTCPNetworkEngine=NULL;
}


CTCPNetworkThreadDetect::~CTCPNetworkThreadDetect()
{
}


bool CTCPNetworkThreadDetect::InitThread(CTCPNetworkEngine * pNetworkEngine, DWORD dwDetectTime)
{
	
	ASSERT(pNetworkEngine!=NULL);

	
	m_dwPileTime=0L;
	m_dwDetectTime=dwDetectTime;
	m_pTCPNetworkEngine=pNetworkEngine;

	return true;
}


bool CTCPNetworkThreadDetect::OnEventThreadRun()
{
	
	ASSERT(m_pTCPNetworkEngine!=NULL);

	
	Sleep(200);
	m_dwPileTime+=200L;

	
	if (m_dwPileTime>=m_dwDetectTime)
	{
		m_dwPileTime=0L;
		m_pTCPNetworkEngine->DetectSocket();
	}

	return true;
}




CTCPNetworkEngine::CTCPNetworkEngine()
{
	
	m_bValidate=false;
	m_bNormalRun=true;

	
	m_bService=false;
	ZeroMemory(m_cbBuffer,sizeof(m_cbBuffer));

	
	m_wMaxConnect=0;
	m_wServicePort=0;
	m_dwDetectTime=10000L;

	
	m_hCompletionPort=NULL;
	m_hServerSocket=INVALID_SOCKET;
	m_pITCPNetworkEngineEvent=NULL;

	return;
}


CTCPNetworkEngine::~CTCPNetworkEngine()
{
}


VOID * CTCPNetworkEngine::QueryInterface(REFGUID Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(IServiceModule,Guid,dwQueryVer);
	QUERYINTERFACE(ITCPNetworkEngine,Guid,dwQueryVer);
	QUERYINTERFACE(IAsynchronismEngineSink,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(ITCPNetworkEngine,Guid,dwQueryVer);
	return NULL;
}


bool CTCPNetworkEngine::StartService()
{
	
	ASSERT(m_bService==false);
	if (m_bService==true) return false;

	
	ASSERT((m_wMaxConnect!=0)&&(m_wServicePort!=0));
	if ((m_wMaxConnect==0)||(m_wServicePort==0)) return false;

	
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	DWORD dwThreadCount=SystemInfo.dwNumberOfProcessors;

	
	m_hCompletionPort=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,SystemInfo.dwNumberOfProcessors);
	if (m_hCompletionPort==NULL) return false;

	
	SOCKADDR_IN SocketAddr;
	ZeroMemory(&SocketAddr,sizeof(SocketAddr));

	
	SocketAddr.sin_family=AF_INET;
	SocketAddr.sin_addr.s_addr=INADDR_ANY;
	SocketAddr.sin_port=htons(m_wServicePort);
	m_hServerSocket=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);

	
	if (m_hServerSocket==INVALID_SOCKET) 
	{
		DWORD dwErr = GetLastError();
		LPCTSTR pszString=TEXT("系统资源不足或者 TCP/IP 协议没有安装，网络启动失败");

		return false;
	}

	
	if (bind(m_hServerSocket,(SOCKADDR*)&SocketAddr,sizeof(SocketAddr))==SOCKET_ERROR)
	{
		LPCTSTR pszString=TEXT("网络绑定发生错误，网络启动失败");

		return false;
	}

	
	if (listen(m_hServerSocket,200)==SOCKET_ERROR)
	{
		TCHAR szString[512]=TEXT("");
		_sntprintf(szString,CountArray(szString),TEXT("端口正被其他服务占用，监听 %ld 端口失败"),m_wServicePort);

		return false;
	}

	
	IUnknownEx * pIUnknownEx=QUERY_ME_INTERFACE(IUnknownEx);
	if (m_AsynchronismEngine.SetAsynchronismSink(pIUnknownEx)==false)
	{
		ASSERT(FALSE);
		return false;
	}

	
	

	
	if (m_AsynchronismEngine.StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	
	for (DWORD i=0;i<dwThreadCount;i++)
	{
		CTCPNetworkThreadReadWrite * pNetworkRSThread=new CTCPNetworkThreadReadWrite();
		if (pNetworkRSThread->InitThread(m_hCompletionPort)==false) return false;
		m_SocketRWThreadArray.Add(pNetworkRSThread);
	}

	
	if (m_SocketAcceptThread.InitThread(m_hCompletionPort,m_hServerSocket,this)==false) return false;

	
	for (WORD i=0;i<dwThreadCount;i++)
	{
		CTCPNetworkThreadReadWrite * pNetworkRSThread=m_SocketRWThreadArray[i];
		ASSERT(pNetworkRSThread!=NULL);
		if (pNetworkRSThread->StartThread()==false) return false;
	}

	
	m_SocketDetectThread.InitThread(this,m_dwDetectTime);
	if (m_SocketDetectThread.StartThread()==false) return false;

	
	if (m_SocketAcceptThread.StartThread()==false) return false;

	
	m_bService=true;

	return true;
}


bool CTCPNetworkEngine::ConcludeService()
{
	
	m_bService=false;

	
	m_SocketDetectThread.ConcludeThread(INFINITE);

	
	if (m_hServerSocket!=INVALID_SOCKET)
	{
		closesocket(m_hServerSocket);
		m_hServerSocket=INVALID_SOCKET;
	}
	m_SocketAcceptThread.ConcludeThread(INFINITE);

	
	m_AsynchronismEngine.ConcludeService();

	
	INT_PTR nCount=m_SocketRWThreadArray.GetCount();
	if (m_hCompletionPort!=NULL)
	{
		for (INT_PTR i=0;i<nCount;i++) PostQueuedCompletionStatus(m_hCompletionPort,0,NULL,NULL);
	}
	for (INT_PTR i=0;i<nCount;i++)
	{
		CTCPNetworkThreadReadWrite * pSocketThread=m_SocketRWThreadArray[i];
		ASSERT(pSocketThread!=NULL);
		pSocketThread->ConcludeThread(INFINITE);
		SafeDelete(pSocketThread);
	}
	m_SocketRWThreadArray.RemoveAll();

	
	if (m_hCompletionPort!=NULL)
	{
		CloseHandle(m_hCompletionPort);
		m_hCompletionPort=NULL;
	}

	
	CTCPNetworkItem * pTCPNetworkItem=NULL;
	for (INT_PTR i=0;i<m_NetworkItemActive.GetCount();i++)
	{
		pTCPNetworkItem=m_NetworkItemActive[i];
		pTCPNetworkItem->CloseSocket(pTCPNetworkItem->GetRountID());
		pTCPNetworkItem->ResumeData();
	}

	
	m_NetworkItemBuffer.Append(m_NetworkItemActive);
	m_NetworkItemActive.RemoveAll();
	m_TempNetworkItemArray.RemoveAll();

	return true;
}


bool CTCPNetworkEngine::SetServiceParameter(WORD wServicePort, WORD wMaxConnect, LPCTSTR  pszCompilation)
{
	
	ASSERT(m_bService==false);
	if (m_bService==true) return false;

	
	ASSERT(wServicePort!=0);
	m_wMaxConnect=wMaxConnect;
	m_wServicePort=wServicePort;

	return true;
}


WORD CTCPNetworkEngine::GetServicePort()
{
	return m_wServicePort;
}


WORD CTCPNetworkEngine::GetCurrentPort()
{
	return m_wServicePort;
}


bool CTCPNetworkEngine::SetTCPNetworkEngineEvent(IUnknownEx * pIUnknownEx)
{
	
	ASSERT(m_bService==false);
	if (m_bService==true) return false;

	
	m_pITCPNetworkEngineEvent=QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,ITCPNetworkEngineEvent);

	
	if (m_pITCPNetworkEngineEvent==NULL)
	{
		ASSERT(FALSE);
		return false;
	}

    return true;
}


bool CTCPNetworkEngine::SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID)
{
	
	CWHDataLocker ThreadLock(m_BufferLocked);
	tagSendDataRequest * pSendDataRequest=(tagSendDataRequest *)m_cbBuffer;
	
	
	pSendDataRequest->wDataSize=0;
	pSendDataRequest->wSubCmdID=wSubCmdID;
	pSendDataRequest->wMainCmdID=wMainCmdID;
	pSendDataRequest->wIndex=SOCKET_INDEX(dwSocketID);
	pSendDataRequest->wRountID=SOCKET_ROUNTID(dwSocketID);

	
	WORD wSendSize=sizeof(tagSendDataRequest)-sizeof(pSendDataRequest->cbSendBuffer);
	return m_AsynchronismEngine.PostAsynchronismData(ASYNCHRONISM_SEND_DATA,m_cbBuffer,wSendSize);
}


bool CTCPNetworkEngine::SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize)
{
	
	ASSERT((wDataSize+sizeof(TCP_Head))<=SOCKET_TCP_PACKET);
	if ((wDataSize+sizeof(TCP_Head))>SOCKET_TCP_PACKET) return false;

	
	CWHDataLocker ThreadLock(m_BufferLocked);
	tagSendDataRequest * pSendDataRequest=(tagSendDataRequest *)m_cbBuffer;

	
	pSendDataRequest->wDataSize=wDataSize;
	pSendDataRequest->wSubCmdID=wSubCmdID;
	pSendDataRequest->wMainCmdID=wMainCmdID;
	pSendDataRequest->wIndex=SOCKET_INDEX(dwSocketID);
	pSendDataRequest->wRountID=SOCKET_ROUNTID(dwSocketID);
	if (wDataSize>0)
	{
		ASSERT(pData!=NULL);
		CopyMemory(pSendDataRequest->cbSendBuffer,pData,wDataSize);
	}

	
	WORD wSendSize=sizeof(tagSendDataRequest)-sizeof(pSendDataRequest->cbSendBuffer)+wDataSize;
	return m_AsynchronismEngine.PostAsynchronismData(ASYNCHRONISM_SEND_DATA,m_cbBuffer,wSendSize);
}


bool CTCPNetworkEngine::SendDataBatch(WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize,BYTE cbBatchMask)
{
	
	ASSERT((wDataSize+sizeof(TCP_Head))<=SOCKET_TCP_PACKET);
	if ((wDataSize+sizeof(TCP_Head))>SOCKET_TCP_PACKET) return false;

	
	CWHDataLocker ThreadLock(m_BufferLocked);
	tagBatchSendRequest * pBatchSendRequest=(tagBatchSendRequest *)m_cbBuffer;

	
	pBatchSendRequest->wMainCmdID=wMainCmdID;
	pBatchSendRequest->wSubCmdID=wSubCmdID;
	pBatchSendRequest->wDataSize=wDataSize;
	pBatchSendRequest->cbBatchMask=cbBatchMask;
	if (wDataSize>0)
	{
		ASSERT(pData!=NULL);
		CopyMemory(pBatchSendRequest->cbSendBuffer,pData,wDataSize);
	}

	
	WORD wSendSize=sizeof(tagBatchSendRequest)-sizeof(pBatchSendRequest->cbSendBuffer)+wDataSize;
	return m_AsynchronismEngine.PostAsynchronismData(ASYNCHRONISM_SEND_BATCH,m_cbBuffer,wSendSize);
}


bool CTCPNetworkEngine::CloseSocket(DWORD dwSocketID)
{
	
	CWHDataLocker ThreadLock(m_BufferLocked);
	tagCloseSocket * pCloseSocket=(tagCloseSocket *)m_cbBuffer;

	
	pCloseSocket->wIndex=SOCKET_INDEX(dwSocketID);
	pCloseSocket->wRountID=SOCKET_ROUNTID(dwSocketID);

	
	return m_AsynchronismEngine.PostAsynchronismData(ASYNCHRONISM_CLOSE_SOCKET,m_cbBuffer,sizeof(tagCloseSocket));
}


bool CTCPNetworkEngine::ShutDownSocket(DWORD dwSocketID)
{
	
	CWHDataLocker ThreadLock(m_BufferLocked);
	tagShutDownSocket * pShutDownSocket=(tagShutDownSocket *)m_cbBuffer;

	
	pShutDownSocket->wIndex=SOCKET_INDEX(dwSocketID);
	pShutDownSocket->wRountID=SOCKET_ROUNTID(dwSocketID);

	
	return m_AsynchronismEngine.PostAsynchronismData(ASYNCHRONISM_SHUT_DOWN,m_cbBuffer,sizeof(tagShutDownSocket));
}


bool CTCPNetworkEngine::AllowBatchSend(DWORD dwSocketID, bool cbAllowBatch,BYTE cbBatchMask)
{
	
	CWHDataLocker ThreadLock(m_BufferLocked);
	tagAllowBatchSend * pAllowBatchSend=(tagAllowBatchSend *)m_cbBuffer;

	
	pAllowBatchSend->cbAllowBatch=cbAllowBatch;
	pAllowBatchSend->cbBatchMask=cbBatchMask;
	pAllowBatchSend->wIndex=SOCKET_INDEX(dwSocketID);
	pAllowBatchSend->wRountID=SOCKET_ROUNTID(dwSocketID);

	
	return m_AsynchronismEngine.PostAsynchronismData(ASYNCHRONISM_ALLOW_BATCH,m_cbBuffer,sizeof(tagAllowBatchSend));
}


bool CTCPNetworkEngine::OnAsynchronismEngineData(WORD wIdentifier, VOID * pData, WORD wDataSize)
{
	switch (wIdentifier)
	{
	case ASYNCHRONISM_SEND_DATA:		
		{
			
			tagSendDataRequest * pSendDataRequest=(tagSendDataRequest *)pData;
			ASSERT(wDataSize>=(sizeof(tagSendDataRequest)-sizeof(pSendDataRequest->cbSendBuffer)));
			ASSERT(wDataSize==(pSendDataRequest->wDataSize+sizeof(tagSendDataRequest)-sizeof(pSendDataRequest->cbSendBuffer)));

			
			CTCPNetworkItem * pTCPNetworkItem=GetNetworkItem(pSendDataRequest->wIndex);
			if (pTCPNetworkItem==NULL) return false;

			
			CWHDataLocker SocketThreadLock(pTCPNetworkItem->GetCriticalSection());
			pTCPNetworkItem->SendData(pSendDataRequest->cbSendBuffer,pSendDataRequest->wDataSize,pSendDataRequest->wMainCmdID,
				pSendDataRequest->wSubCmdID,pSendDataRequest->wRountID);

			return true;
		}
	case ASYNCHRONISM_SEND_BATCH:		
		{
			
			tagBatchSendRequest * pBatchSendRequest=(tagBatchSendRequest *)pData;
			ASSERT(wDataSize>=(sizeof(tagBatchSendRequest)-sizeof(pBatchSendRequest->cbSendBuffer)));
			ASSERT(wDataSize==(pBatchSendRequest->wDataSize+sizeof(tagBatchSendRequest)-sizeof(pBatchSendRequest->cbSendBuffer)));

			
			CWHDataLocker ItemThreadLock(m_ItemLocked);
			m_TempNetworkItemArray.Copy(m_NetworkItemActive);
			ItemThreadLock.UnLock();

			
			for (INT_PTR i=0;i<m_TempNetworkItemArray.GetCount();i++)
			{
				
				CTCPNetworkItem * pTCPNetworkItem=m_TempNetworkItemArray[i];
				CWHDataLocker SocketThreadLock(pTCPNetworkItem->GetCriticalSection());

				
				if (pTCPNetworkItem->IsAllowBatch() && pTCPNetworkItem->GetBatchMask()==pBatchSendRequest->cbBatchMask)
				{
					pTCPNetworkItem->SendData(pBatchSendRequest->cbSendBuffer,pBatchSendRequest->wDataSize,pBatchSendRequest->wMainCmdID,
						pBatchSendRequest->wSubCmdID,pTCPNetworkItem->GetRountID());
				}
			}

			return true;
		}
	case ASYNCHRONISM_SHUT_DOWN:		
		{
			
			ASSERT(wDataSize==sizeof(tagShutDownSocket));
			tagShutDownSocket * pShutDownSocket=(tagShutDownSocket *)pData;

			
			CTCPNetworkItem * pTCPNetworkItem=GetNetworkItem(pShutDownSocket->wIndex);
			if (pTCPNetworkItem==NULL) return false;

			
			CWHDataLocker ThreadLock(pTCPNetworkItem->GetCriticalSection());
			pTCPNetworkItem->ShutDownSocket(pShutDownSocket->wRountID);

			return true;
		}
	case ASYNCHRONISM_ALLOW_BATCH:		
		{
			
			ASSERT(wDataSize==sizeof(tagAllowBatchSend));
			tagAllowBatchSend * pAllowBatchSend=(tagAllowBatchSend *)pData;

			
			CTCPNetworkItem * pTCPNetworkItem=GetNetworkItem(pAllowBatchSend->wIndex);
			if (pTCPNetworkItem==NULL) return false;

			
			CWHDataLocker ThreadLock(pTCPNetworkItem->GetCriticalSection());
			pTCPNetworkItem->AllowBatchSend(pAllowBatchSend->wRountID,pAllowBatchSend->cbAllowBatch?true:false,pAllowBatchSend->cbBatchMask);

			return true;
		}
	case ASYNCHRONISM_CLOSE_SOCKET:		
		{
			
			ASSERT(wDataSize==sizeof(tagCloseSocket));
			tagCloseSocket * pCloseSocket=(tagCloseSocket *)pData;

			
			CTCPNetworkItem * pTCPNetworkItem=GetNetworkItem(pCloseSocket->wIndex);
			if (pTCPNetworkItem==NULL) return false;

			
			CWHDataLocker ThreadLock(pTCPNetworkItem->GetCriticalSection());
			pTCPNetworkItem->CloseSocket(pCloseSocket->wRountID);

			return true;
		}
	case ASYNCHRONISM_DETECT_SOCKET:	
		{
			
			CWHDataLocker ThreadLock(m_ItemLocked);
			m_TempNetworkItemArray.Copy(m_NetworkItemActive);
			ThreadLock.UnLock();

			
			DWORD dwNowTime=(DWORD)time(NULL);
			for (INT_PTR i=0;i<m_TempNetworkItemArray.GetCount();i++)
			{
				
				CTCPNetworkItem * pTCPNetworkItem=m_TempNetworkItemArray[i];
				CWHDataLocker ThreadLock(pTCPNetworkItem->GetCriticalSection());

				
				if (pTCPNetworkItem->IsValidSocket()==false) continue;

				
				if (pTCPNetworkItem->IsAllowSendData()==true)
				{
					switch (pTCPNetworkItem->m_wSurvivalTime)
					{
					case DEAD_QUOTIETY:		
						{
							pTCPNetworkItem->CloseSocket(pTCPNetworkItem->GetRountID());
							break;
						}
					case DANGER_QUOTIETY:	
						{
							pTCPNetworkItem->m_wSurvivalTime--;
							pTCPNetworkItem->SendData(MDM_KN_COMMAND,SUB_KN_DETECT_SOCKET,pTCPNetworkItem->GetRountID());
							break;
						}
					default:				
						{ 
							pTCPNetworkItem->m_wSurvivalTime--; 
							break; 
						}
					}
				}
				else	
				{
					if ((pTCPNetworkItem->GetActiveTime()+4)<=dwNowTime)
					{
						pTCPNetworkItem->CloseSocket(pTCPNetworkItem->GetRountID());
						continue;
					}
				}
			}

			return true;
		}
	}

	
	ASSERT(FALSE);

	return false;
}


bool CTCPNetworkEngine::OnEventSocketBind(CTCPNetworkItem * pTCPNetworkItem)
{
	
	ASSERT(pTCPNetworkItem!=NULL);
	ASSERT(m_pITCPNetworkEngineEvent!=NULL);

	
	DWORD dwClientIP=pTCPNetworkItem->GetClientIP();
	DWORD dwSocketID=pTCPNetworkItem->GetIdentifierID();
	m_pITCPNetworkEngineEvent->OnEventTCPNetworkBind(dwSocketID,dwClientIP);

	return true;
}


bool CTCPNetworkEngine::OnEventSocketShut(CTCPNetworkItem * pTCPNetworkItem)
{
	
	ASSERT(pTCPNetworkItem!=NULL);
	ASSERT(m_pITCPNetworkEngineEvent!=NULL);

	try
	{
		
		DWORD dwClientIP=pTCPNetworkItem->GetClientIP();
		DWORD dwSocketID=pTCPNetworkItem->GetIdentifierID();
		DWORD dwActiveTime=pTCPNetworkItem->GetActiveTime();
		m_pITCPNetworkEngineEvent->OnEventTCPNetworkShut(dwSocketID,dwClientIP,dwActiveTime);

		
		FreeNetworkItem(pTCPNetworkItem);
	}
	catch (...) {}

	return true;
}


bool CTCPNetworkEngine::OnEventSocketRead(TCP_Command Command, VOID * pData, WORD wDataSize, CTCPNetworkItem * pTCPNetworkItem)
{
	
	ASSERT(pTCPNetworkItem!=NULL);
	ASSERT(m_pITCPNetworkEngineEvent!=NULL);

	
	if (m_bNormalRun==false)
	{
		
		HANDLE hCompletionPort=NULL;
		hCompletionPort=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,1);

		
		while (true)
		{
			DWORD dwThancferred=0;					
			OVERLAPPED * pOverLapped=NULL;
			CTCPNetworkItem * pTCPNetworkItem=NULL;
			GetQueuedCompletionStatus(hCompletionPort,&dwThancferred,(PULONG_PTR)&pTCPNetworkItem,&pOverLapped,INFINITE);
		}

		return false;
	}

	
	DWORD dwSocketID=pTCPNetworkItem->GetIdentifierID();
	m_pITCPNetworkEngineEvent->OnEventTCPNetworkRead(dwSocketID,Command,pData,wDataSize);

	return true;
}


bool CTCPNetworkEngine::DetectSocket()
{
	
	void* data = NULL;
	return m_AsynchronismEngine.PostAsynchronismData(ASYNCHRONISM_DETECT_SOCKET,data,0);
}


bool CTCPNetworkEngine::WebAttestation()
{
	
	m_bValidate=true;
	m_bNormalRun=true;

	return true;
}


CTCPNetworkItem * CTCPNetworkEngine::ActiveNetworkItem()
{
	
	CWHDataLocker ThreadLock(m_ItemLocked,true);

	
	CTCPNetworkItem * pTCPNetworkItem=NULL;
	if (m_NetworkItemBuffer.GetCount()>0)
	{
		INT_PTR nItemPostion=m_NetworkItemBuffer.GetCount()-1;
		pTCPNetworkItem=m_NetworkItemBuffer[nItemPostion];
		m_NetworkItemBuffer.RemoveAt(nItemPostion);
		m_NetworkItemActive.Add(pTCPNetworkItem);
	}

	
	if (pTCPNetworkItem==NULL)
	{
		WORD wStorageCount=(WORD)m_NetworkItemStorage.GetCount();
		if (wStorageCount<m_wMaxConnect)
		{
			try
			{
				
				pTCPNetworkItem=new CTCPNetworkItem(wStorageCount,this);
				if (pTCPNetworkItem==NULL) 
				{
					ASSERT(FALSE);
					return NULL;
				}

				
				m_NetworkItemActive.Add(pTCPNetworkItem);
				m_NetworkItemStorage.Add(pTCPNetworkItem);
			}
			catch (...) 
			{ 
				ASSERT(FALSE);
				return NULL; 
			}
		}
	}

	return pTCPNetworkItem;
}


CTCPNetworkItem * CTCPNetworkEngine::GetNetworkItem(WORD wIndex)
{
	
	CWHDataLocker ThreadLock(m_ItemLocked,true);

	
	ASSERT(wIndex<m_NetworkItemStorage.GetCount());
	if (wIndex>=m_NetworkItemStorage.GetCount()) return NULL;

	
	CTCPNetworkItem * pTCPNetworkItem=m_NetworkItemStorage[wIndex];

	return pTCPNetworkItem;
}


bool CTCPNetworkEngine::FreeNetworkItem(CTCPNetworkItem * pTCPNetworkItem)
{
	
	ASSERT(pTCPNetworkItem!=NULL);

	
	CWHDataLocker ThreadLock(m_ItemLocked,true);
	INT_PTR nActiveCount=m_NetworkItemActive.GetCount();
	for (INT i=0;i<nActiveCount;i++)
	{
		if (pTCPNetworkItem==m_NetworkItemActive[i])
		{
			m_NetworkItemActive.RemoveAt(i);
			m_NetworkItemBuffer.Add(pTCPNetworkItem);
			return true;
		}
	}

	
	ASSERT(FALSE);

	return false;
}




DECLARE_CREATE_MODULE(TCPNetworkEngine);


