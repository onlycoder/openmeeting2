#include "StdAfx.h"
#include "MeetingConnImpl.h"
#include "Poco/Timestamp.h"
#include "Poco/MD5Engine.h"
#include "MeetingFrameImpl.h"

using Poco::MD5Engine;
using Poco::DigestEngine;
using Poco::Timestamp;
#define BUF_LEN 1024*1024
MeetingConnImpl *MeetingConnImpl::	m_instance = NULL;
MeetingConnImpl *MeetingConnImpl::GetInstance(void)
{
	if(m_instance == NULL)
	{
		m_instance = new MeetingConnImpl();
		::atexit(ReleaseALL);
	}
	return m_instance;
}

MeetingConnImpl::MeetingConnImpl(void)
{
	m_ulRoomID = 1;
	m_mySessionID = 0;
	m_userRole = 4;
	m_bHasLogined = false;
	m_bHasLoginRoom = false;
	m_bLogicServerConnected = false;
	m_bRawSockConnected = false;
	m_TempRecvBuf = new char[BUF_LEN];
	memset(m_strHeadImg,0,sizeof(m_strHeadImg));
	memset(m_strUserAccount,0,sizeof(m_strUserAccount));
	memset(m_strUserName,0,sizeof(m_strUserName));
	memset(m_strServer,0,sizeof(m_strServer));
	memset(m_TempRecvBuf,0,sizeof(m_TempRecvBuf));
	memset(m_strLogin,0,sizeof(m_strLogin));
	m_recvLen = 0;
	m_pSock = NULL;
	m_mySessionID = 0;
	//m_pLogFile = new FileStream("c:\\command.txt");
	m_bStop = false;
	m_pIMeetingEvent = NULL;
}


MeetingConnImpl::~MeetingConnImpl(void)
{
	if(m_TempRecvBuf)
	{
		delete []m_TempRecvBuf;
		m_TempRecvBuf = NULL;
	}

	//释放这个指针的时候要判断线程是否已经结束了
	while(thread.isRunning())
	{
		Thread::sleep(10);
	}

	if(m_pSock) 
	{
		delete m_pSock;
		m_pSock = NULL;
	}
}


void MeetingConnImpl::ReConnect()
{
	if(thread.isRunning()||m_bRawSockConnected == true || m_bLogicServerConnected == true ||m_bHasLogined == true )
		return;
	m_pSock = new StreamSocket();
	try
	{
		if(m_pSock)
		{
			m_pSock->connectNB(SocketAddress(m_strServer));
		}
	}
	catch (ConnectionRefusedException&)
	{
		m_pIMeetingEvent->OnNetEvent(NET_EVENT_TIME_OUT,"can't connect to login server");
		return ;
	}
	catch(NetException&)
	{
		m_pIMeetingEvent->OnNetEvent(NET_EVENT_TIME_OUT,"can't connect to login server");
		return ;
	}
	catch(IOException&)
	{
		m_pIMeetingEvent->OnNetEvent(NET_EVENT_TIME_OUT,"can't connect to login server");
		return ;
	}
	/* 启动信令接收线程 */
	m_bStop = false;
	thread.start(*this);
}

long filesize(FILE*stream)
{
	long curpos,length;
	curpos=ftell(stream);
	fseek(stream,0L,SEEK_END);
	length=ftell(stream);
	fseek(stream,curpos,SEEK_SET);
	return length;
}

bool MeetingConnImpl::Connect(char* strServer,IMeetingFrameEvent* pIMeetingEvent)
{
	if(thread.isRunning())
		return false;
	m_bRawSockConnected = false;
	if(pIMeetingEvent == NULL)
		return false;
	m_pIMeetingEvent = pIMeetingEvent;
	if(NULL != m_pSock)
	{
		m_pSock->close();
		delete m_pSock;
		m_pSock = NULL;
	}
	m_pSock = new StreamSocket();
	strcpy(m_strServer,strServer);
	try
	{
		if(m_pSock)
		{
			m_pSock->connectNB(SocketAddress(strServer));
		}
	}
	catch (ConnectionRefusedException&)
	{
		m_pIMeetingEvent->OnNetEvent(NET_EVENT_TIME_OUT,"can't connect to login server");
		goto OnError;
	}
	catch(NetException&)
	{
		m_pIMeetingEvent->OnNetEvent(NET_EVENT_TIME_OUT,"can't connect to login server");
		goto OnError;
	}
	catch(IOException&)
	{
		m_pIMeetingEvent->OnNetEvent(NET_EVENT_TIME_OUT,"can't connect to login server");
		goto OnError;
	}
	/* 启动信令接收线程 */
	m_bStop = false;
	thread.start(*this);
	return true;
OnError:
	m_pSock->close();
	delete m_pSock;
	m_pSock = NULL;
	return false;
}
void MeetingConnImpl::Release()
{
	m_bStop = true;
	m_bHasLogined = false;
	m_bHasLoginRoom = false;
	thread.join();
	m_bLogicServerConnected = m_bRawSockConnected = false;
}
//用房间号登录，呢称，房间密码，终端类型
void MeetingConnImpl::LoginWithRoomNo(const char* strUserName, const char* strRoomPassword,uint32_t clientType,uint32_t u32RoomID)
{
	//md5
	m_tryLoginCount = 0;
	m_lastLoginTime = Timestamp();
	m_loginType = 2;
	m_userRole = 2;
	m_ulRoomID = u32RoomID;
	//MD5Engine engine;
	//engine.update(strRoomPassword);
	strcpy(m_strUserAccount, strUserName);
	strcpy(m_strUserName, strUserName);
	strcpy(m_strPassword,strRoomPassword);
	
	m_clientType = clientType;
	if(m_bHasLogined == true)
		return;
	m_bHasLogined = false;
	 char utfStr[1024]={0};
	sprintf(utfStr, "{\"cmd\":\"loginWithRoomNo\",\"roomID\":%u,\"password\":\"%s\",\"client_type\":%u}\r\n",u32RoomID,m_strPassword,clientType);
	strcpy(m_strLogin,utfStr);

}
//用户帐号，密码登入
void MeetingConnImpl::Login(const char* strUserName, const char* strPassword,uint32_t clientType,bool bEncrypt)
{
	/*
	std::string strTest="[18621066138,18979080001]";
	std::string strTest2 = strTest;
	Parser parser;
	Var result;
	try
	{
		result = parser.parse(strTest2);
		Poco::JSON::Array::Ptr array =result.extract<Poco::JSON::Array::Ptr>();
		for(Poco::Int32 i = 0; i<array->size();i++)
		{
			Var item = array->get(i);
			unsigned long long phone = item;
			cout<<phone;
		}
	}
	catch(JSONException& jsone)
	{
		//printf("%s \n",buffer);
		std::cout << jsone.message() << std::endl;
		return;
	}
	*/
	//md5
	
	m_loginType = 1;
	m_userRole  = 2;
	m_tryLoginCount = 0;
	m_lastLoginTime = Timestamp();
	if(bEncrypt)
	{
		MD5Engine engine;
		engine.update(strPassword);
		strcpy(m_strUserAccount, strUserName);
		strcpy(m_strPassword, DigestEngine::digestToHex(engine.digest()).data());
	}
	else
	{
		strcpy(m_strUserAccount, strUserName);
		strcpy(m_strPassword,strPassword);
	}
	m_clientType = clientType;
	if(m_bHasLogined == true)
		return;
	m_bHasLogined = false;
	char utfStr[1024]={0};
	sprintf(utfStr, "{\"cmd\":\"login\",\"account\":\"%s\",\"password\":\"%s\",\"client_type\":%u}\r\n",m_strUserAccount,m_strPassword,clientType);
	strcpy(m_strLogin,utfStr);
	
}
//登出
void MeetingConnImpl::Logout()
{
	
	char * msg = new char[64];
	strcpy(msg,"{\"cmd\":\"logout\"}\r\n");
	FastMutex::ScopedLock autoLock(m_lock);
	m_sendBufList.push(msg);	
	
}

