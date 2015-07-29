#ifndef __help_ini_file_h__
#define __help_ini_file_h__

#include "stdafx.h"


class FrameInifile
{
public:
    FrameInifile()
    {
		std::wstring ModulePath = GetModulePath();
		ModulePath += L"\\MeetingServer.ini";
        m_inifile = ModulePath.c_str();
    }
public:
    void Create(std::wstring inifile)
    {
        m_inifile = inifile;
    }
    std::wstring  GetString(const std::wstring &app, const std::wstring  &key, const std::wstring  &def = L"")
    {
        WCHAR buf[256] = {0};
        GetPrivateProfileStringW(app.c_str(), key.c_str(), def.c_str(), buf, 256, m_inifile.c_str());
        return buf;
    }
    void SetString(const std::wstring  &app, const std::wstring  &key, const std::wstring  &value)
    {
        WritePrivateProfileStringW(app.c_str(), key.c_str(), value.c_str(), m_inifile.c_str());
    }
    int GetInteger(const std::wstring  &app, const std::wstring  &key, int def = 0)
    {
        return (int)GetPrivateProfileIntW(app.c_str(), key.c_str(), def, m_inifile.c_str());
    }
	/*  void SetInteger(const CString &app, const CString &key, int value)
	{
	CString v;
	v.Format(L"%u", value);
	WritePrivateProfileString(app, key, v, m_inifile);
	}*/
	static std::wstring GetModulePath()
	{
		wchar_t wszAppPath[MAX_PATH] = {0};
		::GetModuleFileNameW(NULL, wszAppPath, MAX_PATH);
		std::wstring wstrAppPath = wszAppPath;
		std::wstring strTemp = wstrAppPath.substr(0, wstrAppPath.find_last_of(L'\\') + 1);

		return strTemp;
	}
private:
    std::wstring m_inifile;
};

#endif /* __INICONFIG_H__ */
