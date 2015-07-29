#ifndef EVENTCENTER_H
#define EVENTCENTER_H

#include <QtGui/QWidget>
#include "stdafx.h"
#include <map>
#include <QTimer>
class EventCenter : public QWidget,public IMeetingFrameEvent,public IMediaStreamEvent
{
	Q_OBJECT

public:
	EventCenter(QWidget *parent = 0, Qt::WFlags flags = 0);
	~EventCenter();
public:
	static EventCenter*GetInstance();
	void setRoomId(unsigned int roomId){ m_roomId = roomId;};
private:
	static EventCenter* m_instahce;
public:
	virtual void On_MeetingEvent_ShareCursor(float x,float y);

	virtual void On_MeetingEvent_StopShareCursor();

	virtual void On_MeetingEvent_EnterRoom_Result(uint32_t status,char *msg);
	//返回用户登录结果
	virtual void On_MeetingEvent_Login_Result(uint32_t status,char *msg);
	//用户上线
	virtual void On_MeetingEvent_Member_Online(uint64_t ulUserID,char*strUserName,
		uint32_t userType,uint32_t userRoomType);      //用户上线
	//用户下线
	virtual void On_MeetingEvent_Member_Offline(uint64_t ulUserID);     //用户下线
	//收到文本聊天消息
	virtual void On_MeetingEvent_Text_Msg(uint64_t ulUserID,char*strMsg, 
		int font_size =14,unsigned long fontFlat = 0,unsigned long color = 0,char *familly = "");

	//通知界面层被设置成发言人
	virtual void On_MeetingEvent_SetAsSpeaker(uint64_t ulUserID,bool bSet);
	//通知界面层被设置成为主讲
	virtual void On_MeetingEvent_SetAsShower(uint64_t ulUserID,bool bSet);
	//通知界面层被设置成为管理员
	virtual void On_MeetingEvent_SetAsAdmin(uint64_t ulUserID,bool bSet);
	//通知主讲人或者管理员，有普通用户申请了发言
	virtual void On_MeetingEvent_ApplySpeaker(uint64_t fromUserID);
	//通知界面层用户取消了发言请求
	virtual void On_MeetingEvent_CancelApplySpeaker(uint64_t fromUserID);
	//通知界面层，用户角色改变了
	virtual void On_MeetingEvent_UserRole_Change(uint64_t ulUserID,int role);
	//通知界面接收视频
	virtual void On_MeetingEvent_RecvMyVideo(uint64_t sessionID,uint32_t channelID);
	//通知界面接收视频
	virtual void On_MeetingEvent_CloseMyVideo(uint64_t sessionID,uint32_t channelID);
	// 网络状态;
	virtual void OnNetEvent(uint32_t code,const char* msg="");
	//得到房间列表
	virtual void On_MeetingEvent_GetRoomList(uint32_t total);

	virtual void On_MeetingEvent_UpdateUI();

	//发言模式有了变化
	virtual void On_MeetingEvent_SpeakMode_Change(int mode);

	virtual void On_MeetingEvent_RoomMode_Change(int mode);

	virtual void On_MeetingEvent_SetAsBigVideo(uint64_t sessionID);

	virtual void StreamEvent_OnNetInfo(int delay,int uploadLost,int downloadLost);
	//声音的大小,bCapture是否为采集声音，ulSSRC编号,value 能量值
	virtual void StreamEvent_AudioEnergy(bool bCapture,uint32_t ulSSRC,int16_t value);
	//得到第一个关键帧，视频开始可以显示出来
	virtual void StreamEvent_GetFirstKeyFrame(uint32_t ulSSRC);

	virtual void On_MeetingEvent_OpenDoc(std::string strUrl,int index,int total);

	virtual void On_MeetingEvent_CloseDoc();

	virtual void On_MeetingEvent_StartVideoMonitor() ;
	virtual void On_MeetingEvent_StopVideoMonitor() ;
	//暂停发送视频
	virtual void On_MeetingEvent_PauseVideo();
	//恢复发送视频
	virtual void On_MeetingEvent_ResumeVideo() ;

	virtual void On_MeetingEvent_RecvUserVideoSSRC(uint32_t ssrc,uint64_t sessionId,char* userName);

	virtual void On_MeetingEvent_UpdateSoft(std::string url);
signals:
	void EventLoginSuccess();
	void EventLoginFailed();
	void EventRecvRemoteVideo(unsigned long long sessionID);
	void EventAddUser();
	void EventDelUser();
	void EventGetRoomList(int total);
	void EventUpdateUI();

	void EventSetSpeaker(bool bSet);
	void EventSpeakModeChange(int mode);
	void EventRoomModeChange(int mode);
	void EventUserOffline(unsigned long long sessionID);
	void EventOnStreamNetInfo(int delay,int uploadLost,int downloadLost);
	void EventSetAsBigVideo(unsigned long long sessionID);
	void EventEnterRoom(unsigned int result);
	void EventOpenDoc(QString url,int index,int total);
	void EventCloseDoc();
	void EventCloseRemoteCursor();
	void EventShowRemoteCursor(float x,float y);
	void EventOnStartVideoMonitor();
	void EventOnStopVideoMonitor();
	void EventRecvVideoSSRC(unsigned long long sessionId,unsigned int ssrc);
	void EventUpdateSoft(QString url);
	void EventNetEvent(unsigned int code);
	//sean add
	//void getMessageEvent();
	void getMessageEvent(unsigned long long ulUserID,QString* strMsg, 
		int font_size,unsigned long fontFlat,unsigned long color,QString *familly);

public slots:
	void OnRecvRemoteVideo(unsigned long long sessionID);
	void OnUpdateUI();
	void OnSetSpeaker(bool bSet);
	void OnSpeakModeChange(int mode);
	void OnRoomModeChange(int mode);
	void OnUserOffline(unsigned long long sessionID);
	void OnSetAsBigVideo(unsigned long long sessionID);
	void OnEnterRoom(unsigned int result);
	void On_UI_StartVideoMonitor();
	void On_UI_StopVideoMonitor();
	void On_UI_RecvVideoSSRC(unsigned long long sessionID,unsigned int ssrc);
	void On_UI_UpdateSoft(QString url);
	void On_UI_NetEvent(unsigned int code);
	void On_UI_LoginSuccess();
	void OnReConnectTimer();
protected:
	virtual bool winEvent (MSG* msg, long *result);
	QTimer* m_reConnectTimer;
	bool m_bInRoom;
	unsigned int m_roomId;
};

#endif // EVENTCENTER_H
