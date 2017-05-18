#ifndef MACRO_HEAD_FILE
#define MACRO_HEAD_FILE

#include <wtypes.h>

#define INVALID_BYTE				((BYTE)(0xFF))						
#define INVALID_WORD				((WORD)(0xFFFF))					
#define INVALID_DWORD				((DWORD)(0xFFFFFFFF))				

#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))

#define INVALID_IP_ADDRESS(IPAddress) (((IPAddress==0L)||(IPAddress==INADDR_NONE)))

#ifdef _UNICODE
	#define CountStringBuffer CountStringBufferW
#else
	#define CountStringBuffer CountStringBufferA
#endif

#define CountStringBufferA(String) ((UINT)((lstrlenA(String)+1)*sizeof(CHAR)))
#define CountStringBufferW(String) ((UINT)((lstrlenW(String)+1)*sizeof(WCHAR)))

#define SafeRelease(pObject) { if (pObject!=NULL) { pObject->Release(); pObject=NULL; } }
#define SafeDelete(pData) { try { delete pData; } catch (...) { ASSERT(FALSE); } pData=NULL; } 
#define SafeCloseHandle(hHandle) { if (hHandle!=NULL) { CloseHandle(hHandle); hHandle=NULL; } }
#define SafeDeleteArray(pData) { try { delete [] pData; } catch (...) { ASSERT(FALSE); } pData=NULL; } 

inline VOID SwitchScoreFormat(LONGLONG lScore, UINT uSpace,LPCTSTR pszFormat,LPTSTR pszBuffer, WORD wBufferSize)
{
	TCHAR szSwitchScore[32]=TEXT("");
	_sntprintf(szSwitchScore,CountArray(szSwitchScore),pszFormat,lScore);

	if(uSpace>0)
	{
		
		WORD wTargetIndex=0;
		WORD wSourceIndex=0;
		WORD wSourceStringLen=0;
		
		while (szSwitchScore[wSourceStringLen]!=0 && szSwitchScore[wSourceStringLen]!='.') 
		{
			++wSourceStringLen;
		}
		
		for (INT i=0;i<wSourceStringLen;i++)
		{
			pszBuffer[wTargetIndex++]=szSwitchScore[wSourceIndex++];

			if ((wSourceStringLen-wSourceIndex>0) && ((wSourceStringLen-wSourceIndex)%uSpace==0)) 
			{
				pszBuffer[wTargetIndex++]=TEXT(',');
			}			
		}
	
		while (szSwitchScore[wSourceIndex]!=0)
		{
			pszBuffer[wTargetIndex++] = szSwitchScore[wSourceIndex++];
		}

		
		pszBuffer[wTargetIndex++]=0;
	}
	else
	{
		CopyMemory(pszBuffer,szSwitchScore,wBufferSize);
	}

	return;
}



inline VOID SwitchScoreFormat(double lScore, UINT uSpace,LPCTSTR pszFormat,LPTSTR pszBuffer, WORD wBufferSize)
{
	
	TCHAR szSwitchScore[32]=TEXT("");
	_sntprintf(szSwitchScore,CountArray(szSwitchScore),pszFormat,lScore);

	if(uSpace>0)
	{
		
		WORD wTargetIndex=0;
		WORD wSourceIndex=0;
		WORD wSourceStringLen=0;

		
		while (szSwitchScore[wSourceStringLen]!=0 && szSwitchScore[wSourceStringLen]!='.') 
		{
			++wSourceStringLen;
		}

		
		for (INT i=0;i<wSourceStringLen;i++)
		{
			
			pszBuffer[wTargetIndex++]=szSwitchScore[wSourceIndex++];

			
			if ((wSourceStringLen-wSourceIndex>0) && ((wSourceStringLen-wSourceIndex)%uSpace==0)) 
			{
				pszBuffer[wTargetIndex++]=TEXT(',');
			}			
		}

		
		while (szSwitchScore[wSourceIndex]!=0)
		{
			pszBuffer[wTargetIndex++] = szSwitchScore[wSourceIndex++];
		}

		
		pszBuffer[wTargetIndex++]=0;
	}
	else
	{
		CopyMemory(pszBuffer,szSwitchScore,wBufferSize);
	}

	return;
}



#endif