void MeetingConnImpl::SendTextMsg(uint64_t toSessionID,char * msg,char* fontName,uint32_t fontSize,uint32_t color,uint64_t flag)
{
	std::string msg2 = HTTPCookie::escape(msg);
	std::string strFont = HTTPCookie::escape(fontName);
	char *utfStr = new char[1024*6];
	if(fontName == NULL)
		fontName = "";
	sprintf(utfStr,"{\"cmd\":\"textChat\",\"fromSessionID\":%I64u,\"fromUserName\":\"%s\",\"toSessionID\":%I64u,\"msg\":\"%s\",\"fontSize\":%u,\"color\":%u,\"flag\":%I64u,\"fontName\":\"%s\",\"roomID\":%u}\r\n",
		m_mySessionID,m_strUserName,toSessionID,msg2.data(),fontSize,color,flag,strFont.data(),m_ulRoomID);
	FastMutex::ScopedLock autoLock(m_lock);
	PMsgItem pMsgItem = new MsgItem();
	pMsgItem->ulUserID = toSessionID;
	pMsgItem->font_size = fontSize;
	pMsgItem->color = color;
	pMsgItem->fontFlat = flag;
	strcpy(pMsgItem->userName,this->m_strUserName);
	strcpy(pMsgItem->msg,msg);
	strcpy(pMsgItem->familly,fontName);
	//插入到列表
	m_chatMsgList.push_back(vector<PMsgItem>::value_type(pMsgItem));
	m_sendBufList.push(utfStr);
}
//是否连接
bool MeetingConnImpl::IsConnect()
{
	return thread.isRunning();
}

//得到房间列表
void MeetingConnImpl::GetRoomList()
{
	char *utfStr = new char[64];
	sprintf(utfStr, "{\"cmd\":\"getRoomList\"}\r\n");
	FastMutex::ScopedLock autoLock(m_lock);
	m_sendBufList.push(utfStr);
}
//得到房间用户列表
void MeetingConnImpl::GetRoomUserList(uint32_t roomID)
{
	char *utfStr = new char[64];
	sprintf(utfStr, "{\"cmd\":\"getRoomUserList\",\"roomID\":%u}\r\n",roomID);
	FastMutex::ScopedLock autoLock(m_lock);
	m_sendBufList.push(utfStr);
	
}
//得到房间信息
void MeetingConnImpl::GetRoomInfo(uint32_t roomID)
{
	char *utfStr = new char[128];
	sprintf(utfStr, "{\"cmd\":\"getRoomInfo\",\"roomID\":%u}\r\n",roomID);
	FastMutex::ScopedLock autoLock(m_lock);
	m_sendBufList.push(utfStr);
	
}
//得到聊天历史记录
void MeetingConnImpl::GetChatHistory(uint32_t roomID)
{
	char *utfStr = new char[128];
	sprintf(utfStr, "{\"cmd\":\"getChatHistory\",\"roomID\":%u}\r\n",roomID);
	FastMutex::ScopedLock autoLock(m_lock);
	m_sendBufList.push(utfStr);
	
}
//透明消息
void MeetingConnImpl::TransParentRoomCommand(uint64_t toSessionID,char* strSubCommand)
{
	char *utfStr = new char[1024*4];
	sprintf(utfStr,"{\"cmd\":\"transparentCmd\",\"fromSessionID\":%I64u,\"toSessionID\":%I64u,\"roomID\":%u,\"fromUserName\":\"%s\",\"subCmd\":%s}\r\n",
		m_mySessionID,toSessionID,m_ulRoomID,m_strUserAccount,strSubCommand);
	FastMutex::ScopedLock autoLock(m_lock);
	m_sendBufList.push(utfStr);
}

void MeetingConnImpl::SetAsAdmin(char* strCmd)
{
	char *utfStr = new char[1024];
	sprintf(utfStr,"%s\r\n",strCmd);
	FastMutex::ScopedLock autoLock(m_lock);
	m_sendBufList.push(utfStr);
}

void MeetingConnImpl::SendCmd(char* strCmd)
{
	char *utfStr = new char[1024];
	sprintf(utfStr,"%s\r\n",strCmd);
	FastMutex::ScopedLock autoLock(m_lock);
	m_sendBufList.push(utfStr);
}

void MeetingConnImpl::SetSpeakMode(char* strCmd)
{
	char *utfStr = new char[1024];
	sprintf(utfStr,"%s\r\n",strCmd);
	FastMutex::ScopedLock autoLock(m_lock);
	m_sendBufList.push(utfStr);
}

