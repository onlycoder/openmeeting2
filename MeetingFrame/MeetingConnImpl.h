#pragma once
#include <time.h>
#include <math.h>

#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include "Poco/ThreadTarget.h"
#include "Poco/Event.h"
#include "Poco/Timestamp.h"
#include "Poco/Timespan.h"
#include "Poco/HashMap.h"
#include "Poco/Exception.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/NetException.h"
#include "Poco/Timespan.h"
#include "Poco/Stopwatch.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Query.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Handler.h"
#include "Poco/JSON/Template.h"
#include "Poco/JSON/Array.h"
#include "Poco/Net/HTTPCookie.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include <map>
#include <queue>
#include <vector>

using namespace Poco::JSON;
using namespace Poco::Dynamic;
using Poco::Dynamic::Array;
using Poco::File;
using Poco::FileStream;
using Poco::Net::HTTPCookie;
using namespace std;
using Poco::Net::Socket;
using Poco::Net::SocketStream;
using Poco::Net::StreamSocket;
using Poco::Net::ServerSocket;
using Poco::Net::SocketAddress;
using Poco::Net::ConnectionRefusedException;
using Poco::Net::NetException;
using Poco::Timespan;
using Poco::Stopwatch;
using Poco::TimeoutException;
using Poco::InvalidArgumentException;
using Poco::HashMap;
using namespace Poco;
using Poco::Thread;
using Poco::Runnable;
using Poco::ThreadTarget;
using Poco::Event;

using namespace std;
#include "IMeetingFrame.h"

class MeetingConnImpl : public Runnable
{
private:
	MeetingConnImpl(void);
public:
	~MeetingConnImpl(void);
public:
	static MeetingConnImpl * GetInstance();
	static void ReleaseALL(){delete m_instance;}
private:
	static MeetingConnImpl * m_instance;
public:
	bool Connect(char* strServer,IMeetingFrameEvent* pIMeetingEvent);
	void Release();
	//用帐号，密码 登入
	void Login(const char* strUserName, const char* strPassword,uint32_t clientType,bool bEncrypt = true);
	//用房间号登录
	void LoginWithRoomNo(const char* strUserName, const char* strRoomPassword,uint32_t clientType,uint32_t u32RoomID);

	void SendTextMsg(uint64_t toSessionID,char * msg,char* fontName="",uint32_t fontSize = 14,uint32_t color = 0,uint64_t flag = 0);

	void TransParentRoomCommand(uint64_t toUserID,char* strSubCommand);
	//登出
	void Logout();
	//是否连接
	bool IsConnect();
	
	//得到房间用户列表
	void GetRoomUserList(uint32_t roomID);
	//得到房间信息
	void GetRoomInfo(uint32_t roomID);
	//得到聊天历史记录
	void GetChatHistory(uint32_t roomID);
	//进入房间
	void EnterRoom(uint32_t roomID);
	//离开房间
	void LeaveRoom();

	virtual void ReConnect();

	uint32_t GetRoomID(){return m_ulRoomID;};

	void MeetingConnImpl::GetRoomList();
	PROOM_INFO GetRoomInfo(int index);

	void SetAsAdmin(char* strCmd);

	void SendCmd(char* strCmd);
	
	void SetSpeakMode(char* strCmd);

	void SetRoomMode(char* strCmd);
	//得到聊天消息
	PMsgItem GetChatMsgItem(int index);
private:
	//拼信令
	void ProcessRawCommand( char* buffer,uint32_t nLen);
	//处理接收到的 json字符串
	void ProcessJsonCommand(char* buffer,Poco::Int32 nLen);
	std::string IMUnescape(const std::string& str);
	//第一次握手
	void OnHandSharke1(Object::Ptr object);
	//第二次握手
	void OnHandSharke2(Object::Ptr object);
	void OnLogin(Object::Ptr object);
	void OnLoginWithRoomNo(Object::Ptr object);
	void OnLogout(Object::Ptr object);
	void OnGetRoomUserList(Object::Ptr object);
	
	void OnGetRoomList(Object::Ptr object);
	void OnEnterRoom(Object::Ptr object);
	void OnTransparentCmd(Object::Ptr object);
	void OnGetRoomInfo(Object::Ptr object);
	void OnLeaveRoom(Object::Ptr object);
	void OnTextChat(Object::Ptr object);
	void OnGetChatHistory(Object::Ptr object);
public:
	virtual void run() ;    //线程主体
public:
	//用户信息
	char m_strUserAccount[64];
	char m_strUserName[64];
	char m_strPassword[64];
	char m_strHeadImg[256];
	uint32_t m_userRole;  //用户角色
	uint64_t m_mySessionID;
	uint32_t m_ulRoomID;         //房间ID
	Poco::UInt16 m_loginType;
private:
	//是否停止
	bool m_bStop;
	//接收数据线程
	Thread thread;
	char m_TempBuf[1500];
	bool m_bHasLogined;                //是否已登录
	bool m_bHasLoginRoom;              //是否已进入房间
	//tcp连接
	StreamSocket *m_pSock;
	bool m_bRawSockConnected;      //物理连接已成功
	bool m_bLogicServerConnected;  //三次握手已成功
	char *m_TempRecvBuf;
	uint32_t  m_recvLen;
	IMeetingFrameEvent * m_pIMeetingEvent;
	Timestamp m_lastTime;
	//登录服务器地址
	char m_strServer[64];
	//终端类型
	uint32_t m_clientType;
	FileStream* m_pLogFile;
	
	uint16_t  m_tryLoginCount;
	Poco::Timestamp  m_lastLoginTime;
	char m_strLogin[1500];
	queue<char*> m_sendBufList;
	//锁
	FastMutex m_lock;

	vector<PROOM_INFO> m_pRoomList;
	vector<PMsgItem> m_chatMsgList;  //聊天消息列表
};

