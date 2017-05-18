#ifndef MODULE_HEAD_FILE
#define MODULE_HEAD_FILE

#include "Macro.h"



#define VER_IUnknownEx INTERFACE_VERSION(1,1)
static const GUID IID_IUnknownEx={0x5feec21e,0xdbf3,0x46f0,0x9f,0x57,0xd1,0xcd,0x71,0x1c,0x46,0xde};


interface IUnknownEx
{
	
	virtual VOID Release()=NULL;
	
	virtual VOID * QueryInterface(REFGUID Guid, DWORD dwQueryVer)=NULL;
};





#define BULID_VER					0									
#define PRODUCT_VER					6									


#define INTERFACE_VERSION(cbMainVer,cbSubVer)							\
		(DWORD)(														\
		(((BYTE)(PRODUCT_VER))<<24)+									\
		(((BYTE)(cbMainVer))<<16)+										\
		((BYTE)(cbSubVer)<<8))+											\
		((BYTE)(BULID_VER))


#define PROCESS_VERSION(cbMainVer,cbSubVer,cbBuildVer)					\
		(DWORD)(														\
		(((BYTE)(PRODUCT_VER))<<24)+									\
		(((BYTE)(cbMainVer))<<16)+										\
		((BYTE)(cbSubVer)<<8)+											\
		(BYTE)(cbBuildVer))


inline BYTE GetProductVer(DWORD dwVersion)
{
	return ((BYTE *)&dwVersion)[3];
}


inline BYTE GetMainVer(DWORD dwVersion)
{
	return ((BYTE *)&dwVersion)[2];
}


inline BYTE GetSubVer(DWORD dwVersion)
{
	return ((BYTE *)&dwVersion)[1];
}


inline BYTE GetBuildVer(DWORD dwVersion)
{
	return ((BYTE *)&dwVersion)[0];
}


inline bool InterfaceVersionCompare(DWORD dwQueryVer, DWORD dwInterfaceVer)
{
	if (GetSubVer(dwQueryVer)>GetSubVer(dwInterfaceVer)) return false;
	if (GetMainVer(dwQueryVer)!=GetMainVer(dwInterfaceVer)) return false;
	if (GetBuildVer(dwQueryVer)!=GetBuildVer(dwInterfaceVer)) return false;
	if (GetProductVer(dwQueryVer)!=GetProductVer(dwInterfaceVer)) return false;
	return true;
};