void MeetingConnImpl::SetRoomMode(char* strCmd)
{
	char *utfStr = new char[1024];
	sprintf(utfStr,"%s\r\n",strCmd);
	FastMutex::ScopedLock autoLock(m_lock);
	m_sendBufList.push(utfStr);
}

//进入房间
void MeetingConnImpl::EnterRoom(uint32_t roomID)
{
	char *utfStr = new char[1024];
	m_ulRoomID = roomID;
	sprintf(utfStr, "{\"cmd\":\"enterRoom\",\"roomID\":%u,\"userName\":\"%s\",\"userRole\":%u,\"clientType\":%u}\r\n",
		roomID,m_strUserName,m_userRole,1);
	FastMutex::ScopedLock autoLock(m_lock);
	m_sendBufList.push(utfStr);
}
//离开房间
void MeetingConnImpl::LeaveRoom()
{
	char *utfStr = new char[1024];
	sprintf(utfStr, "{\"cmd\":\"leaveRoom\",\"roomID\":%u}\r\n",m_ulRoomID);
	FastMutex::ScopedLock autoLock(m_lock);
	m_sendBufList.push(utfStr);
}
void MeetingConnImpl::run()
{
	Poco::Timespan span(2*1000);
	char buffer[1500*10] = {0};
	SocketAddress sender;
	Poco::Int32 n;
	m_recvLen = 0;
	memset(m_TempRecvBuf,0,sizeof(m_TempRecvBuf));
	//探测是否连接成功;
	Poco::Timespan spanTimeOut(10,0);
	Socket::SocketList readList;
	Socket::SocketList writeList;
	Socket::SocketList errList;

	Timestamp timeHeart;
	Timestamp timeTimeout;
	Timespan errorSpan(1000);
	span = Timespan(1000*100);

	writeList.push_back(*m_pSock);
	errList.push_back(*m_pSock);

	if(Socket::select(readList,writeList,errList,spanTimeOut))
	{
		if(writeList.size()>0)
		{
			m_bRawSockConnected = true;
			m_pIMeetingEvent->OnNetEvent(NET_EVENT_CONNECT_SUCCESS,"connect success");
			
		}
		else
		{
			//连接不上
			m_pIMeetingEvent->OnNetEvent(NET_EVENT_TIME_OUT,"connect login server time out");
			goto OnError;
		}
	}
	else
	{

		m_pIMeetingEvent->OnNetEvent(NET_EVENT_TIME_OUT,"connect login server time out");
		goto OnError;
	}

	if(m_bRawSockConnected)
	{
		//发送握手协议1
		char msg[512];
		sprintf(msg,"{\"cmd\":\"handSharke1\",\"cmdVar\":%d}\r\n",MeetingFrameImpl::GetInstance()->GetVersion());
		try
		{
			if(m_bRawSockConnected)
				m_pSock->sendBytes(msg,strlen(msg));
		}
		catch(NetException&)
		{
			m_bRawSockConnected = false;
			m_bStop = true;
			m_pIMeetingEvent->OnNetEvent(NET_EVENT_LOST_CONNECTION,"server has a error");
			goto OnError;
		}
	}

	
	m_recvLen = 0;
	m_lastTime = 0;

	while(!m_bStop &&m_bRawSockConnected)
	{
		char c = '\r';
		char c2 = '\n';
		if(!m_bStop && m_pSock->poll(errorSpan,Socket::SELECT_ERROR))
		{
			if(m_bHasLogined)
			{
				m_bRawSockConnected = false;
				m_pIMeetingEvent->OnNetEvent(NET_EVENT_LOST_CONNECTION,"Proxy Server Error");
			}
			goto OnError;
		}

		if(!m_bStop && m_pSock->poll(span, Socket::SELECT_READ))
		{
			try
			{
				if(m_pSock->available()<=0)
				{
					if(m_bHasLogined)
					{
						m_bRawSockConnected = false;
						m_pIMeetingEvent->OnNetEvent(NET_EVENT_LOST_CONNECTION,"Proxy Server Error");
					}
					goto OnError;
				}

				if(m_bStop)
				{
					m_bHasLogined = m_bLogicServerConnected = m_bRawSockConnected = false;
					return;
				}

				memset(buffer,0,sizeof(buffer));
				n = m_pSock->receiveBytes(buffer,sizeof(buffer));
				m_lastTime = Timestamp();
				if(n>0)
				{
					//切包，拼包
					ProcessRawCommand(buffer,n);
				}
				else
				{
					if(!m_bStop)
						m_pIMeetingEvent->OnNetEvent(NET_EVENT_LOST_CONNECTION,"lost connection");
					goto OnError;
				}
			}
			catch (Poco::Exception& exc)
			{
				std::cerr << "AA_proxy_Server: " << exc.displayText() << std::endl;
				if(!m_bStop)
					m_pIMeetingEvent->OnNetEvent(NET_EVENT_LOST_CONNECTION,"proxy server error");
				goto OnError;
				return;
			}
		}
		//一分钟发一次心跳
		if(timeHeart.elapsed()>30*1000*1000)
		{
			char *msg2="{\"cmd\":\"ping\"}\r\n";
			if(m_bLogicServerConnected)
				m_pSock->sendBytes(msg2,strlen(msg2));
			timeHeart = Timestamp();
		}
		// 70秒没有收到数据，掉线了;
		if(timeTimeout.elapsed() >30*1000*1000)
		{  
			if(m_lastTime.elapsed()>35*1000*1000)
			{
				m_pIMeetingEvent->OnNetEvent(NET_EVENT_LOST_CONNECTION,"lost connection");
				goto OnError;
			}
			timeTimeout = Timestamp();
		}

		if(m_bHasLogined == true && m_bLogicServerConnected == true)
		{
			//如果已经登录成功，检测缓冲区有没有要发送的数据，发送数据
			{
				FastMutex::ScopedLock autoLock(m_lock);
				while(m_sendBufList.size()>0)
				{
					char * strSend = m_sendBufList.front();
					if(m_pSock!=NULL)
					{
						m_pSock->sendBytes(strSend,strlen(strSend));
						m_sendBufList.pop();
						delete strSend;
					}
					else
					{
						break;
					}
					
				}
			}
		}
		else if(m_tryLoginCount<5)
		{
			//发送登录数据
			if(m_bLogicServerConnected == true)
			{
				if(m_tryLoginCount == 0)
				{
					m_pSock->sendBytes(m_strLogin,strlen(m_strLogin));
					m_tryLoginCount++;
					m_lastLoginTime = Timestamp();
				}
				else
				{
					if(m_lastLoginTime.elapsed() >1000*5*1000)
					{
						m_pSock->sendBytes(m_strLogin,strlen(m_strLogin));
						m_tryLoginCount++;
						m_lastLoginTime = Timestamp();
					}
				}
			}
			
		}
	}//end while

	//把没有发送的数据发送完
	{
		FastMutex::ScopedLock autoLock(m_lock);
		while(m_sendBufList.size()>0)
		{
			char * strSend = m_sendBufList.front();
			if(m_pSock!=NULL)
			{
				m_pSock->sendBytes(strSend,strlen(strSend));
				m_sendBufList.pop();
				delete strSend;
			}
			else
			{
				break;
			}

		}
	}

OnError:
	if(m_bLogicServerConnected && NULL != m_pSock)
	{
		m_pSock->shutdown();
	}
	m_bStop  = true;
	m_bHasLogined = false;
	m_bLogicServerConnected = m_bRawSockConnected= false;
	delete m_pSock;
	m_pSock = NULL;
}

