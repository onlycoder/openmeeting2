#include "StdAfx.h"
#include "MeetingFrameImpl.h"
#include "string_conv.h"
#include "MeetingConnImpl.h"
#include "Poco/Net/HTTPCookie.h"
using Poco::Net::HTTPCookie;

static void  GetFileNameFromExePath(LPCTSTR lpctstrFileName, LPTSTR lptstrNewFileName)
{
	TCHAR	strExeFilePath[4096] = _T("");
	TCHAR*	p = NULL;
	::GetModuleFileNameW(NULL, strExeFilePath, 4096);
	p = wcsrchr(strExeFilePath, _T('\\'));
	if (p)
	{
		*p = 0;
	}
	wsprintf(lptstrNewFileName, _T("%s\\%s"), strExeFilePath, lpctstrFileName);
}
bool CreateMeetingFrame(IMeetingFrame** pMeetingFrame,IMeetingFrameEvent* pEvent,
	HWND mainHWND,IMediaStreamEvent* pStreamEvent,std::string strServer)
{
	if(strServer.length()<4|| strServer.length()<12)
		return false;
	MeetingFrameImpl * pMeetingFrameImpl = MeetingFrameImpl::GetInstance();
	MeetingFrameImpl::GetInstance()->SetIMeetingFrameEvent(pEvent);
	if(pMeetingFrameImpl)
	{
		*pMeetingFrame = pMeetingFrameImpl;
		return pMeetingFrameImpl->Init(mainHWND,pStreamEvent,strServer);
	}
	return false;
}
MeetingFrameImpl* MeetingFrameImpl::m_instance = NULL;

MeetingFrameImpl* MeetingFrameImpl::GetInstance()
{
	if(m_instance == NULL)
	{
		m_instance = new MeetingFrameImpl();
	}
	return m_instance;
}

MeetingFrameImpl::MeetingFrameImpl()
{
	m_mainHWND = NULL;
	m_pRoomInfo = NULL;
	m_videoIndex = 0;
	memset(&m_myUserInfo,0,sizeof(m_myUserInfo));
	m_bStartAudio = false;
	m_bStartVideo = false;
	m_pIZYMediaStreamManager = NULL;
	m_bShareDoc = false;
	m_bBroadcast = false;
	m_bVideoMonitor = false;
	m_bHasStartVideo2 = false;
	m_Version = 16;
}

void MeetingFrameImpl::SetIMeetingFrameEvent(IMeetingFrameEvent* pEvent)
{
	m_pEvent = pEvent;
	
}

MeetingFrameImpl::~MeetingFrameImpl(void)
{
	MeetingConnImpl::GetInstance()->Release();
}

bool MeetingFrameImpl::EnterRoom(uint32_t uiRoomID,char *strRoomPwd,char*strToken)
{
	MeetingConnImpl::GetInstance()->EnterRoom(uiRoomID);
	return true;
}

void MeetingFrameImpl::LeaveRoom()
{
	if(m_bShareDoc == true)
		CloseDoc();
	 MeetingConnImpl::GetInstance()->LeaveRoom();
	 if(m_pIZYMediaStreamManager && kAudioMode ==1)
		m_pIZYMediaStreamManager->StopAudioRecver(m_ulMixAudioSSRC,MeetingConnImpl::GetInstance()->GetRoomID(),kAudioMode);
	 
	 //停止接收音视频
	 Mutex::ScopedLock lock(m_Mutex);
	 UINT_VOID_MAP::Iterator item = m_mapOnlineUser.begin();
	 while(item!=m_mapOnlineUser.end())
	 {
		 PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
		 uint32_t ulSSRC = pInfo->ulVideoSSRC;
		 if(ulSSRC > 0)
		 {
			 m_pIZYMediaStreamManager->StopVideoRecver(ulSSRC,MeetingConnImpl::GetInstance()->GetRoomID());
			
		 }
		 if(m_pIZYMediaStreamManager && kAudioMode!=1 && pInfo->ulAudioSSRC>0)
			  m_pIZYMediaStreamManager->StopAudioRecver(pInfo->ulAudioSSRC,MeetingConnImpl::GetInstance()->GetRoomID(),kAudioMode);
		 item++;
	 }

	 //清空在线用户
	 item = m_mapOnlineUser.begin();
	 while(item!=m_mapOnlineUser.end())
	 {
		 delete item->second;
		 item++;
	 }
	 m_mapOnlineUser.clear();
}

void MeetingFrameImpl::LoginWithRoomNo(const char* strUserName, const char* strRoomPassword,uint32_t clientType,uint32_t u32RoomID)
{
	MeetingConnImpl::GetInstance()->LoginWithRoomNo(strUserName,strRoomPassword,clientType,u32RoomID);
}

 bool MeetingFrameImpl::Login(char* strUserName,char* strPassword,uint32_t ulRoomID,bool bEncrypt)
 {
	 MeetingConnImpl::GetInstance()->Login(strUserName,strPassword,ulRoomID,bEncrypt);
	 return true;
 }

 bool MeetingFrameImpl::Logout()
 {
	 //如果用户已打开视频，音频，通知其它用户关闭

	 //如果自己已打开视频，通知对方打开我的视频;
	 Mutex::ScopedLock lock(m_Mutex);
	 if(m_myUserInfo.ulVideoSSRC > 0)
	 {
		 StopPublishVideo(0);
		 m_myUserInfo.ulVideoSSRC = 0;
	 }

	 if(m_myUserInfo.ulAudioSSRC > 0)
	 {
		 StopPublishAudio(0);
		 m_myUserInfo.ulAudioSSRC = 0;
	 }
	  //通知同房间用户我已经下线
	  MeetingConnImpl::GetInstance()->Logout();
	  return true;
 }
 bool MeetingFrameImpl::ApplySpeaker()   //申请发言
 {
	 char subMsg[512] = {0};
	 sprintf(subMsg,"{\"cmd\":\"applySpeaker\",\"fromSessionID\":%I64u}",m_myUserInfo.sessionID);
	 MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
	 return true;
 }

