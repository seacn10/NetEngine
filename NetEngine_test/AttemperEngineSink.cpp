#include "StdAfx.h"


#include "AttemperEngineSink.h"

#include <iostream>
#include <assert.h>

#define IDI_LOAD_PLATFORM_PARAMETER		1							
#define TIME_LOAD_PLATFORM_PARAMETER	600000						

CAttemperEngineSink::CAttemperEngineSink()
{
	m_wCollectItem=INVALID_WORD;
	
	m_pITCPNetworkEngine=NULL;

	return;
}

CAttemperEngineSink::~CAttemperEngineSink()
{
}

VOID * CAttemperEngineSink::QueryInterface(REFGUID Guid, DWORD dwQueryVer)
{
	
	QUERYINTERFACE(IAttemperEngineSink, Guid, dwQueryVer);
	
	QUERYINTERFACE_IUNKNOWNEX(IAttemperEngineSink, Guid, dwQueryVer);
	return NULL;
}

bool CAttemperEngineSink::OnAttemperEngineStart(IUnknownEx * pIUnknownEx)
{
	return true;
}

bool CAttemperEngineSink::OnAttemperEngineConclude(IUnknownEx * pIUnknownEx)
{
	m_wCollectItem=INVALID_WORD;
	m_WaitCollectItemArray.RemoveAll();

	m_pITCPNetworkEngine=NULL;

	return true;
}

bool CAttemperEngineSink::OnEventTCPNetworkBind(DWORD dwClientAddr, DWORD dwSocketID)
{
	struct in_addr in = { 0 };
	in.S_un.S_addr = dwClientAddr;

	char szInfo[1024] = { 0 };
	sprintf_s(szInfo, 1024, "receive connect from client£¬Client IP:%u.%u.%u.%u\r\n", in.S_un.S_un_b.s_b1, in.S_un.S_un_b.s_b2, in.S_un.S_un_b.s_b3, in.S_un.S_un_b.s_b4);
	
	printf_s(szInfo);
	return true;
}

bool CAttemperEngineSink::OnEventTCPNetworkRead(TCP_Command Command, VOID * pData, WORD wDataSize, DWORD dwSocketID)
{
	if (Command.wMainCmdID == 3)
	{
		switch (Command.wSubCmdID)
		{
		case 160:
			{
				UserEnableInsure *pInsure = (UserEnableInsure*)pData;

				TCHAR tcInfo[1024] = { 0 };
				_stprintf_s(tcInfo, 1024, L"receive client message: UserID is %u, MachineID is %s, LogonPass is %s, InsurePass is %s\r\n", pInsure->dwUserID, pInsure->szMachineID, pInsure->szLogonPass, pInsure->szInsurePass);

				wprintf(tcInfo);
			}

			break;
		default:
			break;
		}
	}
	return true;
}


bool CAttemperEngineSink::OnEventTCPNetworkShut(DWORD dwClientAddr, DWORD dwActiveTime, DWORD dwSocketID)
{
	wprintf(L"OnEventTCPNetworkShut\r\n");
	return true;
}