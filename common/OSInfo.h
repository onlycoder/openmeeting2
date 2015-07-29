#pragma once
#include <windows.h>

class COSInfo
{
public:
	COSInfo(void);
	~COSInfo(void);

	static BOOL IsWin95();
	static BOOL IsWin98();
	static BOOL IsWinMe();
	static BOOL IsWin2000();
	static BOOL IsWinXP();
	static BOOL IsAboveWinXP();
	static BOOL IsWinVista();
	static BOOL IsWin7();

private:
	static void Init();

private:
	static OSVERSIONINFOEX				m_oOSVerInfo;
	static BOOL							m_bInit;
};

