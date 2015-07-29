// stdafx.cpp : 只包括标准包含文件的源文件
// MeetingFrame.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中
// 引用任何所需的附加头文件，而不是在此文件中引用
#pragma  comment(lib,"Ws2_32.lib")
#ifdef _DEBUG
//#pragma comment(lib,"..//libs//Debug//Raknet.lib")
#pragma comment(lib,"ZYMediaFrame.lib")
#pragma comment(lib,"ZYMediaStream.lib")
#else
//#pragma comment(lib,"../libs/Release/Raknet.lib")
#pragma comment(lib,"ZYMediaFrame.lib")
#pragma comment(lib,"ZYMediaStream.lib")
#endif

uint32_t kAudioMode = 0;
void  AA_A2W(const char* strAsc, wchar_t* strWide)
{
	int	nLen = MultiByteToWideChar(CP_ACP, 0, strAsc, strlen(strAsc), NULL, 0); 

	lstrcpy(strWide, _T(""));
	MultiByteToWideChar(CP_ACP, 0, strAsc, strlen(strAsc), strWide, nLen); 
	strWide[nLen] =_T( '\0');
}

void AA_W2A(const wchar_t* strWide, char* strAsc)
{
	int	nSourceLen = wcslen(strWide);

	strcpy(strAsc, "");
	int	nLen = WideCharToMultiByte(CP_ACP, 0, strWide, nSourceLen, NULL, 0, NULL, NULL);

	WideCharToMultiByte(CP_ACP, 0, strWide, nSourceLen, strAsc, nLen, NULL, NULL);
	strAsc[nLen] = '\0';
}

void  AA_W2UTF8(char* strUTF8, const wchar_t* strSource, int nMaxLen)
{
	int	nSourceLen = wcslen(strSource);

	if (nSourceLen > nMaxLen)
		nSourceLen = nMaxLen;

	memset(strUTF8, 0, nMaxLen);
	int	nLen = WideCharToMultiByte(CP_UTF8, 0, strSource, nSourceLen, NULL, 0, NULL, NULL);

	WideCharToMultiByte(CP_UTF8, 0, strSource, nSourceLen, strUTF8, nLen, NULL, NULL); 
}

void AA_UTF82W(wchar_t* strDest, char* strUTF8, int nMaxLen)
{
	int	nLen = MultiByteToWideChar(CP_UTF8, 0, strUTF8, strlen(strUTF8), NULL, 0); 

	if (nLen > nMaxLen)
		nLen = nMaxLen;

	memset(strDest, 0, nMaxLen*sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, strUTF8, strlen(strUTF8), strDest, nMaxLen); 
}

void LogTrace(LPCTSTR pszFormat, ...)  
{  
//#ifdef _DEBUG
	va_list pArgs;  
	TCHAR szMessageBuffer[1024*10]={0};  
	va_start( pArgs, pszFormat );  
	_vsntprintf( szMessageBuffer, 16380, pszFormat, pArgs );  
	va_end( pArgs );  
	::OutputDebugStringW(szMessageBuffer);
//#endif
} 