bool MeetingFrameImpl::CancelApplySpeaker()
{
	
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"cancelApplySpeaker\",\"fromSessionID\":%I64u}",m_myUserInfo.sessionID);
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
	return true;
}
 
void MeetingFrameImpl::Release()
{
	MeetingConnImpl::GetInstance()->Release();
	SetMicMute(false);
	if(m_pIZYMediaStreamManager)
		m_pIZYMediaStreamManager->DeInit();
}

bool MeetingFrameImpl::Init(HWND mainHWND,IMediaStreamEvent* pStreamEvent,std::string strServer)
{
	bool ret = false;
	m_mainHWND = mainHWND;
	m_pStreamEvent = pStreamEvent;
	//配置文件
	
	m_MixAudioPort = 7083;
	m_RtpMediaPort = 7084;
	int pos = 0;
	if((pos = strServer.find(":"))<=0)
		return false;
	strcpy(m_strMediaHost,strServer.substr(0,pos).data());
	MeetingConnImpl::GetInstance()->Connect((char*)(strServer.data()),m_pEvent);
	
	return ret;
}



int MeetingFrameImpl::OnRecvRemoteVideo(uint64_t ulRemoteUserID ,unsigned long ulChannelID, uint32_t ulSSRC)
{
	if(m_pIZYMediaStreamManager)
	{
		//更新这个用户的属性，保存视频编号
		Mutex::ScopedLock lock(m_Mutex);
		UINT_VOID_MAP::Iterator item = m_mapOnlineUser.find(ulRemoteUserID);
		if(item != m_mapOnlineUser.end())
		{
			PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
			pInfo->ulVideoSSRC = ulSSRC;
		}
		//::PostMessage(m_mainHWND,WM_RECV_REMOTE_VIDEO,ulChannelID,ulRemoteUserID);
		if(m_pEvent)
			m_pEvent->On_MeetingEvent_RecvMyVideo(ulRemoteUserID,ulChannelID);
	}
	return 0;
}
void MeetingFrameImpl::OnCloseRemoteVideo(uint64_t ulRemoteUserID,uint32_t ulChannelID)
{
	Mutex::ScopedLock lock(m_Mutex);
	UINT_VOID_MAP::Iterator item = m_mapOnlineUser.find(ulRemoteUserID);
	if(item!=m_mapOnlineUser.end())
	{
		PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
		uint32_t ulSSRC =pInfo->ulVideoSSRC;
		m_pEvent->On_MeetingEvent_CloseMyVideo(ulRemoteUserID,ulSSRC);
		pInfo->ulVideoSSRC = 0;
	}
}
void MeetingFrameImpl::OnCloseRemoteAudio(uint64_t ulRemoteUserID,uint32_t ulChannelID)
{
	Mutex::ScopedLock lock(m_Mutex);
	UINT_VOID_MAP::Iterator item = m_mapOnlineUser.find(ulRemoteUserID);
	if(item!=m_mapOnlineUser.end())
	{
		PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
		uint32_t ulSSRC =pInfo->ulAudioSSRC;
		m_pIZYMediaStreamManager->StopAudioRecver(ulSSRC,MeetingConnImpl::GetInstance()->GetRoomID(),kAudioMode);
		pInfo->ulAudioSSRC = 0;
		if(m_pEvent)
			m_pEvent->On_MeetingEvent_UpdateUI();
	}
}
int MeetingFrameImpl::OnRecvRemoteAudio(uint64_t ulRemoteUserID , unsigned long  ulChannelID, uint32_t ulSSRC)
{
	
	if(m_pIZYMediaStreamManager)
	{
		Mutex::ScopedLock lock(m_Mutex);
		//更新这个用户属性
		UINT_VOID_MAP::Iterator item = m_mapOnlineUser.find(ulRemoteUserID);
		if(item != m_mapOnlineUser.end())
		{
			m_ulAudioPlaySSRC = ulSSRC;
			m_pIZYMediaStreamManager->StartAudioRecver(ulSSRC,m_mainHWND,MeetingConnImpl::GetInstance()->GetRoomID(),
				kAudioMode,m_pRoomInfo->sampleRate);
			PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
			pInfo->ulAudioSSRC = ulSSRC;
		}
		if(m_pEvent)
			m_pEvent->On_MeetingEvent_UpdateUI();
	}
	return 0;
}



void MeetingFrameImpl::UpdateVideoLayout()
{
	
}

void MeetingFrameImpl::SendTextMsg(uint64_t ulUserID,char* msg,int font_size,unsigned long fontFlag,unsigned long color,char *familly)
{
	MeetingConnImpl::GetInstance()->SendTextMsg(ulUserID,msg,familly,font_size,color,fontFlag);
}



void MeetingFrameImpl::SetAsSpeaker(uint64_t ulUserID,bool bSet)  //设置为发言人,bSet为真是设置，bSet为flash时取消
{
	if(0 == ulUserID||ulUserID == m_myUserInfo.sessionID)
	{
		
	}
	else
	{
		PClient_Online_User_Info pOnlineUserInfo = FindOnlineUser(ulUserID);
		if(!pOnlineUserInfo)
			return;
		char m_TempBuf[256] = {0};
		memcpy(m_TempBuf,&ulUserID,4);
		memcpy(m_TempBuf+4,&bSet,1);
		char subMsg[512] = {0};
		sprintf(subMsg,"{\"cmd\":\"setSpeaker\",\"sessionID\":%I64u,\"bSet\":%u}",ulUserID,bSet);
		MeetingConnImpl::GetInstance()->TransParentRoomCommand(ulUserID,subMsg);
	}
	
}

void MeetingFrameImpl::SetRoomMode(int mode)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(m_pRoomInfo)
	{
		m_pRoomInfo->roomMode = mode;
	}
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"setRoomMode\",\"mode\":%u}",mode);
	MeetingConnImpl::GetInstance()->SetRoomMode(subMsg);
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
}