void MeetingConnImpl::ProcessRawCommand( char* buffer,uint32_t nLen)
{	
	char * pos = buffer;
	Poco::Int32 lastLen = 0;
	// 查找/r/n ,把信令切分开来;
	for (Poco::Int32 i = 0;i<nLen;i++)
	{
		if(buffer[i]=='\r' && buffer[i+1] == '\n') {
			// 如果前面没有缓冲;
			if(m_recvLen == 0) {
				memset(m_TempRecvBuf,0,BUF_LEN);
				memcpy(m_TempRecvBuf,pos,i-lastLen);
				m_TempRecvBuf[i-lastLen]='\0';
				ProcessJsonCommand(m_TempRecvBuf,i-lastLen);
				lastLen=(i+2);
				pos = buffer+lastLen;
				i++;
				//printf("find command 1,nLen: %u,%s\n",nLen,m_TempRecvBuf);
			}
			else
			{
				// 上一个信令还没有处理，有缓冲;
				memcpy(m_TempRecvBuf+m_recvLen,buffer,i);
				m_TempRecvBuf[m_recvLen+i]='\0';
				ProcessJsonCommand(m_TempRecvBuf,m_recvLen+i);
				m_TempRecvBuf[m_recvLen+i]=0;
				//printf("find command 2,nLen: %u,%s\n",nLen,m_TempRecvBuf);
				m_recvLen = 0;
				memset(m_TempRecvBuf,0,BUF_LEN);
				lastLen=(i+2);
				pos = buffer+lastLen;
				i++;
			}
		}
	}

	// 没有找到，直接复制到缓冲区中去;
	if(lastLen == 0)
	{
		//printf("not find command:\n %s \n",buffer);
		memcpy(m_TempRecvBuf+m_recvLen,buffer,nLen); 
		m_recvLen+=nLen;
		if(m_recvLen>BUF_LEN)
		{
			//printf("out of rang of buffer \n");
		}
	}
	else if(lastLen<nLen)
	{
		memcpy(m_TempRecvBuf+m_recvLen,pos,nLen-lastLen);
		m_recvLen = nLen-lastLen;
	}
}

//第一次握手
void MeetingConnImpl::OnHandSharke1(Object::Ptr object)
{
	Var varStatus = object->get("status");
	if(varStatus == 0)
	{
		Var varToken = object->get("token");
		string strToken = varToken;
		char msg2[1024];
		sprintf(msg2,"{\"cmd\":\"handSharke2\",\"token\":\"%s\"}\r\n",strToken.data());
		if(m_bRawSockConnected)
			m_pSock->sendBytes(msg2,strlen(msg2));
	}
	else
	{
		std::string strUpdateUrl = "http://onlycoder.net/meeting/setup.exe";
		if(object->has("updateUrl")){
			Var url = object->get("updateUrl");
			strUpdateUrl = url.toString();
		}
		if(m_pIMeetingEvent)
			m_pIMeetingEvent->On_MeetingEvent_UpdateSoft(strUpdateUrl);
	}
	
}
//第二次握手
void MeetingConnImpl::OnHandSharke2(Object::Ptr object)
{
	Var varSessionID = object->get("sessionID");
	m_mySessionID = varSessionID;
	m_bLogicServerConnected = true;
}