#define QUERYINTERFACE(Interface,Guid,dwQueryVer)															\
	if ((Guid==IID_##Interface)&&(InterfaceVersionCompare(dwQueryVer,VER_##Interface)))						\
		return static_cast<Interface *>(this);											


#define QUERYINTERFACE_IUNKNOWNEX(BaseInterface,Guid,dwQueryVer)											\
	if ((Guid==IID_IUnknownEx)&&(InterfaceVersionCompare(dwQueryVer,VER_IUnknownEx)))						\
		return static_cast<IUnknownEx *>(static_cast<BaseInterface *>(this));			





#define QUERY_ME_INTERFACE(Interface)																		\
	((Interface *)QueryInterface(IID_##Interface,VER_##Interface))


#define QUERY_OBJECT_INTERFACE(Object,Interface)															\
	((Interface *)Object.QueryInterface(IID_##Interface,VER_##Interface))


#define QUERY_OBJECT_PTR_INTERFACE(pObject,Interface)														\
	((pObject==NULL)?NULL:((Interface *)pObject->QueryInterface(IID_##Interface,VER_##Interface)))





typedef VOID * (ModuleCreateProc)(REFGUID Gudi, DWORD dwInterfaceVer);


template <typename IModeluInterface> class CTempldateHelper
{
	
public:
	REFGUID							m_Guid;								
	const DWORD						m_dwVersion;						

	
public:
	CHAR							m_szCreateProc[32];					
	TCHAR							m_szModuleDllName[MAX_PATH];		

	
public:
	HINSTANCE						m_hDllInstance;						
	IModeluInterface *				m_pIModeluInterface;				

	
public:
	TCHAR							m_szDescribe[128];					

	
public:
	
	CTempldateHelper(REFGUID Guid, DWORD dwVersion);
	
	CTempldateHelper(REFGUID Guid, DWORD dwVersion, LPCTSTR pszModuleDll, LPCSTR pszCreateProc);
	
	virtual ~CTempldateHelper();

	
public:
	
	bool CloseInstance();
	
	bool CreateInstance();

	
public:
	
	VOID SetModuleCreateInfo(LPCTSTR pszModuleDllName, LPCSTR pszCreateProc);

	
public:
	
	inline LPCTSTR GetErrorDescribe() const;
	
	inline IModeluInterface * operator->() const;
	
	inline IModeluInterface * GetInterface() const;
};





template <typename IModeluInterface>
CTempldateHelper<IModeluInterface>::CTempldateHelper(REFGUID Guid, DWORD dwVersion) : m_dwVersion(dwVersion), m_Guid(Guid)
{
	
	m_szDescribe[0]=0;

	
	m_hDllInstance=NULL;
	m_pIModeluInterface=NULL;

	
	ZeroMemory(m_szCreateProc,sizeof(m_szCreateProc));
	ZeroMemory(m_szModuleDllName,sizeof(m_szModuleDllName));

	return;
}


template <typename IModeluInterface>
CTempldateHelper<IModeluInterface>::CTempldateHelper(REFGUID Guid, DWORD dwVersion, LPCTSTR pszModuleDll, LPCSTR pszCreateProc) : m_dwVersion(dwVersion), m_Guid(Guid)
{
	
	m_szDescribe[0]=0;

	
	m_hDllInstance=NULL;
	m_pIModeluInterface=NULL;

	
	lstrcpynA(m_szCreateProc,pszCreateProc,CountArray(m_szCreateProc));
	lstrcpyn(m_szModuleDllName,pszModuleDll,CountArray(m_szModuleDllName));

	return;
}


template <typename IModeluInterface>
CTempldateHelper<IModeluInterface>::~CTempldateHelper()
{
	CloseInstance();
}


template <typename IModeluInterface>
bool CTempldateHelper<IModeluInterface>::CreateInstance()
{
	
	CloseInstance();

	
	try
	{
		m_hDllInstance=LoadLibrary(m_szModuleDllName);
		if (m_hDllInstance==NULL) 
		{
			_sntprintf(m_szDescribe,CountArray(m_szDescribe),TEXT("“%s”模块加载失败"),m_szModuleDllName);
			return false;
		}

		
		ModuleCreateProc * CreateProc=(ModuleCreateProc *)GetProcAddress(m_hDllInstance,m_szCreateProc);
		if (CreateProc==NULL) 
		{
			_sntprintf(m_szDescribe,CountArray(m_szDescribe),TEXT("找不到组件创建函数“%s”"),m_szCreateProc);
			return false;
		}

		
		m_pIModeluInterface=(IModeluInterface *)CreateProc(m_Guid,m_dwVersion);
		if (m_pIModeluInterface==NULL) 
		{
			_sntprintf(m_szDescribe,CountArray(m_szDescribe),TEXT("调用函数“%s”生成对象失败"),m_szCreateProc);
			return false;
		}
	}
	catch (LPCTSTR pszError)
	{
		_sntprintf(m_szDescribe,CountArray(m_szDescribe),TEXT("由于“%s”，组件创建失败"),pszError);
		return false;
	}
	catch (...)	
	{ 
		_sntprintf(m_szDescribe,CountArray(m_szDescribe),TEXT("组件创建函数“%s”产生未知异常错误，组件创建失败"),m_szCreateProc);
		return false;
	}

	return true;
}


template <typename IModeluInterface>
bool CTempldateHelper<IModeluInterface>::CloseInstance()
{
	
	m_szDescribe[0]=0;

	
	if (m_pIModeluInterface!=NULL)
	{
		m_pIModeluInterface->Release();
		m_pIModeluInterface=NULL;
	}

	
	if (m_hDllInstance!=NULL)
	{
		FreeLibrary(m_hDllInstance);
		m_hDllInstance=NULL;
	}

	return true;
}


template <typename IModeluInterface>
VOID CTempldateHelper<IModeluInterface>::SetModuleCreateInfo(LPCTSTR pszModuleDllName, LPCSTR pszCreateProc)
{
	
	lstrcpynA(m_szCreateProc,pszCreateProc,CountArray(m_szCreateProc));
	lstrcpyn(m_szModuleDllName,pszModuleDllName,CountArray(m_szModuleDllName));

	return;
}

template <typename IModeluInterface>
inline LPCTSTR CTempldateHelper<IModeluInterface>::GetErrorDescribe() const
{ 
	return m_szDescribe; 
}


template <typename IModeluInterface>
inline IModeluInterface * CTempldateHelper<IModeluInterface>::operator->() const
{ 
	return GetInterface(); 
}


template <typename IModeluInterface>
inline IModeluInterface * CTempldateHelper<IModeluInterface>::GetInterface() const
{ 
	return m_pIModeluInterface; 
}





#define DECLARE_CREATE_MODULE(OBJECT_NAME)																	\
extern "C" __declspec(dllexport) VOID * Create##OBJECT_NAME(REFGUID Guid, DWORD dwInterfaceVer)		\
{																											\
	C##OBJECT_NAME * p##OBJECT_NAME=NULL;																	\
	try																										\
	{																										\
		p##OBJECT_NAME=new C##OBJECT_NAME();																\
		if (p##OBJECT_NAME==NULL) throw TEXT("创建失败");													\
		VOID * pObject=p##OBJECT_NAME->QueryInterface(Guid,dwInterfaceVer);									\
		if (pObject==NULL) throw TEXT("接口查询失败");														\
		return pObject;																						\
	}																										\
	catch (...) {}																							\
	SafeDelete(p##OBJECT_NAME);																				\
	return NULL;																							\
}


#define DECLARE_MODULE_DYNAMIC(OBJECT_NAME)																	\
class C##OBJECT_NAME##Helper : public CTempldateHelper<I##OBJECT_NAME>										\
{																											\
public:																										\
	C##OBJECT_NAME##Helper() : CTempldateHelper<I##OBJECT_NAME>(IID_I##OBJECT_NAME,VER_I##OBJECT_NAME) { }	\
};


#define DECLARE_MODULE_HELPER(OBJECT_NAME,MODULE_DLL_NAME,CREATE_FUNCTION_NAME)								\
class C##OBJECT_NAME##Helper : public CTempldateHelper<I##OBJECT_NAME>										\
{																											\
public:																										\
	C##OBJECT_NAME##Helper() : CTempldateHelper<I##OBJECT_NAME>(IID_I##OBJECT_NAME,							\
		VER_I##OBJECT_NAME,MODULE_DLL_NAME,CREATE_FUNCTION_NAME) { }										\
};



#endif
