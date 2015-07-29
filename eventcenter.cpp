#include "eventcenter.h"
#include "login/UserLoginWdg.h"
#include "Room/RoomWdg.h"
#include "Video/VideoWin.h"
#include "Video/VideoMgr.h"
#include "Video/VideoMonitorMgr.h"
#include "VideoMonitor/videomonitormain.h"
#include "Room/LeftWdg.h"
#include "Room/WdgNetInfo.h"
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
IMeetingFrame* g_pMeetingFrame = NULL;
IZYMediaStreamManager *g_pStreamMgr = NULL;

EventCenter*EventCenter::m_instahce = NULL;
EventCenter* EventCenter::GetInstance()
{
	
	return m_instahce;
}

EventCenter::EventCenter(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	m_instahce = this;
	m_bInRoom = false;
	m_roomId = 0;
	//setWindowFlags(Qt::FramelessWindowHint);//这个是widget的标题栏和边框去掉
	//setAttribute(Qt::WA_TranslucentBackground);
	this->hide();
	//RoomWdg::GetInstance()->CenterWindow();
	//RoomWdg::GetInstance()->showMaximized();
	
	UserLoginWdg::GetInstance()->show();
	//初始化视频会议框架
	CreateMeetingFrame(&g_pMeetingFrame,this,(HWND)RoomWdg::GetInstance()->winId(),
		this,AppSetting::GetInstance()->GetServerAddress().toStdString());
	connect(this,SIGNAL(EventLoginSuccess()),this,SLOT(On_UI_LoginSuccess()));
	connect(this,SIGNAL(EventLoginFailed()),UserLoginWdg::GetInstance(),SLOT(OnLoginFailed()));
	connect(this,SIGNAL(EventAddUser()),LeftWdg::GetInstance(),SLOT(OnAddUser()));
	connect(this,SIGNAL(EventDelUser()),LeftWdg::GetInstance(),SLOT(OnDelUser()));
	connect(this,SIGNAL(EventGetRoomList(int)),UserLoginWdg::GetInstance(),SLOT(OnGetRoomList(int)));
	connect(this,SIGNAL(EventUpdateUI()),this,SLOT(OnUpdateUI()));

	connect(this,SIGNAL(EventSetSpeaker(bool)),this,SLOT(OnSetSpeaker(bool)));
	connect(this,SIGNAL(EventSpeakModeChange(int)),this,SLOT(OnSpeakModeChange(int)));
	
	connect(this,SIGNAL(EventRoomModeChange(int)),this,SLOT(OnRoomModeChange(int)));
	connect(this,SIGNAL(EventOnStreamNetInfo(int,int,int)),WdgNetInfo::GetInstance(),SLOT(OnNetInfo(int,int,int)));
	connect(this,SIGNAL(EventOnStreamNetInfo(int,int,int)),RoomWdg::GetInstance(),SLOT(OnNetInfo(int,int,int)));
	// sean add 收到消息后发送信号
	connect(this,SIGNAL(getMessageEvent(unsigned long long ,QString*, 
		int ,unsigned long ,unsigned long ,QString *)),LeftWdg::GetInstance(),SLOT(getMessageFromOthers(unsigned long long ,QString*, int ,unsigned long ,unsigned long ,QString *)));

	connect(this,SIGNAL(EventUserOffline(unsigned long long)),this,SLOT(OnUserOffline(unsigned long long )));

	connect(this,SIGNAL(EventRecvRemoteVideo(unsigned long long)),
		this,SLOT(OnRecvRemoteVideo(unsigned long long)));

	connect(this,SIGNAL(EventSetAsBigVideo(unsigned long long)),
		this,SLOT(OnSetAsBigVideo(unsigned long long)));
	connect(this,SIGNAL(EventEnterRoom(unsigned int)),this,SLOT(OnEnterRoom(unsigned int)));
	//打开文档
	connect(this,SIGNAL(EventOpenDoc(QString,int,int)),RoomWdg::GetInstance(),
		SLOT(OnOpenDoc(QString,int,int)));
	//关闭文档
	connect(this,SIGNAL(EventCloseDoc()),RoomWdg::GetInstance(),SLOT(OnCloseDoc()));

	//共享光标
	connect(this,SIGNAL(EventShowRemoteCursor(float,float)),RoomWdg::GetInstance(),
		SLOT(OnShowRemoteCursor(float,float)));
	//关闭光标
	connect(this,SIGNAL(EventCloseRemoteCursor()),RoomWdg::GetInstance(),SLOT(OnCloseRemoteCursor()));
	connect(this,SIGNAL(EventOnStartVideoMonitor()),this,SLOT(On_UI_StartVideoMonitor()));
	connect(this,SIGNAL(EventOnStopVideoMonitor()),this,SLOT(On_UI_StopVideoMonitor()));
	connect(this,SIGNAL(EventRecvVideoSSRC(unsigned long long ,unsigned int)),
		this,SLOT(On_UI_RecvVideoSSRC(unsigned long long ,unsigned int)));
	connect(this,SIGNAL(EventUpdateSoft(QString)),this,SLOT(On_UI_UpdateSoft(QString)));
	connect(this,SIGNAL(EventNetEvent(unsigned int)),this,SLOT(On_UI_NetEvent(unsigned int)));
	//掉线重连定时器
	m_reConnectTimer = new QTimer(this);
	m_reConnectTimer->setInterval(1000*3);
	connect(m_reConnectTimer, SIGNAL(timeout()), this,SLOT(OnReConnectTimer()));
}