void MeetingConnImpl::OnLoginWithRoomNo(Object::Ptr object)
{
	Var varStatus = object->get("status");
	uint32_t statusCode = varStatus;
	switch(statusCode)
	{
	case 0:
		{
			m_bHasLogined = true;
			//strcpy(m_strUserName,userName.data());
			strcpy(m_strHeadImg,"default.jpg");
			//解析配置项
			Var varConfig = object->get("config");
			if(!varConfig.isEmpty())
			{
				std::string strConfig = varConfig;
				//解析内置命令，UserOnline,UserOffline,OnModifyUserFeeling
				Parser parser;
				Var result;
				try
				{
					result = parser.parse(strConfig);
				}
				catch(JSONException& jsone)
				{
					std::cout << jsone.message() << std::endl;
				}
				Object::Ptr subObj = result.extract<Object::Ptr>();
				Var varMediaHost = subObj->get("MediaHost");
				Var varRtpMediaPort = subObj->get("RtpMediaPort");
				Var varMixAudioPort = subObj->get("MixAudioPort");
				Var varBaseUrl = subObj->get("BaseUrl");
				if(!varMediaHost.isEmpty())
				{
					std::string strServer = varMediaHost;
					strcpy(MeetingFrameImpl::GetInstance()->m_strMediaHost,strServer.data());
				}

				if(!varBaseUrl.isEmpty())
				{
					std::string strBaseUrl = varBaseUrl;
					strcpy(MeetingFrameImpl::GetInstance()->m_strBaseUrl,strBaseUrl.data());
				}

				if(!varRtpMediaPort.isEmpty())
				{
					MeetingFrameImpl::GetInstance()->m_RtpMediaPort = (int)varRtpMediaPort;
				}
				if(!varMixAudioPort.isEmpty())
				{
					MeetingFrameImpl::GetInstance()->m_MixAudioPort = (int)varMixAudioPort;
				}
			}
			else
			{
				//没有取到配置项目
			}


			if(m_pIMeetingEvent)
			{
				MeetingFrameImpl::GetInstance()->On_MeetingEvent_Login_Result(0,"login success");
			}
			break;
		}
	default:
		{
			Var varMsg = object->get("msg");
			string strMsg = varMsg;
		}
	}
}
void MeetingConnImpl::OnLogin(Object::Ptr object)
{
	Var varStatus = object->get("status");
	uint32_t statusCode = varStatus;
	switch(statusCode)
	{
	case 0:
		{
			m_bHasLogined = true;
			Var varUserName = object->get("userName");
			
			string userName = varUserName;
			
			strcpy(m_strUserName,userName.data());
			//解析配置项
			Var varConfig = object->get("config");
			if(!varConfig.isEmpty())
			{
				std::string strConfig = varConfig;
				//解析内置命令，UserOnline,UserOffline,OnModifyUserFeeling
				Parser parser;
				Var result;
				try
				{
					result = parser.parse(strConfig);
				}
				catch(JSONException& jsone)
				{
					std::cout << jsone.message() << std::endl;
				}
				Object::Ptr subObj = result.extract<Object::Ptr>();
				Var varMediaHost = subObj->get("MediaHost");
				Var varRtpMediaPort = subObj->get("RtpMediaPort");
				Var varMixAudioPort = subObj->get("MixAudioPort");
				Var varBaseUrl = subObj->get("BaseUrl");
				if(!varMediaHost.isEmpty())
				{
					std::string strServer = varMediaHost;
					strcpy(MeetingFrameImpl::GetInstance()->m_strMediaHost,strServer.data());
				}

				if(!varBaseUrl.isEmpty())
				{
					std::string strBaseUrl = varBaseUrl;
					strcpy(MeetingFrameImpl::GetInstance()->m_strBaseUrl,strBaseUrl.data());
				}

				if(!varRtpMediaPort.isEmpty())
				{
					MeetingFrameImpl::GetInstance()->m_RtpMediaPort = (int)varRtpMediaPort;
				}
				if(!varMixAudioPort.isEmpty())
				{
					MeetingFrameImpl::GetInstance()->m_MixAudioPort = (int)varMixAudioPort;
				}
			}
			else
			{
				//没有取到配置项目
			}
			

			if(m_pIMeetingEvent)
			{
				MeetingFrameImpl::GetInstance()->On_MeetingEvent_Login_Result(0,"login success");
			}
			break;
		}
	default:
		{
			Var varMsg = object->get("msg");
			string strMsg = varMsg;
			if(m_pIMeetingEvent)
			{
				MeetingFrameImpl::GetInstance()->On_MeetingEvent_Login_Result(statusCode,(char*)strMsg.data());
			}
		}
	}
	
}

void MeetingConnImpl::OnLogout(Object::Ptr object)
{
	Var varStatus = object->get("status");
}

void MeetingConnImpl::OnGetRoomUserList(Object::Ptr object)
{
	Var varStatus = object->get("status");
	if(varStatus == 0)
	{
		Var resultObj = object->get("result");
		Poco::JSON::Array::Ptr array =resultObj.extract<Poco::JSON::Array::Ptr>();
		for(Poco::Int32 i = 0; i<array->size();i++)
		{
			Parser parser;
			Var result;
			Var item = array->get(i);
			string strItem = item;
			try
			{

				result = parser.parse(strItem);

			}
			catch(JSONException& jsone)
			{
				std::cout << jsone.message() << std::endl;
				return;
			}

			Object::Ptr tempObj = result.extract<Object::Ptr>();
			Var varSessionID = tempObj->get("sessionID");
			Var varUserName = tempObj->get("userName");
			Var varUserRole = tempObj->get("userRole");
			Var varClientType = tempObj->get("clientType");
			
			string strUserName = varUserName;
			
			uint32_t userRole = varUserRole;
			uint32_t clientType = varClientType;
			uint64_t sessionID= varSessionID;

			MeetingFrameImpl::GetInstance()->On_MeetingEvent_Member_Online(sessionID,(char*)strUserName.data(),"",clientType,userRole);
		}
	}
}

//得到房间列表
void MeetingConnImpl::OnGetRoomList(Object::Ptr object)
{
	FastMutex::ScopedLock autoLock(m_lock);
	Var varStatus = object->get("status");
	if(varStatus == 0)
	{
		try
		{
			Var resultObj = object->get("result");
			Poco::JSON::Array::Ptr array =resultObj.extract<Poco::JSON::Array::Ptr>();
			for(Poco::Int32 i = 0; i<array->size();i++)
			{
				Parser parser;
				Var result;
				Var item = array->get(i);
				string strItem = item;
				result = parser.parse(strItem);
				Object::Ptr tempObj = result.extract<Object::Ptr>();
				Var varRoomID = tempObj->get("roomID");
				Var varRoomName = tempObj->get("roomName");
				Var varRoomPassword = tempObj->get("roomPassword");
				Var varAdminPassword = tempObj->get("adminPassword");
				Var varMixAudio = tempObj->get("mixAudio");
				Var varAudioSamplerate = tempObj->get("audioSamplerate");
				Var varDefaultVideoSize = tempObj->get("defaultVideoSize");
				Var varIsPublic = tempObj->get("isPublic");
				Var varMaxUser = tempObj->get("maxUser");
				Var varOnlineUser = tempObj->get("onlineUser");
				Var varStartTime = tempObj->get("startTime");
				Var varEndTime = tempObj->get("endTime");
				Var varAudioType = tempObj->get("audioType");   //类型 会议，直播
				PROOM_INFO pRoomInfo = new ROOM_INFO();
				FastMutex::ScopedLock autoLock(m_lock);
				pRoomInfo->ulRoomID = varRoomID;
				strcpy(pRoomInfo->strRoomName,varRoomName.toString().data());
				strcpy(pRoomInfo->strPassword,varRoomPassword.toString().data());
				strcpy(pRoomInfo->strAdminPwd,varAdminPassword.toString().data());
				pRoomInfo->bMixAudio = varMixAudio;
				pRoomInfo->speakMode = varAudioType;
				pRoomInfo->sampleRate = varAudioSamplerate;
				pRoomInfo->defaultVideoSize = varDefaultVideoSize;
				pRoomInfo->maxUser = varMaxUser;
				pRoomInfo->startTime = varStartTime;
				pRoomInfo->endTime = varEndTime;
				if(varOnlineUser.isEmpty())
				{
					pRoomInfo->onlineUser = 0;
				}
				else
				{
					pRoomInfo->onlineUser = varOnlineUser;
				}
				//m_mapRoom.insert(UINT_VOID_MAP::ValueType(pRoomInfo->ulRoomID,pRoomInfo));
				m_pRoomList.push_back(vector<PROOM_INFO>::value_type(pRoomInfo));
			}
			if(array->size()>0)
				m_pIMeetingEvent->On_MeetingEvent_GetRoomList(array->size());
		}
		catch(Poco::Exception& exc)
		{
			std::cout <<"get room list error:"<< exc.message() << std::endl;
			return;
		}
	}
	else
	{
		Var varMsg = object->get("msg");
		string errMsg = varMsg;
		m_pIMeetingEvent->On_MeetingEvent_GetRoomList(0);
	}
}

 PROOM_INFO MeetingConnImpl::GetRoomInfo(int index)
{
	FastMutex::ScopedLock autoLock(m_lock);
	if(m_pRoomList.size()<index+1)
		return NULL;
	
	return  m_pRoomList[index];
	
}