void MeetingFrameImpl::SetSpeakMode(int mode)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(GetUserRole(0)!=1||mode == m_pRoomInfo->speakMode)
		return;
	if(m_pRoomInfo)
		m_pRoomInfo->speakMode = mode;
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"setSpeakMode\",\"mode\":%u}",mode);
	MeetingConnImpl::GetInstance()->SetSpeakMode(subMsg);
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
}

void MeetingFrameImpl::OpenDoc(std::string strUrl,int index,int total)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(strUrl.length() == 0)
		return;
	m_bShareDoc = true;
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"openDoc\",\"url\":\"%s\",\"index\":%d,\"total\":%d}",strUrl.data(),index,total);
	MeetingConnImpl::GetInstance()->SendCmd(subMsg);
	//MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
}

void MeetingFrameImpl::CloseDoc()
{
	Mutex::ScopedLock lock(m_Mutex);
	m_bShareDoc = false;
	char *subMsg = "{\"cmd\":\"closeDoc\"}";
	MeetingConnImpl::GetInstance()->SendCmd(subMsg);
	//MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
}

//共享光标
void MeetingFrameImpl::ShareCursor(float x,float y)
{
	Mutex::ScopedLock lock(m_Mutex);
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"shareCursor\",\"x\":%f,\"y\":%f}",x,y);
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
}
//停止共享光标
void MeetingFrameImpl::StopShareCursor()
{
	Mutex::ScopedLock lock(m_Mutex);
	char *subMsg = "{\"cmd\":\"stopShareCursor\"}";
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
}

void MeetingFrameImpl::SetAsBigVideo(uint64_t ulUserID)
{
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"setBigVideo\",\"sessionID\":%I64u}",ulUserID);
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
}

void MeetingFrameImpl::SetAsAdmin(uint64_t ulUserID,bool bSet)  //设置为主讲
{
	if(0 == ulUserID||ulUserID == m_myUserInfo.sessionID)
	{
		ulUserID = m_myUserInfo.sessionID;
		if(m_orgi_user_role>1)
		{
			if(bSet)
				m_myUserInfo.userRole = 1;
			else
				m_myUserInfo.userRole = 2;
		}
	}
	else
	{
		PClient_Online_User_Info pOnlineUserInfo = FindOnlineUser(ulUserID);
		if(!pOnlineUserInfo)
			return;
		if(m_orgi_user_role>1)
		{
			if(bSet)
				pOnlineUserInfo->userRole = 1;
			else
				pOnlineUserInfo->userRole = 2;
		}

	}
	char m_TempBuf[256] = {0};
	memcpy(m_TempBuf,&ulUserID,4);
	memcpy(m_TempBuf+4,&bSet,1);
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"setAdmin\",\"sessionID\":%I64u,\"bSet\":%u}",ulUserID,bSet);
	MeetingConnImpl::GetInstance()->SetAsAdmin(subMsg);
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
	if(m_pEvent)
	{
		m_pEvent->On_MeetingEvent_UpdateUI();
	}
}


void MeetingFrameImpl::StopAllMediaStream()
{
	StopPublishAudio(0);
	StopPublishVideo(0);
	Mutex::ScopedLock lock(m_VideoListMutex);
	while(m_RecvVideoList.size()>0)
	{
		uint64_t userID = m_RecvVideoList.front();
		StopRecvRemoteVideo(userID);
		m_RecvVideoList.pop();
	}

	Mutex::ScopedLock lock2(m_AudioListMutex);
	while(m_RecvAudioList.size()>0)
	{
		uint64_t userID = m_RecvAudioList.front();
		StopRecvRemoteAudio(userID);
		m_RecvAudioList.pop();
	}
}


void MeetingFrameImpl::StopRecvRemoteVideo(uint64_t ulUserID)
{
	Mutex::ScopedLock lock(m_Mutex);
	UINT_VOID_MAP::Iterator item = m_mapOnlineUser.find(ulUserID);
	if(item!=m_mapOnlineUser.end())
	{
		PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
		uint32_t ulSSRC =pInfo->ulVideoSSRC;
		m_pIZYMediaStreamManager->StopVideoRecver(ulSSRC,MeetingConnImpl::GetInstance()->GetRoomID());
	}
}

void MeetingFrameImpl::StopRecvRemoteVideo2(uint32_t ssrc)
{
	Mutex::ScopedLock lock(m_Mutex);
	m_pIZYMediaStreamManager->StopVideoRecver(ssrc,MeetingConnImpl::GetInstance()->GetRoomID());
}

void MeetingFrameImpl::StartRecvRemoteVideo2(uint64_t sessionId,uint32_t ssrc,IVideoWin*videoWin)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(m_pIZYMediaStreamManager)
	{
			if(videoWin)
			{
				videoWin->SetUserID(sessionId);
				videoWin->SetUserName(GetUserName(sessionId));
				m_pIZYMediaStreamManager->StartVideoRecver(ssrc, videoWin, MeetingConnImpl::GetInstance()->GetRoomID());
				//通知对方发送关键帧给我
				
				char subMsg[512] = {0};
				sprintf(subMsg,"{\"cmd\":\"requireKeyFrame\",\"ssrc\":%u}",ssrc);
				MeetingConnImpl::GetInstance()->TransParentRoomCommand(sessionId,subMsg);
				
			}
			else
			{
				OutputDebugStringW(L"Get New IVideoWin Failed!\n");
			}
	}
	
}

