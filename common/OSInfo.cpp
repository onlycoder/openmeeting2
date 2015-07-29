#include "OSInfo.h"

OSVERSIONINFOEX COSInfo::m_oOSVerInfo = {0};
BOOL COSInfo::m_bInit = FALSE;

COSInfo::COSInfo(void)
{
}

COSInfo::~COSInfo(void)
{
}

BOOL COSInfo::IsWin95()
{
	Init();
	return (m_oOSVerInfo.dwMajorVersion == 4 && m_oOSVerInfo.dwMinorVersion == 0);
}

BOOL COSInfo::IsWin98()
{
	Init();
	return (m_oOSVerInfo.dwMajorVersion == 4 && m_oOSVerInfo.dwMinorVersion == 10);
}

BOOL COSInfo::IsWinMe()
{
	Init();
	return (m_oOSVerInfo.dwMajorVersion == 4 && m_oOSVerInfo.dwMinorVersion == 90);
}

BOOL COSInfo::IsWin2000()
{
	Init();
	return (m_oOSVerInfo.dwMajorVersion == 5 && m_oOSVerInfo.dwMinorVersion == 0);
}

BOOL COSInfo::IsWinXP()
{
	Init();
	return (m_oOSVerInfo.dwMajorVersion == 5 && m_oOSVerInfo.dwMinorVersion == 1);
}

BOOL COSInfo::IsAboveWinXP()
{
	Init();
	return (m_oOSVerInfo.dwMajorVersion > 5);
}

BOOL COSInfo::IsWinVista()
{
	Init();
	return (m_oOSVerInfo.dwMajorVersion == 6 && m_oOSVerInfo.dwMinorVersion == 0);
}

BOOL COSInfo::IsWin7()
{
	Init();
	return (m_oOSVerInfo.dwMajorVersion == 6 && m_oOSVerInfo.dwMinorVersion == 1);
}
///////////////////////////////////////////////////////
void COSInfo::Init()
{
	if(m_bInit)
		return;

	m_bInit = TRUE;

	memset(&m_oOSVerInfo, 0, sizeof(OSVERSIONINFOEX));
    m_oOSVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if(!GetVersionEx((OSVERSIONINFO*)&m_oOSVerInfo))
	{
		// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
		m_oOSVerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO*)&m_oOSVerInfo)) 
			return;
	}
}