EventCenter::~EventCenter()
{

}


bool EventCenter::winEvent (MSG* msg, long *result)
{
	switch(msg->message)
	{
	case WM_CREATE:
		
		break;
	}
	return FALSE;
}

void EventCenter::OnEnterRoom(unsigned int result)
{
	//进入房间成功
	if(result == 0)
	{
		UserLoginWdg::GetInstance()->setHidden(true);
		MeetingListWdg::GetTableViewInstance()->hide();
	}
	else
	{
		UserLoginWdg::GetInstance()->setHidden(false);
		MeetingListWdg::GetTableViewInstance()->hide();
		RoomWdg::GetInstance()->setHidden(true);
		QMessageBox::information(NULL,"提示","用户达到房间最大数");
	}
}

void EventCenter::On_MeetingEvent_ShareCursor(float x,float y)
{
	
	emit EventShowRemoteCursor(x,y);
}

void EventCenter::On_MeetingEvent_StopShareCursor()
{
	qDebug()<<"On_MeetingEvent_StopShareCursor()";
	emit EventCloseRemoteCursor();
}

void EventCenter::On_MeetingEvent_EnterRoom_Result(uint32_t status,char *msg)
{
	emit EventEnterRoom(status);
	emit EventAddUser();
	m_bInRoom = true;
	if(g_pMeetingFrame!=NULL)
	{
		ROOM_INFO roomInfo;
		if(g_pMeetingFrame->GetCurrentRoomInfo(&roomInfo))
		{
			if(roomInfo.speakMode == 1)
			{
				g_pMeetingFrame->StartPublishAudio(0,AppSetting::GetInstance()->GetMicIndex()-1);
			}
		}
	}
}
//返回用户登录结果
void EventCenter::On_MeetingEvent_Login_Result(uint32_t status,char *msg)
{
	qDebug()<<status<<msg;
	if(status == 0)
	{
		emit EventLoginSuccess();
	}
	else
	{
		emit EventLoginFailed();
	}
}
//用户上线
void EventCenter::On_MeetingEvent_Member_Online(uint64_t ulUserID,char*strUserName,
	uint32_t userType,uint32_t userRoomType)
{
	qDebug()<<"user online"<<QString::fromUtf8(strUserName)<<ulUserID;
	emit EventAddUser();
	emit EventUpdateUI();
}
//用户下线
void EventCenter::On_MeetingEvent_Member_Offline(uint64_t ulUserID)
{
	
	emit EventUserOffline(ulUserID);
	emit EventDelUser();
}

void EventCenter::OnUserOffline(unsigned long long sessionID)
{
	if(g_pMeetingFrame)
	{
		VideoMgr::GetInstance()->GivebackVideo(sessionID);
		VideoMonitorMain::GetInstance()->RemoveVideo(sessionID);
	}

	emit EventUpdateUI();
}

void EventCenter::On_MeetingEvent_UpdateSoft(std::string url)
{
	emit EventUpdateSoft(QString(url.data()));
}

void EventCenter::On_UI_UpdateSoft(QString url)
{
	QString strTip;
	if(url.isEmpty())
		return;
	strTip = QString("软件现在的版本号是：%1,与服务器版本号不一致，请点击确定按钮自动更新，或者手动下载，下载地址：%2").
		arg(g_pMeetingFrame->GetVersion()).arg(url);
	QMessageBox::StandardButton rb = QMessageBox::warning(NULL,"升级提示",strTip,
		QMessageBox::Cancel|QMessageBox::Ok);
	if(rb == QMessageBox::Ok){
		//启动更新软件进程
		QProcess *pProcess = new QProcess(this);
		QStringList argList;
		argList.append(url);
		pProcess->startDetached("AutoUpdate.exe",argList); 
		g_pMeetingFrame->Release();
		QApplication::quit();
	}
}
void EventCenter::On_MeetingEvent_GetRoomList(uint32_t total)
{
	//qDebug()<<"On_MeetingEvent_GetRoomList";
	emit EventGetRoomList(total);
}
//收到文本聊天消息
void EventCenter::On_MeetingEvent_Text_Msg(uint64_t ulUserID,char*strMsg, 
	int font_size,unsigned long fontFlat,unsigned long color,char *familly)
{
	qDebug()<<"On_MeetingEvent_Text_Msg == \n"<<strMsg;
	QString *str = new QString(strMsg);
	QString *familyStr = new QString(familly);
	emit getMessageEvent(ulUserID,str,font_size,fontFlat,color,familyStr);


}