void MeetingFrameImpl::StartRecvRemoteVideo(uint64_t ulUserID,uint32_t ulChannelID,IVideoWin*NewIVideoWin)
{
	Mutex::ScopedLock lock(m_Mutex);
	UINT_VOID_MAP::Iterator item = m_mapOnlineUser.find(ulUserID);
	if(item!=m_mapOnlineUser.end())
	{
		PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
		uint32_t ulSSRC =pInfo->ulVideoSSRC;
		Mutex::ScopedLock lock(m_VideoListMutex);
		m_RecvVideoList.push(ulUserID);
		if(m_pIZYMediaStreamManager)
		{
			if(NewIVideoWin)
			{
				NewIVideoWin->SetUserID(ulUserID);
				NewIVideoWin->SetUserName(GetUserName(ulUserID));
				m_pIZYMediaStreamManager->StartVideoRecver(ulSSRC, NewIVideoWin, MeetingConnImpl::GetInstance()->GetRoomID());
				//通知对方发送关键帧给我
				char subMsg[512] = {0};
				sprintf(subMsg,"{\"cmd\":\"requireKeyFrame\",\"ssrc\":%u}",ulSSRC);
				MeetingConnImpl::GetInstance()->TransParentRoomCommand(ulUserID,subMsg);
			}
			else
			{
				OutputDebugStringW(L"Get New IVideoWin Failed!\n");
			}
		}
	}
}

void MeetingFrameImpl::On_MeetingEvent_RequireKeyFrame(uint32_t ssrc)
{
	Mutex::ScopedLock lock(m_Mutex);
	RequireKeyFrame(ssrc);
}

void MeetingFrameImpl::StartRecvRemoteAudio(uint64_t ulUserID)
{
	Mutex::ScopedLock lock(m_Mutex);
	UINT_VOID_MAP::Iterator item = m_mapOnlineUser.find(ulUserID);
	if(item!=m_mapOnlineUser.end())
	{
		Mutex::ScopedLock lock(m_AudioListMutex);
		m_RecvAudioList.push(ulUserID);
		PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
		uint32_t ulSSRC =pInfo->ulAudioSSRC;
		m_ulAudioPlaySSRC = ulSSRC;
		if(m_pEvent)
			m_pEvent->On_MeetingEvent_UpdateUI();
		if(m_pIZYMediaStreamManager)
			m_pIZYMediaStreamManager->StartAudioRecver(ulSSRC,m_mainHWND,MeetingConnImpl::GetInstance()->GetRoomID(),
			kAudioMode,m_pRoomInfo->sampleRate);
	}
}
void MeetingFrameImpl::StopRecvRemoteAudio(uint64_t ulUserID)
{
	Mutex::ScopedLock lock(m_Mutex);
	UINT_VOID_MAP::Iterator item = m_mapOnlineUser.find(ulUserID);
	if(item!=m_mapOnlineUser.end())
	{
		PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
		uint32_t ulSSRC =pInfo->ulAudioSSRC;
		pInfo->ulAudioSSRC = 0;
		if(m_pEvent)
			m_pEvent->On_MeetingEvent_UpdateUI();
		m_pIZYMediaStreamManager->StopAudioRecver(ulSSRC,MeetingConnImpl::GetInstance()->GetRoomID(),kAudioMode);
	}
}

void MeetingFrameImpl::StartPublishVideo(uint64_t ulToUserID,uint32_t ulChannelID,IVideoWin*videoWin)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(ulChannelID <0)
		return;
	int count = GetVideoCaptureDeviceCount();
	if(count == 0)
		return;

	if(ulChannelID>GetVideoCaptureDeviceCount()-1)
		ulChannelID = 0;
	if(m_bStartVideo == true)
		return;
	m_bStartVideo = true;
	uint64_t myGuid ;
	Poco::Timestamp t;
	uint32_t ulSSRC = unsigned int(t.epochMicroseconds()&0xFFFFFFFF);
	if(videoWin)
	{
		videoWin->SetUserID(0);
		videoWin->SetUserName(m_myUserInfo.strUserName);
		m_myUserInfo.ulVideoSSRC = m_pIZYMediaStreamManager->StartVideoSender(ulSSRC,ulChannelID,
			videoWin, MeetingConnImpl::GetInstance()->GetRoomID());
		m_bBroadcast = true;
		if(m_myUserInfo.ulVideoSSRC > 0)
		{
			char pData[64];
			myGuid = m_pIZYMediaStreamManager->GetGUID();
			memcpy(pData,&myGuid,8);
			memcpy(pData+8,&m_myUserInfo.ulVideoSSRC,4);
			char subMsg[512] = {0};
			sprintf(subMsg,"{\"cmd\":\"recvMyVideo\",\"ssrc\":%u,\"fromSessionID\":%I64u,\"channelID\":%u}"
				,m_myUserInfo.ulVideoSSRC,m_myUserInfo.sessionID,ulChannelID);
			MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
		}
		
	}
	else
	{
		OutputDebugStringW(L"Get New IVideoWin Failed!\n");
	}
}

uint32_t MeetingFrameImpl::StartPublishVideo2(uint64_t ulToUserID,uint32_t ulChannelID,IVideoWin*videoWin)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(ulChannelID <0)
		return 0;
	int count = GetVideoCaptureDeviceCount();
	if(count == 0)
		return 0;

	if(ulChannelID>GetVideoCaptureDeviceCount()-1)
		ulChannelID = 0;
	if(m_bHasStartVideo2 == true)
		return 0;
	m_bHasStartVideo2 = true;
	m_bVideoMonitor = true;
	uint64_t myGuid ;
	Poco::Timestamp t;
	uint32_t ulSSRC = unsigned int(t.epochMicroseconds()&0xFFFFFFFF);
	if(videoWin)
	{
		videoWin->SetUserID(0);
		videoWin->SetUserName(GetUserName(ulToUserID));
		m_myUserInfo.ulVideoSSRC = m_pIZYMediaStreamManager->StartVideoSender(ulSSRC,ulChannelID,
			videoWin, MeetingConnImpl::GetInstance()->GetRoomID());
		if(m_myUserInfo.ulVideoSSRC > 0)
		{
			return ulSSRC;
		}
	}
	else
	{
		return 0;
	}
}

void MeetingFrameImpl::StopPublishVideo2(uint32_t ulChannelID)
{
	if(m_myUserInfo.ulVideoSSRC == 0)
		return;
	if(m_bHasStartVideo2 == false)
		return;
	m_bHasStartVideo2 =false;

	if(m_pIZYMediaStreamManager)
	{
		m_bVideoMonitor = false;
		m_pIZYMediaStreamManager->StopVideoSender(m_myUserInfo.ulVideoSSRC,MeetingConnImpl::GetInstance()->GetRoomID());
	}
	m_myUserInfo.ulVideoSSRC = 0;
}

