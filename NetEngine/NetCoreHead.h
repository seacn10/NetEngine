
#define SERVICE_CORE_DLL

#ifndef SERVICE_CORE_CLASS
#ifdef  SERVICE_CORE_DLL
#define SERVICE_CORE_CLASS _declspec(dllexport)
#else
#define SERVICE_CORE_CLASS _declspec(dllimport)
#endif
#endif

#ifndef _DEBUG
#define NET_ENGINE_DLL_NAME	TEXT("NetEngine.dll")			//组件 DLL 名字
#else
#define NET_ENGINE_DLL_NAME	TEXT("NetEngineD.dll")			//组件 DLL 名字
#endif