//进入房间
void MeetingConnImpl::OnEnterRoom(Object::Ptr object)
{
	Var varStatus = object->get("status");
	uint32_t status = varStatus;
	if(status == 0)
	{
		Var varUserRole = object->get("userRole");
		m_userRole = varUserRole;
		GetRoomInfo(m_ulRoomID);
	}
	else
	{
		MeetingFrameImpl::GetInstance()->On_MeetingEvent_Enter_Room_Result(status,NULL);
	}
}
//退出房间
void MeetingConnImpl::OnLeaveRoom(Object::Ptr object)
{
	Var varStatus = object->get("status");
	if(varStatus == 0)
	{

	}
}
void MeetingConnImpl::OnTransparentCmd(Object::Ptr object)
{
	Var varFromUserID = object->get("fromSessionID");
	uint64_t fromSessionID = varFromUserID;
	Var varToUserID = object->get("toSessionID");
	uint64_t toSessionID = varToUserID;
	Var varFromUserName = object->get("fromUserName");
	std::string strFromUserName = varFromUserName;
	Var varSubCommand = object->get("subCmd");
	std::string strSubCommand = varSubCommand;
	//解析内置命令，UserOnline,UserOffline,OnModifyUserFeeling
	Parser parser;
	Var result;
	//try
	{
		result = parser.parse(strSubCommand);
	}
	/*catch(JSONException& jsone)
	{
	std::cout << jsone.message() << std::endl;
	}*/
	Object::Ptr subObj = result.extract<Object::Ptr>();
	Var varCommand = subObj->get("cmd");
	std::string strCommand = varCommand;
	printf("%s \n",strCommand.data());
	if(strCommand == "userOnline")
	{
		Var varSessionID = subObj->get("sessionID");
		Var varUserName = subObj->get("userName");
		Var varClientType = subObj->get("clientType");
		Var varUserRole = subObj->get("userRole");
		
		string strUserName = varUserName;
		
		uint32_t userRole = varUserRole;
		uint32_t clientType = varClientType;
		uint64_t sessionID = varSessionID;
		MeetingFrameImpl::GetInstance()->On_MeetingEvent_Member_Online(sessionID,(char*)strUserName.data(),"",clientType,userRole);
	}
	else if(strCommand == "userOffline")
	{
		Var varSessionID = subObj->get("sessionID");
		uint64_t sessionID = varSessionID;
		MeetingFrameImpl::GetInstance()->On_MeetingEvent_Member_Offline(sessionID);
	}
	else if(strCommand == "recvMyVideo")
	{
		
		Var varSSRC = subObj->get("ssrc");
		Var varFromSessionID = subObj->get("fromSessionID");
		uint32_t ulSSRC = varSSRC;
		uint64_t ulFromSessionID = varFromSessionID;
		Var varChannelID = subObj->get("channelID");
		uint32_t ulChannelID = varChannelID;
		MeetingFrameImpl::GetInstance()->OnRecvRemoteVideo(ulFromSessionID,ulChannelID,ulSSRC);
	}
	else if(strCommand == "requireKeyFrame")
	{
		Var varSSRC = subObj->get("ssrc");
		uint32_t ulSSRC = varSSRC;
		MeetingFrameImpl::GetInstance()->On_MeetingEvent_RequireKeyFrame(ulSSRC);
	}
	else if(strCommand == "closeMyVideo")
	{
		Var varFromSessionID = subObj->get("fromSessionID");
		Var varChannelID = subObj->get("channelID");
		uint64_t ulFromSessionID = varFromSessionID;
		uint32_t ulChannelID = varChannelID;
		MeetingFrameImpl::GetInstance()->OnCloseRemoteVideo(ulFromSessionID,ulChannelID);
	}
	else if(strCommand == "recvMyAudio")
	{
		Var varSSRC = subObj->get("ssrc");
		Var varFromSessionID = subObj->get("fromSessionID");
		uint32_t ulSSRC = varSSRC;
		uint64_t ulFromSessionID = varFromSessionID;
		Var varChannelID = subObj->get("channelID");
		uint32_t ulChannelID = varChannelID;
		MeetingFrameImpl::GetInstance()->OnRecvRemoteAudio(ulFromSessionID,ulChannelID,ulSSRC);
	}
	else if(strCommand == "closeMyAudio")
	{
		Var varFromSessionID = subObj->get("fromSessionID");
		Var varChannelID = subObj->get("channelID");
		uint64_t ulFromSessionID = varFromSessionID;
		uint32_t ulChannelID = varChannelID;
		MeetingFrameImpl::GetInstance()->OnCloseRemoteAudio(ulFromSessionID,ulChannelID);
	}
	else if(strCommand == "setSpeaker")
	{
		Var varBSet = subObj->get("bSet");
		uint32_t ulSet = varBSet;
		Var varFromUserID = subObj->get("sessionID");
		uint64_t ulFromUserID = varFromUserID;
		if(ulFromUserID==m_mySessionID)
			MeetingFrameImpl::GetInstance()->OnSetAsSpeaker(ulFromUserID,ulSet);
	}
	else if(strCommand == "setSpeakMode")
	{
		Var varMode = subObj->get("mode");
		int mode = varMode;
		MeetingFrameImpl::GetInstance()->On_MeetingEvent_SpeakMode_Change(mode);
	}
	else if(strCommand == "setRoomMode")
	{
		Var varMode = subObj->get("mode");
		int mode = varMode;
		MeetingFrameImpl::GetInstance()->On_MeetingEvent_RoomMode_Change(mode);
	}
	else if(strCommand == "setShower")
	{
		Var varBSet = subObj->get("bSet");
		uint32_t ulSet = varBSet;
		Var varFromUserID = subObj->get("sessionID");
		uint64_t ulFromUserID = varFromUserID;
		MeetingFrameImpl::GetInstance()->OnSetAsShower(ulFromUserID,ulSet);
	}
	else if(strCommand == "setAdmin")
	{
		Var varBSet = subObj->get("bSet");
		uint32_t ulSet = varBSet;
		Var varFromUserID = subObj->get("sessionID");
		uint64_t ulFromUserID = varFromUserID;
		MeetingFrameImpl::GetInstance()->OnSetAsAdmin(ulFromUserID,ulSet);
	}
	else if(strCommand == "setBigVideo")
	{
		Var varFromUserID = subObj->get("sessionID");
		uint64_t ulFromUserID = varFromUserID;
		MeetingFrameImpl::GetInstance()->OnSetAsBigVideo(ulFromUserID);
	}
	else if(strCommand == "applySpeaker")
	{
		Var varFromUserID = subObj->get("fromSessionID");
		uint64_t ulFromUserID = varFromUserID;

		MeetingFrameImpl::GetInstance()->OnApplySpeaker(ulFromUserID);
	}
	else if(strCommand == "cancelApplySpeaker")
	{
		Var varFromUserID = subObj->get("fromSessionID");
		uint64_t ulFromUserID = varFromUserID;
		MeetingFrameImpl::GetInstance()->OnCancelApplySpeaker(ulFromUserID);
	}
	else if(strCommand == "openDoc")
	{
		Var varUrl = subObj->get("url");
		std::string url = varUrl;
		Var varIndex = subObj->get("index");
		Var varTotal = subObj->get("total");
		int index = varIndex;
		int total = varTotal;
		if(m_pIMeetingEvent)
		{
			m_pIMeetingEvent->On_MeetingEvent_OpenDoc(url,index,total);
		}
	}
	else if(strCommand == "closeDoc")
	{
		if(m_pIMeetingEvent)
		{
			m_pIMeetingEvent->On_MeetingEvent_CloseDoc();
		}
	}
	else if(strCommand == "stopShareCursor")
	{
		if(m_pIMeetingEvent)
		{
			m_pIMeetingEvent->On_MeetingEvent_StopShareCursor();
		}
	}
	else if(strCommand == "shareCursor")
	{
		Var varX = subObj->get("x");
		Var varY = subObj->get("y");
		float x = varX;
		float y = varY;
		if(m_pIMeetingEvent)
		{
			m_pIMeetingEvent->On_MeetingEvent_ShareCursor(x,y);
		}
	}else if(strCommand == "startVideoMonitor"){

		MeetingFrameImpl::GetInstance()->OnStartVideoMonitor();

	}else if(strCommand == "stopVideoMonitor"){
		MeetingFrameImpl::GetInstance()->OnStopVideoMonitor();

	}else if(strCommand == "pauseVideo"){
		MeetingFrameImpl::GetInstance()->OnPauseVideo();

	}else if(strCommand == "resumeVideo"){
		MeetingFrameImpl::GetInstance()->OnResumeVideo();

	}else if(strCommand == "sendVideoSSRCToAdmin"){
		Var varSSRC = subObj->get("ssrc");
		uint32_t ssrc = varSSRC;
		MeetingFrameImpl::GetInstance()->OnRecvVideoSSRC(fromSessionID,ssrc);
	}
	
}