void MeetingFrameImpl::StartPublishAudio(uint64_t ulToUserID,uint32_t ulChannelID)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(m_bStartAudio == true)
		return;
	//禁止
	if(ulChannelID == -2)
		return;
	if(ulChannelID>GetSoundCaptureDeviceCount()-1)
		ulChannelID = -1;
	m_bStartAudio = true;
	uint64_t myGuid ;
	uint32_t ulRoomID = MeetingConnImpl::GetInstance()->GetRoomID();
	m_myUserInfo.ulAudioSSRC = m_pIZYMediaStreamManager->StartAudioSender(ulChannelID,m_mainHWND,ulRoomID,
		kAudioMode,m_pRoomInfo->sampleRate);
	if(m_myUserInfo.ulAudioSSRC > 0)
	{
		char pData[64];
		myGuid = m_pIZYMediaStreamManager->GetGUID();
		memcpy(pData,&myGuid,8);
		memcpy(pData+8,&m_myUserInfo.ulAudioSSRC,4);
		if(kAudioMode !=1)
		{
			char subMsg[512] = {0};
			sprintf(subMsg,"{\"cmd\":\"recvMyAudio\",\"ssrc\":%u,\"fromSessionID\":%I64u,\"channelID\":%u}"
				,m_myUserInfo.ulAudioSSRC,m_myUserInfo.sessionID,ulChannelID);
			MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
		}
		if(m_pEvent)
			m_pEvent->On_MeetingEvent_UpdateUI();
	}
}

void MeetingFrameImpl::StopPublishVideo(uint32_t ulChannelID)
{
	if(m_myUserInfo.ulVideoSSRC == 0)
		return;
	if(m_bStartVideo == false)
		return;
	m_bStartVideo =false;

	if(m_pIZYMediaStreamManager)
	{
		m_bBroadcast = false;
		m_pIZYMediaStreamManager->StopVideoSender(m_myUserInfo.ulVideoSSRC,MeetingConnImpl::GetInstance()->GetRoomID());
	}
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"closeMyVideo\",\"fromSessionID\":%I64u,\"channelID\":%u}",m_myUserInfo.sessionID,ulChannelID);
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
	
	m_myUserInfo.ulVideoSSRC = 0;

}

bool MeetingFrameImpl::HasStartAudio()
{
	return m_bStartAudio;
}

bool MeetingFrameImpl::HasStartVideo()
{
	return m_bStartVideo;
}

void MeetingFrameImpl::StopPublishAudio(uint32_t ulChannelID)
{
	if(m_bStartAudio == false)
		return;
	m_bStartAudio = false;

	if(m_myUserInfo.ulAudioSSRC == 0)
		return;

	if(m_pEvent)
		m_pEvent->On_MeetingEvent_UpdateUI();

	if(m_pIZYMediaStreamManager)
	{
		m_pIZYMediaStreamManager->StopAudioSender(m_myUserInfo.ulAudioSSRC,MeetingConnImpl::GetInstance()->GetRoomID(),kAudioMode);
		m_myUserInfo.ulAudioSSRC = 0;

		char subMsg[512] = {0};
		sprintf(subMsg,"{\"cmd\":\"closeMyAudio\",\"fromSessionID\":%I64u,\"channelID\":%u}",m_myUserInfo.sessionID,ulChannelID);
		MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
	}
}

void MeetingFrameImpl::GiveGift(uint64_t ulToUserID,uint32_t type)  //送礼物
{
	char m_TempBuf[256] = {0};
	memcpy(m_TempBuf,&ulToUserID,4);
	memcpy(m_TempBuf+4,&type,4);

	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"giveGift\"}",m_myUserInfo.ulAudioSSRC);
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
}

void MeetingFrameImpl::ShowHandclap()  //鼓掌
{
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"showHandClap\"}",m_myUserInfo.ulAudioSSRC);
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
}

char* MeetingFrameImpl::GetUserName(uint64_t ulUserID)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(ulUserID == 0)
	{
		return m_myUserInfo.strUserName;
	}
	else
	{
		UINT_VOID_MAP::Iterator item = m_mapOnlineUser.find(ulUserID);
		if(item!=m_mapOnlineUser.end())
		{
			PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
			return pInfo->strUserName;
		}
	}
	return NULL;
}

int MeetingFrameImpl::GetUserRole(uint64_t ulUserID)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(ulUserID == 0)
	{
		return m_myUserInfo.userRole;
	}
	else
	{
		UINT_VOID_MAP::Iterator item = m_mapOnlineUser.find(ulUserID);
		if(item!=m_mapOnlineUser.end())
		{
			PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
			return pInfo->userRole;
		}
	}
	return 0;
}

int MeetingFrameImpl::GetUserType(uint64_t ulUserID)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(ulUserID == 0)
	{
		return m_myUserInfo.userRole;
	}
	else
	{
		UINT_VOID_MAP::Iterator item = m_mapOnlineUser.find(ulUserID);
		if(item!=m_mapOnlineUser.end())
		{
			PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
			return pInfo->userRole;
		}
	}
	return 0;
}
void MeetingFrameImpl::SetIVideoLayoutMgr(IVideoLayoutMgr* layout_mgr)
{
	
}

PClient_Online_User_Info MeetingFrameImpl::FindOnlineUser(uint64_t ulUserID)
{
	Mutex::ScopedLock lock(m_Mutex);
	UINT_VOID_MAP::Iterator item  = m_mapOnlineUser.find(ulUserID);
	if(item!=m_mapOnlineUser.end())
	{
		return (PClient_Online_User_Info)(item->second);
	}
	return NULL;
}