void EventCenter::On_MeetingEvent_OpenDoc(std::string strUrl,int index,int total)
{
	qDebug()<<"open url:"<<strUrl.data()<<"index:"<<index<<" total:"<<total;
	emit EventOpenDoc(QString(strUrl.data()),index,total);
}

//关闭文档共享
void EventCenter:: On_MeetingEvent_CloseDoc()
{
	emit EventCloseDoc();
}

//通知界面层被设置成发言人
void EventCenter::On_MeetingEvent_SetAsSpeaker(uint64_t ulUserID,bool bSet)
{
	//emit EventSetSpeaker(bSet);

	if(bSet)
	{
		g_pMeetingFrame->StartPublishAudio(0,AppSetting::GetInstance()->GetMicIndex()-1);
	}
	else
	{
		g_pMeetingFrame->StopPublishAudio(0);
	}
}

void EventCenter::OnSetSpeaker(bool bSet)
{
	if(bSet)
	{
		g_pMeetingFrame->StartPublishAudio(0,AppSetting::GetInstance()->GetMicIndex()-1);
	}
	else
	{
		g_pMeetingFrame->StopPublishAudio(0);
	}
}
//通知界面层被设置成为主讲
void EventCenter::On_MeetingEvent_SetAsShower(uint64_t ulUserID,bool bSet)
{

}
//通知界面层被设置成为管理员
void EventCenter::On_MeetingEvent_SetAsAdmin(uint64_t ulUserID,bool bSet)
{

}
//通知主讲人或者管理员，有普通用户申请了发言
void EventCenter::On_MeetingEvent_ApplySpeaker(uint64_t fromUserID)
{

}
//通知界面层用户取消了发言请求
void EventCenter::On_MeetingEvent_CancelApplySpeaker(uint64_t fromUserID)
{

}
//通知界面层，用户角色改变了
void EventCenter::On_MeetingEvent_UserRole_Change(uint64_t ulUserID,int role)
{

}
//通知界面接收视频
void EventCenter::On_MeetingEvent_RecvMyVideo(uint64_t sessionID,uint32_t channelID)
{
	qDebug()<<"RecvMyVideo:"<<sessionID<<":"<<channelID;
	qDebug()<<"******************OnRecvRemoteVideo1111111111111111111*************";
	
	emit EventRecvRemoteVideo(sessionID);
	
	
}

//发言模式有了变化
void EventCenter::On_MeetingEvent_SpeakMode_Change(int mode)
{
	emit EventSpeakModeChange(mode);
}

//发言模式有了变化
void EventCenter::On_MeetingEvent_RoomMode_Change(int mode)
{
	emit EventRoomModeChange(mode);
}

void EventCenter::OnRoomModeChange(int mode)
{
	qDebug()<<"OnRoomModeChange"<<mode;
	RoomWdg::GetInstance()->OnRoomModeChange(mode);
	VideoMgr::GetInstance()->OnRoomModeChange(mode);
}

void EventCenter::OnSpeakModeChange(int mode)
{
	
	if(mode == 1)
	{
		//如果用户还没有开始语音，帮他打开语音
		if(g_pMeetingFrame->HasStartAudio() == false)
			g_pMeetingFrame->StartPublishAudio(0,AppSetting::GetInstance()->GetMicIndex()-1);
	}
	else
	{
		//如果用户已经打开语音，帮他关掉
		if(g_pMeetingFrame->HasStartAudio() == true)
			g_pMeetingFrame->StopPublishAudio(0);
	}
	
	OnUpdateUI();
}
void EventCenter::On_MeetingEvent_CloseMyVideo(uint64_t sessionID,uint32_t channelID)
{
	if(g_pMeetingFrame)
	{
		VideoMgr::GetInstance()->GivebackVideo(sessionID);
		g_pMeetingFrame->StopRecvRemoteVideo(sessionID);
		emit EventUpdateUI();
	}
}
// 网络状态;
void EventCenter::OnNetEvent(uint32_t code,const char* msg)
{
	qDebug()<<"OnNetEvent"<<code<<" msg:"<<msg;
	emit EventNetEvent(code);
}