void MeetingConnImpl::OnGetRoomInfo(Object::Ptr object)
{
	FastMutex::ScopedLock autoLock(m_lock);
	Var varStatus = object->get("status");
	int docIndex = 0;
	int docTotal = 0;
	int shareDoc = 0;
	std::string docName = "";
	if(object->isNull("shareDoc")==false)
	{
		shareDoc = object->get("shareDoc");
		if(shareDoc == 1)
		{
			docIndex = object->get("docIndex");
			docTotal = object->get("docTotal");
			docName =  object->get("docName").toString();
		}
	}

	if(varStatus == 0)
	{
		Var resultObj;
		try
		{
			resultObj = object->get("result");
		}
		catch(JSONException& jsone)
		{
			std::cout << jsone.message() << std::endl;
			return;
		}
		Object::Ptr subObj = resultObj.extract<Object::Ptr>();
		Var varRoomName = subObj->get("roomName");
		Var varRoomID = subObj->get("roomID");
		Var varAdminPassword= subObj->get("adminPassword");
		Var varRoomPassword = subObj->get("roomPassword");
		Var varAudioType = subObj->get("audioType");
		Var varIsPublic = subObj->get("isPublic");
		Var varMixAudio = subObj->get("mixAudio");
		Var varDefaultVideoSize = subObj->get("defaultVideoSize");
		Var varAudioSamplate = subObj->get("audioSamplerate");
		Var varStartTime = subObj->get("startTime");
		Var varEndTime = subObj->get("endTime");
		Var varOnlineUser = subObj->get("onlineUser");
		string strRoomName = varRoomName;
		uint32_t roomID = varRoomID;
		string strPassword = varRoomPassword;
		string strAdminPassword = varAdminPassword;
		uint32_t defaultVideoSize = varDefaultVideoSize;
		uint32_t audioType = varAudioType;
		uint32_t isPublic = varIsPublic;
		uint32_t mixAudio = varMixAudio;
		uint32_t audioSamplate = 0;
		time_t startTime = varStartTime;
		time_t endTime = varEndTime;
		
		ROOM_INFO rs;
		rs.bMixAudio =mixAudio;
		rs.sampleRate = audioSamplate;
		rs.speakMode = audioType;
		rs.defaultVideoSize = defaultVideoSize;
		rs.ulRoomID = roomID;
		rs.ulIsPublic = isPublic;
		rs.roomMode = 1;
		rs.sampleRate = varAudioSamplate;
		rs.startTime = startTime;
		rs.endTime = endTime;
		if(varOnlineUser.isEmpty())
		{
			rs.onlineUser = 0;
		}
		else
		{
			rs.onlineUser = varOnlineUser;
		}
		if(subObj->has("roomType"))
		{
			Var varRoomType = subObj->get("roomType");
			unsigned int ulRoomType = varRoomType;
			rs.roomMode = ulRoomType;
		}

		if(subObj->has("bigVideoUser"))
		{
			Var varbigVideoUser = subObj->get("bigVideoUser");
			uint64_t ulBigVideoUser = varbigVideoUser;
			rs.bigVideoUser = ulBigVideoUser;
		}
		strcpy(rs.strAdminPwd,strAdminPassword.data());
		strcpy(rs.strRoomName,strRoomName.data());
		MeetingFrameImpl::GetInstance()->On_MeetingEvent_Enter_Room_Result(0,(char*)&rs);
		if(shareDoc==1)
		{
			if(m_pIMeetingEvent)
			{
				m_pIMeetingEvent->On_MeetingEvent_OpenDoc(docName,docIndex,docTotal);
			}
		}
		GetRoomUserList(m_ulRoomID);
	}
	else
	{
		Var varMsg = object->get("msg");
		string errMsg = varMsg;
		MeetingFrameImpl::GetInstance()->On_MeetingEvent_Enter_Room_Result(1,NULL);
	}
}