void MeetingFrameImpl::OnSetAsSpeaker(uint64_t ulRemoteUserID,bool bSet)
{
	
	PClient_Online_User_Info pOnlineUserInfo;
	if(ulRemoteUserID == m_myUserInfo.sessionID)
		pOnlineUserInfo = &m_myUserInfo;
	else
		pOnlineUserInfo= FindOnlineUser(ulRemoteUserID);
	if(!pOnlineUserInfo)
		return;
	/*
	if(bSet)
		pOnlineUserInfo->userRole = 3;
	else
		pOnlineUserInfo->userRole = 4;
    */
	if(m_pEvent)
		m_pEvent->On_MeetingEvent_SetAsSpeaker(ulRemoteUserID,bSet);
}
void MeetingFrameImpl::OnSetAsBigVideo(uint64_t ulRemoteUserID)
{
	
	//if(ulRemoteUserID!=this->m_myUserInfo.sessionID)
	{
		if(m_pEvent)
			m_pEvent->On_MeetingEvent_SetAsBigVideo(ulRemoteUserID);
	}
}

void MeetingFrameImpl::OnSetAsAdmin(uint64_t ulRemoteUserID,bool bSet)
{
	PClient_Online_User_Info pOnlineUserInfo;
	if(ulRemoteUserID == m_myUserInfo.sessionID)
		pOnlineUserInfo = &m_myUserInfo;
	else
		pOnlineUserInfo= FindOnlineUser(ulRemoteUserID);
	if(!pOnlineUserInfo)
		return;
	if(bSet)
		pOnlineUserInfo->userRole = 1;
	else
		pOnlineUserInfo->userRole = 2;
	if(m_pEvent)
		m_pEvent->On_MeetingEvent_SetAsAdmin(ulRemoteUserID,bSet);

	if(m_pEvent)
	{
		m_pEvent->On_MeetingEvent_UpdateUI();
	}
}

void MeetingFrameImpl::OnSetAsShower(uint64_t ulRemoteUserID,bool bSet)
{
	PClient_Online_User_Info pOnlineUserInfo;
	if(ulRemoteUserID == m_myUserInfo.sessionID)
		pOnlineUserInfo = &m_myUserInfo;
	else
		pOnlineUserInfo= FindOnlineUser(ulRemoteUserID);
	if(!pOnlineUserInfo)
		return;
	if(bSet)
		pOnlineUserInfo->userRole = 1;
	else
		pOnlineUserInfo->userRole = 2;
	if(m_pEvent)
		m_pEvent->On_MeetingEvent_SetAsShower(ulRemoteUserID,bSet);

	if(m_pEvent)
	{
		m_pEvent->On_MeetingEvent_UpdateUI();
	}
}

void MeetingFrameImpl::OnSetAsShowerThread()
{
	//ApplyShower();
	
}
uint64_t MeetingFrameImpl::GetMySessionID()
{ 
	return m_myUserInfo.sessionID;
};