void EventCenter::On_UI_NetEvent(unsigned int code){
	switch(code){
	case NET_EVENT_CONNECT_SUCCESS:
		qDebug()<<"连接服务器成功";
		if(m_reConnectTimer!=NULL&m_reConnectTimer->isActive())
			m_reConnectTimer->stop();
		break;
	case NET_EVENT_LOST_CONNECTION:
		qDebug()<<"和服务器断开连接了";
		
		if(g_pMeetingFrame)
			g_pMeetingFrame->StopAllMediaStream();
		VideoMgr::GetInstance()->FreeAll();
		if(m_reConnectTimer)
			m_reConnectTimer->start();
		break;
	case NET_EVENT_TIME_OUT:
		//QMessageBox::warning(NULL,"提示","连接服务器超时");
		break;
	}
}
//掉线重新连接
void EventCenter::OnReConnectTimer(){
	if(g_pMeetingFrame)
		g_pMeetingFrame->ReConnect();
}

void EventCenter::OnRecvRemoteVideo(unsigned long long sessionID)
{
	if(g_pMeetingFrame!=NULL)
	{
		IVideoWin * videoWin = VideoMgr::GetInstance()->GetFreeVideo();
		qDebug()<<"******************OnRecvRemoteVideo22222222*************";
		if(videoWin == NULL)
		{
			MessageBox(NULL,L"视频位置已占满",NULL,NULL);
			return;
		}
		videoWin->SetUserID(sessionID);
		g_pMeetingFrame->StartRecvRemoteVideo(sessionID,0,videoWin);
	}
}

void EventCenter::On_MeetingEvent_UpdateUI()
{
	emit EventUpdateUI();
}

void EventCenter::OnUpdateUI()
{
	LeftWdg::GetInstance()->UpdateUI();
	RoomWdg::GetInstance()->UpdateUI();
	VideoMgr::GetInstance()->UpdateUI();
}

void EventCenter::StreamEvent_OnNetInfo(int delay,int uploadLost,int downloadLost)
{
	emit EventOnStreamNetInfo(delay,uploadLost,downloadLost);
}

//声音的大小,bCapture是否为采集声音，ulSSRC编号,value 能量值
void EventCenter::StreamEvent_AudioEnergy(bool bCapture,uint32_t ulSSRC,int16_t value)
{
	LeftWdg::GetInstance()->SetAudioValue(ulSSRC,value);
	//emit EventUpdateUI();
	//qDebug()<<"StreamEvent_AudioEnergy Value: "<<value;
}
//得到第一个关键帧，视频开始可以显示出来
void EventCenter::StreamEvent_GetFirstKeyFrame(uint32_t ulSSRC)
{
	//qDebug()<<"Get First Key Video Frame";
}

void EventCenter::On_MeetingEvent_SetAsBigVideo(uint64_t sessionID)
{
	emit EventSetAsBigVideo(sessionID);
}

void EventCenter::OnSetAsBigVideo(unsigned long long sessionID)
{
	if(sessionID == g_pMeetingFrame->GetMySessionID())
		VideoMgr::GetInstance()->SetBigVideo(0);
	else
		VideoMgr::GetInstance()->SetBigVideo(sessionID);
}

void EventCenter::On_MeetingEvent_StartVideoMonitor(){
	emit EventOnStartVideoMonitor();
}
void EventCenter::On_MeetingEvent_StopVideoMonitor(){
	emit EventOnStopVideoMonitor();
}

//暂停发送视频
void EventCenter::On_MeetingEvent_PauseVideo(){

}
//恢复发送视频
void EventCenter::On_MeetingEvent_ResumeVideo(){

}

void EventCenter::On_MeetingEvent_RecvUserVideoSSRC(uint32_t ssrc,uint64_t sessionId,char* userName){
	emit EventRecvVideoSSRC((unsigned long long)sessionId,(unsigned int)ssrc);
}

void EventCenter::On_UI_StartVideoMonitor(){
	RoomWdg::GetInstance()->OnStartVideoMonitor();
}
void EventCenter::On_UI_StopVideoMonitor(){
	RoomWdg::GetInstance()->OnStopVideoMonitor();
}
void EventCenter::On_UI_RecvVideoSSRC(unsigned long long sessionID,unsigned int ssrc){
	qDebug()<<"recv ssrc:"<<ssrc<<"session id:"<<sessionID;
	if(g_pMeetingFrame->GetUserRole(0)==1)
		VideoMonitorMain::GetInstance()->AddVideo(sessionID,ssrc);
	
}

void EventCenter::On_UI_LoginSuccess(){
	if(m_bInRoom == false){
		UserLoginWdg::GetInstance()->OnLoginSuccess();
	}else{
		//重新连接成功处理
		if(m_roomId!=0){
			g_pMeetingFrame->EnterRoom(m_roomId);
		}
	}
}