void MeetingConnImpl::OnTextChat(Object::Ptr object)
{
	Var msg = object->get("msg");
	std::string strMsg = msg;
	Var varFamilly = object->get("fontName");
	std::string strFamilly = varFamilly;
	Var varFontSize = object->get("fontSize");
	uint32_t ulFontSize = varFontSize;
	Var varColor = object->get("color");
	uint32_t ulColor = varColor;
	Var varFlag = object->get("flag");
	uint64_t ulFlag = varFlag;
	Var varFromUserID = object->get("fromSessionID");
	uint64_t ulFromUserID = varFromUserID;
	Var varFromUserName = object->get("fromUserName");
	string strFromUserName = varFromUserName;
	Var varDate = object->get("date");
	time_t date = varDate;
	std::string msg2 = HTTPCookie::unescape(strMsg);
	std::string strFont2 = HTTPCookie::unescape(strFamilly);
	PMsgItem pMsgItem = new MsgItem();
	pMsgItem->ulUserID = ulFromUserID;
	pMsgItem->font_size = ulFontSize;
	pMsgItem->color = ulColor;
	pMsgItem->fontFlat = ulFlag;
	strcpy(pMsgItem->userName,strFromUserName.data());
	strcpy(pMsgItem->msg,msg2.data());
	strcpy(pMsgItem->familly,strFont2.data());
	//插入到列表
	m_chatMsgList.push_back(vector<PMsgItem>::value_type(pMsgItem));
	if(m_pIMeetingEvent)
	{
		m_pIMeetingEvent->On_MeetingEvent_Text_Msg(ulFromUserID,(char *)(msg2.data()),ulFontSize,ulFlag,ulColor,(char*)(strFont2.data()));
	}
}

void MeetingConnImpl::OnGetChatHistory(Object::Ptr object)
{
	Var varStatus = object->get("status");
	if(varStatus == 0)
	{
		Var resultObj = object->get("result");
	}
	else
	{
		Var varMsg = object->get("msg");
		string errMsg = varMsg;
	}
}

void MeetingConnImpl::ProcessJsonCommand(char * buffer,Poco::Int32 nLen)
{
	Parser parser;
	Var result;
	//printf("\n %s len: %u\n",buffer,nLen);
	TCHAR strLog[1024];
	AA_UTF82W(strLog,buffer,1024);
	LogTrace(L"%s\n",strLog);
	try
	{
		result = parser.parse(buffer);
	}
	catch(JSONException& jsone)
	{
		//printf("%s \n",buffer);
		std::cout << jsone.message() << std::endl;
		return;
	}
	catch(Poco::Exception& exc)
	{
		//printf("json string parse error, len: %u",nLen);
		//printf("%s \n",buffer);
		std::cout << exc.message() << std::endl;
		m_pIMeetingEvent->OnNetEvent(NET_EVENT_JSON_ERROR,"json parse error");
		return;
	}
	Object::Ptr object = result.extract<Object::Ptr>();
	Var varCommand = object->get("cmd");
	std::string strCommand = varCommand;
	if(strCommand == "handSharke1_ret")
	{
		OnHandSharke1(object);
	}
	else if(strCommand == "handSharke2_ret")
	{
		OnHandSharke2(object);
	}
	else if(strCommand == "login_ret")
	{
		OnLogin(object);
	}
	else if(strCommand == "logout_ret")
	{
		OnLogout(object);
	}
	else if(strCommand == "enterRoom_ret")
	{
		OnEnterRoom(object);
	}
	else if(strCommand == "leaveRoom_ret")
	{
		OnLeaveRoom(object);
	}
	else if(strCommand == "getRoomUserList_ret")
	{
		OnGetRoomUserList(object);
	}
	else if(strCommand == "getRoomList_ret")
	{
		OnGetRoomList(object);
	}
	else if(strCommand == "getRoomInfo_ret")
	{
		OnGetRoomInfo(object);
	}
	else if(strCommand == "getChatHistory_ret")
	{
		OnGetChatHistory(object);
	}
	else if(strCommand == "textChat")
	{
		OnTextChat(object);
	}
	else if(strCommand == "transparentCmd")
	{
		OnTransparentCmd(object);
	}
	else if(strCommand == "loginWithRoomNo_ret")
	{
		OnLoginWithRoomNo(object);
	}

}
std::string MeetingConnImpl::IMUnescape(const std::string& str)
{
	std::string res = "";
	try {
		res = Poco::Net::HTTPCookie::unescape(str);
	}
	catch (...) 
	{
		res = "";
		printf("unescape err:%s\n", str.c_str());
	}

	return res; 
}

//得到聊天消息
PMsgItem MeetingConnImpl::GetChatMsgItem(int index)
{
	if(index>m_chatMsgList.size()-1)
		return NULL;
	return m_chatMsgList[index];
}