bool MeetingFrameImpl::SpeakerAllowVideo() 
{  
	return true;
};
void MeetingFrameImpl::On_MeetingEvent_Enter_Room_Result(uint32_t status,  char* pData)
{
	Mutex::ScopedLock lock(m_Mutex);
	
	switch(status)
	{  
	case 0:
		{
			if(m_mapOnlineUser.size()>0){
				//断线重新连接成功，清空在线用户
				m_mapOnlineUser.clear();
			}

			m_myUserInfo.userRole = MeetingConnImpl::GetInstance()->m_userRole;
			PROOM_INFO pRoomInfo = (PROOM_INFO)pData;
			m_pRoomInfo = new ROOM_INFO();
			memcpy((void*)(m_pRoomInfo),pData,sizeof(ROOM_INFO));
			kAudioMode = pRoomInfo->bMixAudio;
			m_orgi_user_role = MeetingConnImpl::GetInstance()->m_userRole;
			m_myUserInfo.userRole = MeetingConnImpl::GetInstance()->m_userRole;
			//启动音频接收
			if(kAudioMode == 1)
			{
				Poco::Timestamp t;
				m_ulMixAudioSSRC = unsigned int(t.epochMicroseconds()&0xFFFFFFFF)+rand();
				m_ulAudioPlaySSRC = m_ulMixAudioSSRC;
				m_pIZYMediaStreamManager->StartAudioRecver(m_ulMixAudioSSRC,m_mainHWND,MeetingConnImpl::GetInstance()->GetRoomID(),
					kAudioMode,m_pRoomInfo->sampleRate);
			}
			if(m_pEvent)
			{
				m_pEvent->On_MeetingEvent_UpdateUI();
			}
		}
		break;
	case kLoginResult_Meeting_NoRoom:             //房间号错误
		printf("not find room ! \n");
		break;
	case kLoginResult_Meeting_RoomPwd_NOTCorrent:        //密码错误
		printf("password not correct! \n");
		break;
	case kLoginResult_Meeting_NotAuth:            //没有权限进入
		break;
	}
	m_pEvent->On_MeetingEvent_EnterRoom_Result(status,"");
}
void MeetingFrameImpl::On_MeetingEvent_Login_Result(uint32_t status,char *msg)
{
	Mutex::ScopedLock lock(m_Mutex);
	bool bHasLogined =  false;
	switch(status)
	{  
	case 0:
		{
			
			if(m_pIZYMediaStreamManager == NULL)
			{
				
				bool ret = CreateZYMediaStreamManager(&m_pIZYMediaStreamManager,m_pStreamEvent);
				if(ret)
					m_pIZYMediaStreamManager->Init(m_strMediaHost,
				                                 m_mainHWND,m_RtpMediaPort,m_MixAudioPort);
			}
			
			m_myUserInfo.sessionID =MeetingConnImpl::GetInstance()->m_mySessionID;
			m_myUserInfo.clientType = 1;
			m_myUserInfo.userRole = MeetingConnImpl::GetInstance()->m_userRole;
			strcpy(m_myUserInfo.strHeadImg,MeetingConnImpl::GetInstance()->m_strHeadImg);
			strcpy(m_myUserInfo.strUserName,MeetingConnImpl::GetInstance()->m_strUserName);
		}
		bHasLogined = true;
		break;
	case kLoginResult_Meeting_NoRoom:             //房间号错误
		printf("not find room ! \n");
		break;
	case 7:                     //用户不存在
		printf("not find user!\n");
		break;
	case kLoginResult_Password_NotCorrect:        //密码错误
		printf("password not correct! \n");
		break;
	case kLoginResult_Meeting_NotAuth:            //没有权限进入
		break;
	}
	m_pEvent->On_MeetingEvent_Login_Result(status,msg);

}
void MeetingFrameImpl::On_MeetingEvent_Member_Online(uint64_t  sessionID,char*strUserName,char*strUserAccount,uint32_t clientType,uint32_t userRole)
{
	//用户上线，插入用户列表
	if(sessionID == m_myUserInfo.sessionID)
		return;
	Mutex::ScopedLock lock(m_Mutex);
	PClient_Online_User_Info pInfo = new Client_Online_User_Info();
	pInfo->sessionID = sessionID;
	strcpy(pInfo->strUserName,strUserName);
	//用户在这个房间里面的角色
	pInfo->userRole = userRole;
	m_mapOnlineUser.insert(UINT_VOID_MAP::ValueType(sessionID,pInfo));
	if(m_pEvent)
	{
		m_pEvent->On_MeetingEvent_Member_Online(sessionID,strUserName,clientType,userRole);
	}
	//如果自己已打开视频，通知对方打开我的视频
	uint64_t myGuid ;
	//for(int i = 0;i<4;i++)
	{
	if(m_myUserInfo.ulVideoSSRC > 0 && m_bBroadcast)
	{
		char pData[64];
		myGuid = m_pIZYMediaStreamManager->GetGUID();
		memcpy(pData,&myGuid,8);
		memcpy(pData+8,&m_myUserInfo.ulVideoSSRC,4);
		Sleep(1000);
		char subMsg[512] = {0};
		sprintf(subMsg,"{\"cmd\":\"recvMyVideo\",\"ssrc\":%u,\"fromSessionID\":%I64u,\"channelID\":%u}",
			m_myUserInfo.ulVideoSSRC,m_myUserInfo.sessionID,0);
		MeetingConnImpl::GetInstance()->TransParentRoomCommand(sessionID,subMsg);
	}
	
	if(m_myUserInfo.ulAudioSSRC > 0)
	{
		uint32_t ulMixedSSRC = MeetingConnImpl::GetInstance()->GetRoomID();
		char pData[64];
		myGuid = m_pIZYMediaStreamManager->GetGUID();
		memcpy(pData,&myGuid,8);
		//memcpy(pData+8,&ulMixedSSRC,4);
		memcpy(pData+8,&m_myUserInfo.ulAudioSSRC,4);
		//如果房间不是混音模式，就通知对方接收
		if(kAudioMode!=1)
		{
			char subMsg[512] = {0};
			sprintf(subMsg,"{\"cmd\":\"recvMyAudio\",\"ssrc\":%u,\"fromSessionID\":%I64u,\"channelID\":%u}",
				m_myUserInfo.ulAudioSSRC,m_myUserInfo.sessionID,0);
			MeetingConnImpl::GetInstance()->TransParentRoomCommand(sessionID,subMsg);
		}
	}

	}//end for
}
void MeetingFrameImpl::On_MeetingEvent_Member_Offline(uint64_t  sessionID)    //用户下线
{
	//用户下线，从用户列表中删除，如果打开了这个用户的音视频，关掉
	Mutex::ScopedLock lock(m_Mutex);
	UINT_VOID_MAP::Iterator item = m_mapOnlineUser.find(sessionID);
	if(item!=m_mapOnlineUser.end())
	{
		//检查是否打开了用户的音视频
		PClient_Online_User_Info pInfo = (PClient_Online_User_Info)(item->second);
		//关闭音频播放器
		if(pInfo->ulAudioSSRC>0)
		{
			m_pIZYMediaStreamManager->StopAudioRecver(pInfo->ulAudioSSRC,MeetingConnImpl::GetInstance()->GetRoomID(),kAudioMode);
		}

		//关闭视频播放器
		if(pInfo->ulVideoSSRC>0)
		{
			m_pIZYMediaStreamManager->StopVideoRecver(pInfo->ulVideoSSRC,MeetingConnImpl::GetInstance()->GetRoomID());
		}
		delete item->second;
		m_mapOnlineUser.erase(item);
	}

	if(m_pEvent)
	{
		m_pEvent->On_MeetingEvent_Member_Offline(sessionID);
	}
}

void MeetingFrameImpl::GetLocalDateTime(TCHAR* pstrTemp)
{
	DateTime dt;
	wsprintf(pstrTemp,L"%u-%u-%u %u:%u:%u",dt.year(),dt.month(),dt.day(),dt.hour(),dt.minute(),dt.second());
	
}



//得到主持人密码
char*  MeetingFrameImpl::GetAdminPassword()    
{
	Mutex::ScopedLock lock(m_Mutex);
	if(m_pRoomInfo)
		return m_pRoomInfo->strAdminPwd;
	else
		return NULL;
}

bool MeetingFrameImpl::GetCurrentRoomInfo(PROOM_INFO pRoomInfo)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(m_pRoomInfo!=NULL)
	{
		memcpy(pRoomInfo,m_pRoomInfo,sizeof(ROOM_INFO));
		return true;
	}
	return false;
}

//收到用户申请发言人请求
void MeetingFrameImpl::OnApplySpeaker(uint64_t ulFromUserID)
{
	//通知界面层

	if(m_pEvent)
		m_pEvent->On_MeetingEvent_ApplySpeaker(ulFromUserID);

}
//收到用户取消发言人请求
void MeetingFrameImpl::OnCancelApplySpeaker(uint64_t ulFromUserID)
{
	//通知界面层

	if(m_pEvent)
		m_pEvent->On_MeetingEvent_CancelApplySpeaker(ulFromUserID);

}


