// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>
#include <tchar.h>
#include "typedef.h"

//视频相关头文件
#include "ZYMediaFrame.h"
#include "IMediaStream.h"

#include "Poco/HashMap.h"
#include "Poco/Exception.h"
#include <map>
using Poco::HashMap;

typedef HashMap<uint64_t,void*> UINT_VOID_MAP;
// TODO: 在此处引用程序需要的其他头文件

void AA_UTF82W(wchar_t* strDest, char* strUTF8, int nMaxLen);
void  AA_W2UTF8(char* strUTF8, const wchar_t* strSource, int nMaxLen);
void AA_W2A(const wchar_t* strWide, char* strAsc);
void  AA_A2W(const char* strAsc, wchar_t* strWide);

//0 中转，1，混音  2，p2p
extern uint32_t kAudioMode ;

void LogTrace(LPCTSTR pszFormat, ...);