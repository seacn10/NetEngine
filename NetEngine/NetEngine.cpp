// NetEngine.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "NetEngine.h"

static AFX_EXTENSION_MODULE	KernelEngineDLL = { NULL, NULL };

extern "C" INT APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (!AfxInitExtensionModule(KernelEngineDLL, hInstance)) return 0;
		new CDynLinkLibrary(KernelEngineDLL);

		CoInitialize(NULL);

		WSADATA WSAData;
		if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) return FALSE;
	}
	else if (dwReason == DLL_THREAD_ATTACH)
	{
		CoInitialize(NULL);
	}
	else if (dwReason == DLL_THREAD_DETACH)
	{
		CoUninitialize();
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		CoUninitialize();

		WSACleanup();

		AfxTermExtensionModule(KernelEngineDLL);
	}

	return 1;
}