void MeetingFrameImpl::SetMicMute(bool bMute, int ulChannelID )
{
	if(m_pIZYMediaStreamManager && m_myUserInfo.ulAudioSSRC)
	{
		m_pIZYMediaStreamManager->SetMute(m_myUserInfo.ulAudioSSRC,bMute);
	}
}

void MeetingFrameImpl::SetMicVolume(int nVolume, int ulChannelID)
{
	if(m_pIZYMediaStreamManager && m_myUserInfo.ulAudioSSRC)
	{
		m_pIZYMediaStreamManager->SetVolume(m_myUserInfo.ulAudioSSRC,nVolume);
	}
}

void MeetingFrameImpl::RequireKeyFrame(uint32_t ssrc)
{
	if(m_pIZYMediaStreamManager && m_myUserInfo.ulVideoSSRC)
	{
		m_pIZYMediaStreamManager->RequireKeyFrame(ssrc);
	}
}
//播放音量
void MeetingFrameImpl::SetSpeakerMute(bool bMute)
{
	if(m_pIZYMediaStreamManager)
	{
		m_pIZYMediaStreamManager->SetMute(m_ulAudioPlaySSRC,bMute);
	}
}

void MeetingFrameImpl::ReConnect()
{
	MeetingConnImpl::GetInstance()->ReConnect();
}



void MeetingFrameImpl::SetSpeakerVolume(int nVolume)
{
	if(m_pIZYMediaStreamManager)
	{
		m_pIZYMediaStreamManager->SetVolume(m_ulAudioPlaySSRC,nVolume);
	}
}


//得到在线用户数
int MeetingFrameImpl::GetUserCount()
{
	Mutex::ScopedLock lock(m_Mutex);
	return  m_mapOnlineUser.size()+1;
}

void MeetingFrameImpl::GetRoomList()
{
	MeetingConnImpl::GetInstance()->GetRoomList();
}

PROOM_INFO MeetingFrameImpl::GetRoomInfo(int index)
{
	if(index<0||index>2000)
		return NULL;
	return MeetingConnImpl::GetInstance()->GetRoomInfo(index);
}

//得到用户属性，用户名，角色
bool MeetingFrameImpl::GetUser(int index,PClient_Online_User_Info pUserInfo)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(index == 0)
	{
		memcpy(pUserInfo,&m_myUserInfo,sizeof(Client_Online_User_Info));
		return true;
	}
	else
	{
		if(index<=m_mapOnlineUser.size())
		{
			
			UINT_VOID_MAP::Iterator item = m_mapOnlineUser.begin();
			int i = 0;
			while(item!=m_mapOnlineUser.end())
			{
				i++;
				if(i == index)
				{
					PClient_Online_User_Info user = (PClient_Online_User_Info)(item->second);
					memcpy(pUserInfo,user,sizeof(Client_Online_User_Info));
					return true;
				}
				item++;
			}
			
		}
			
	}
	
	return false;
}

void MeetingFrameImpl::On_MeetingEvent_SpeakMode_Change(int mode)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(m_pRoomInfo)
	{
		if(mode == m_pRoomInfo->speakMode)
			return;
		m_pRoomInfo->speakMode = mode;

		if(m_pEvent)
		{
			m_pEvent->On_MeetingEvent_SpeakMode_Change(mode);
		}
	}
}

void MeetingFrameImpl::On_MeetingEvent_RoomMode_Change(int mode)
{
	Mutex::ScopedLock lock(m_Mutex);
	if(m_pRoomInfo)
	{
		if(mode == m_pRoomInfo->roomMode)
			return;
		m_pRoomInfo->roomMode = mode;
		if(m_pEvent)
		{
			m_pEvent->On_MeetingEvent_RoomMode_Change(mode);
		}
	}
}

//得到聊天消息
PMsgItem MeetingFrameImpl::GetChatMsgItem(int index)
{
	return MeetingConnImpl::GetInstance()->GetChatMsgItem(index);
}

int  MeetingFrameImpl::GetRoomMode()
{
	Mutex::ScopedLock lock(m_Mutex);
	if(m_pRoomInfo!=NULL)
	{
		return m_pRoomInfo->roomMode;
	}
	else
	{
		return 1;
	}
}

void MeetingFrameImpl::StartVideoMonitor()
{
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"startVideoMonitor\"}");
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
}
void MeetingFrameImpl::StopVideoMonitor()
{
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"stopVideoMonitor\"}");
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(0,subMsg);
}

//暂停发送视频
void MeetingFrameImpl::PauseVideo(uint64_t userId)
{
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"pauseVideo\"}");
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(userId,subMsg);
}

//恢复发送视频
void MeetingFrameImpl::ResumeVideo(uint64_t userId)
{
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"resumeVideo\"}");
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(userId,subMsg);
}

//发送我的视频编号给主持人，以方便他查看
void MeetingFrameImpl::SendVideoSSRCToAdmin(uint64_t userId,uint32_t ssrc)
{
	char subMsg[512] = {0};
	sprintf(subMsg,"{\"cmd\":\"sendVideoSSRCToAdmin\",\"ssrc\":%u}",ssrc);
	MeetingConnImpl::GetInstance()->TransParentRoomCommand(userId,subMsg);
}

void MeetingFrameImpl::OnStartVideoMonitor(){
	if(m_pEvent){
		m_pEvent->On_MeetingEvent_StartVideoMonitor();
	}
}
void MeetingFrameImpl::OnStopVideoMonitor(){
	if(m_pEvent){
		m_pEvent->On_MeetingEvent_StopVideoMonitor();
	}
}
void MeetingFrameImpl::OnPauseVideo(){
	if(m_pEvent){
		m_pEvent->On_MeetingEvent_PauseVideo();
	}
}
void MeetingFrameImpl::OnResumeVideo(){
	if(m_pEvent){
		m_pEvent->On_MeetingEvent_ResumeVideo();
	}
}

void MeetingFrameImpl::OnRecvVideoSSRC(uint64_t userId,uint32_t ssrc){
	if(m_pEvent){
		m_pEvent->On_MeetingEvent_RecvUserVideoSSRC(ssrc,userId,"aa");
	}
}