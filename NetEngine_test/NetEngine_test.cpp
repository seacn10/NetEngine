// NetEngine_test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <assert.h>

#include "..\NetEngine\NetCoreHead.h"
#include "..\NetEngine\NetEngineHead.h"
#include "AttemperEngineSink.h"


#pragma comment(lib, "NetEngine.lib")

CTCPNetworkEngineHelper			m_TCPNetworkEngine;					//��������
//CTCPSocketServiceHelper			m_TCPSocketService;					//Э������
CAttemperEngineHelper			m_AttemperEngine;					//��������

CAttemperEngineSink				m_AttemperEngineSink;				//���ȹ���

const TCHAR szCompilation[] = TEXT("217F8DFB-FE8A-4C3B-A023-C35489A1C0F5");


int _tmain(int argc, _TCHAR* argv[])
{
	if ((m_AttemperEngine.GetInterface() == NULL) && (m_AttemperEngine.CreateInstance() == false))
	{
		return false;
	}

	if ((m_TCPNetworkEngine.GetInterface() == NULL) && (m_TCPNetworkEngine.CreateInstance() == false))
	{
		return false;
	}

	IUnknownEx * pIAttemperEngine = m_AttemperEngine.GetInterface();
	IUnknownEx * pITCPNetworkEngine = m_TCPNetworkEngine.GetInterface();

	//�ص��ӿ�
	IUnknownEx * pIAttemperEngineSink = QUERY_OBJECT_INTERFACE(m_AttemperEngineSink, IUnknownEx);

	//�󶨽ӿ�
	if (m_AttemperEngine->SetAttemperEngineSink(pIAttemperEngineSink) == false)
	{
		return false;
	}

	if (m_AttemperEngine->SetNetworkEngine(pITCPNetworkEngine) == false)
	{
		return false;
	}

	if (m_TCPNetworkEngine->SetTCPNetworkEngineEvent(pIAttemperEngine) == false)
	{
		return false;
	}

	WORD wMaxConnect = 1024;
	WORD wServicePort = 5150;
	if (m_TCPNetworkEngine->SetServiceParameter(wServicePort, wMaxConnect, szCompilation) == false)
	{
		return false;
	}

	if (m_AttemperEngine->StartService() == false)
	{
		assert(FALSE);
		return false;
	}

	if (m_TCPNetworkEngine->StartService() == false)
	{
		assert(FALSE);
		return false;
	}

	getchar();

	return 0;
}
