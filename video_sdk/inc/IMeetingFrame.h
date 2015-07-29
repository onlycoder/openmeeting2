#pragma once

//客户端类型
enum ClientType
{
	kClientType_Windows,
	kClientType_Android,
	kClienttype_ios
};

//登录结果
enum LoginResult
{
	kLoginResult_NoUser,                     //用户不存在
	kLoginResult_Password_NotCorrect,        //密码错误
	kLoginResult_Meeting_NoRoom,             //房间号错误
	kLoginResult_Meeting_NotAuth,            //没有权限进入
	kLoginResult_LoginServer_NotConnect,      //登录服务器没有连接上
	kLoginResult_Meeting_SUCCESS,
	kLoginResult_MeetingServer_NotConnect,
	kLoginResult_Meeting_RoomPwd_NOTCorrent   //房间密码不正确
};

typedef struct tag_Room_Info
{
	uint32_t ulRoomID;             //房间号
	uint32_t speakMode;            //房间语音类型
	uint32_t roomMode;             //房间类型 1会议，2直播   
	uint32_t ulIsPublic;           //是否公开
	uint32_t bMixAudio;          //是否采用服务器混音模式
	uint32_t sampleRate;         //音频采样率，默认8000
	uint32_t defaultVideoSize;    //主进人视频大小
	uint64_t bigVideoUser;        //主会场的用户ID
	uint64_t startTime;           //开始时间
	uint64_t endTime;             //结束时间
	uint32_t maxUser;
	uint32_t onlineUser;          //在线用户数
	char strAdminPwd[64];         //管理员密码
	char strPassword[64];        //房间密码
	char  strRoomName[256];      //房间名 
}ROOM_INFO,*PROOM_INFO;


//通道，包括音频和视频的ssrc,为0代表没有
typedef struct tag_Media_Channel
{
	uint32_t isEmpty;      //是否为空
	uint32_t ulAudioSSRC;
	uint32_t ulVideoSSRC;
}Media_Channel,*PMedia_Channel;

typedef struct tag_Client_Online_User_Info
{
	uint64_t sessionID;
	char strHeadImg[64];    //用户登录帐号
	char strUserName[64];       //用户名
	unsigned char userRole;             //用户类型  1 主持人 2  普通用户
	unsigned char clientType; //客户端类型
	uint32_t ulAudioSSRC;
	uint32_t ulVideoSSRC;
}Client_Online_User_Info,*PClient_Online_User_Info;

//聊天消息结构体
typedef struct tag_Msg_Item
{
	uint64_t ulUserID;
	char userName[64];
	char msg[1024*4];
	int font_size;
	unsigned long fontFlat;  //斜体，粗体，下划线等
	unsigned long color;
    char familly[32];
}MsgItem,*PMsgItem;

#define WM_RECV_REMOTE_VIDEO WM_USER+10
#define WM_PUBLISH_AV        WM_USER+11

#define NET_EVENT_CONNECT_SUCCESS    0  //连接服务器成功
#define NET_EVENT_LOST_CONNECTION    1  //掉线
#define NET_EVENT_TIME_OUT           2  //连接服务器超时
#define NET_EVENT_JSON_ERROR         3  //json 数据解析错误


#define LOGIN_SUCESS                0
#define LOGIN_NO_USER               1
#define LOGIN_DB_ERROR              2
#define LOGIN_PASSWORD_ERROR        3

#define CLIENT_TYPE_WINDOWS         1
#define CLIENT_TYPE_ANDROID         2
#define CLIENT_TYPE_IPHONE          3
#define CLIENT_TYPE_IPAD            4


/************************************************************************/
/* 
					视频会议框架接口                                                                    
*/
/************************************************************************/

//界面层调用底层功能接口
class IMeetingFrame{

	/*****************登录，注销********************/
public:
	virtual void Release() = 0;
	virtual bool Login(char* strUserName,char* strPassword,uint32_t ulRoomID,bool bEncrypt = true) = 0;
	virtual void LoginWithRoomNo(const char* strUserName, const char* strRoomPassword,uint32_t clientType,uint32_t u32RoomID) = 0;
	virtual bool Logout() = 0;
	//进入房间 uiRoomID 房间号，strRoomPwd 有密码保护的房间需要密码，strToken,用户登录成功后会返回一个Token值
	virtual bool EnterRoom(uint32_t uiRoomID,char *strRoomPwd = "",char*strToken="") = 0;
	virtual void LeaveRoom() = 0;
	
	/************************************************************************/
	/*        视频,语音                                                     */
	/************************************************************************/
	virtual void UpdateVideoLayout() = 0;
	//接收视频，语音
	virtual void StartRecvRemoteVideo(uint64_t ulUserID,uint32_t ulChannelID,IVideoWin*videoWin) = 0;
	virtual void StartRecvRemoteAudio(uint64_t ulUserID) = 0;
	virtual void StopRecvRemoteVideo(uint64_t ulUserID) = 0;
	virtual void StopRecvRemoteAudio(uint64_t ulUserID) = 0;

	virtual void StartRecvRemoteVideo2(uint64_t sessionId,uint32_t ssrc,IVideoWin*videoWin) = 0;
	virtual void StopRecvRemoteVideo2(uint32_t ssrc) = 0;
	//发送视频语音
	virtual void StartPublishVideo(uint64_t ulToUserID,uint32_t ulChannelID,IVideoWin*videoWin) = 0;
	virtual void StartPublishAudio(uint64_t ulToUserID,uint32_t ulChannelID) = 0;
	virtual void StopPublishVideo(uint32_t ulChannelID) = 0;
	virtual void StopPublishAudio(uint32_t ulChannelID) = 0;

	virtual uint32_t StartPublishVideo2(uint64_t ulToUserID,uint32_t ulChannelID,IVideoWin*videoWin) = 0;
	virtual void StopPublishVideo2(uint32_t ulChannelID) = 0;
	//停止所有的音视频流，包括发布的和接收的
	virtual void StopAllMediaStream() = 0;
    //音量调节

	//录音静音
    virtual void SetMicMute(bool bMute, int ulChannelID = 0) = 0;
	virtual void SetMicVolume(int nVolume, int ulChannelID = 0) = 0;
	
	//播放音量
    virtual void SetSpeakerMute(bool bMute) = 0;
	virtual void SetSpeakerVolume(int nVolume) = 0;
	//是否已启动音频
	virtual bool HasStartAudio() = 0;
	virtual bool HasStartVideo() = 0;
	/************************************************************************/
	/* 文本聊天                                                             */
	/************************************************************************/

	virtual void SendTextMsg(uint64_t ulUserID,char* msg,int font_size =14,unsigned long fontFlag = 0,unsigned long color = 0,char *familly = 0) = 0;
	/************************************************************************/
	/* 辅助功能                                                             */
	/************************************************************************/
	virtual char* GetUserName(uint64_t ulUserID) = 0;   //得到用户名，ulUserID = 0 代表本人
	virtual uint64_t GetMySessionID() = 0;
	virtual void GetLocalDateTime(TCHAR* pstrTemp) = 0;
	//得到白板Url
	virtual char* GetAdminPassword() = 0;            //得到主持人密码
	virtual int GetUserRole(uint64_t ulUserID) = 0;               //得到用户在这个房间内的角色
	virtual int GetOrgiUserRoomType(uint64_t ulUserID) = 0;               //得到用户在这个房间内的角色
	virtual int GetUserType(uint64_t ulUserID) = 0;                   //得到用户类型
	/************************************************************************/
	/*   权限相关接口                                                       */
	/************************************************************************/
	virtual bool ApplySpeaker() = 0;   //申请发言
	virtual bool CancelApplySpeaker() = 0;
	virtual void SetAsSpeaker(uint64_t ulUserID,bool bSet) = 0;  //设置为发言人,bSet为真是设置，bSet为false时取消
	virtual void SetAsAdmin(uint64_t ulUserID,bool bSet) = 0;
	virtual void SetAsBigVideo(uint64_t ulUserID) = 0;
	//设置语音发言类型 1 自由发言 2 按f2发言 3 主持人点名发言
	virtual void SetSpeakMode(int mode) = 0;
	//设置房间类型 1 会议模式 2 直播模式
	virtual void SetRoomMode(int mode) = 0;
	virtual int  GetRoomMode() = 0;
	virtual void SetIVideoLayoutMgr(IVideoLayoutMgr* layout_mgr) = 0;
    //发言人是否允许打开视频
	virtual bool SpeakerAllowVideo() = 0;
	//掉线重连接
	virtual void ReConnect() = 0;
	
	//得到在线用户数
	virtual int GetUserCount() = 0;
	//得到用户属性，用户名，角色
	virtual  bool GetUser(int index,PClient_Online_User_Info pUserInfo) = 0;
	//得到房间列表
	virtual void GetRoomList() = 0;

	virtual PROOM_INFO GetRoomInfo(int index) = 0;
	virtual bool GetCurrentRoomInfo(PROOM_INFO pRoomInfo) = 0;
	//得到聊天消息
	virtual PMsgItem GetChatMsgItem(int index) = 0;

	virtual void RequireKeyFrame(uint32_t ssrc) = 0;

	//通知房间内成员打开文档
	virtual void OpenDoc(std::string strUrl,int index,int total) = 0;
	virtual void CloseDoc() = 0;
	//共享光标
	virtual void ShareCursor(float x,float y) = 0;
	//停止共享光标
	virtual void StopShareCursor() = 0;
	virtual char* GetBaseUrl() = 0;

	/*************视频监控相关*****************/
	virtual void StartVideoMonitor() = 0;
	virtual void StopVideoMonitor() = 0;
	//暂停发送视频
	virtual void PauseVideo(uint64_t userId) = 0;
	//恢复发送视频
	virtual void ResumeVideo(uint64_t userId) = 0;
	//发送我的视频编号给主持人，以方便他查看
	virtual void SendVideoSSRCToAdmin(uint64_t userId,uint32_t ssrc) = 0;
	//得到版本号
	virtual int GetVersion() = 0;
};

//底层返给界面层的消息
class IMeetingFrameEvent{
public:
	//打开文档
	virtual void On_MeetingEvent_OpenDoc(std::string strUrl,int index,int total) = 0;

	virtual void On_MeetingEvent_CloseDoc() = 0;

	virtual void On_MeetingEvent_ShareCursor(float x,float y) = 0;

	virtual void On_MeetingEvent_StopShareCursor() = 0;

	virtual void On_MeetingEvent_EnterRoom_Result(uint32_t status,char *msg) = 0;
	//返回用户登录结果
	virtual void On_MeetingEvent_Login_Result(uint32_t status,char *msg) = 0;
	//用户上线
	virtual void On_MeetingEvent_Member_Online(uint64_t ulUserID,char*strUserName,uint32_t userType,uint32_t userRoomType) = 0;      //用户上线
	//用户下线
	virtual void On_MeetingEvent_Member_Offline(uint64_t ulUserID) = 0;     //用户下线
	//收到文本聊天消息
	virtual void On_MeetingEvent_Text_Msg(uint64_t ulUserID,char*strMsg, int font_size =14,unsigned long fontFlat = 0,unsigned long color = 0,char *familly = "") = 0;
	//通知界面层被设置成发言人
	virtual void On_MeetingEvent_SetAsSpeaker(uint64_t ulUserID,bool bSet) = 0;
	//通知界面层被设置成为主讲
	virtual void On_MeetingEvent_SetAsShower(uint64_t ulUserID,bool bSet) = 0;
	//通知界面层被设置成为管理员
	virtual void On_MeetingEvent_SetAsAdmin(uint64_t ulUserID,bool bSet) = 0;
	//通知主讲人或者管理员，有普通用户申请了发言
	virtual void On_MeetingEvent_ApplySpeaker(uint64_t fromUserID) = 0;
	//通知界面层用户取消了发言请求
	virtual void On_MeetingEvent_CancelApplySpeaker(uint64_t fromUserID) = 0;
	//通知界面层，用户角色改变了
	virtual void On_MeetingEvent_UserRole_Change(uint64_t ulUserID,int role) = 0;
	//通知界面接收视频
	virtual void On_MeetingEvent_RecvMyVideo(uint64_t sessionID,uint32_t channelID) = 0;

	virtual void On_MeetingEvent_CloseMyVideo(uint64_t sessionID,uint32_t channelID) = 0;

	//设置成主会场
	virtual void On_MeetingEvent_SetAsBigVideo(uint64_t sessionID) = 0;

	// 网络状态;
	virtual void OnNetEvent(uint32_t code,const char* msg="") = 0;

	virtual void On_MeetingEvent_GetRoomList(uint32_t total) = 0;

	virtual void On_MeetingEvent_UpdateUI() = 0;	//更新界面
	
	//发言模式有了变化
	virtual void On_MeetingEvent_SpeakMode_Change(int mode) = 0;

	virtual void On_MeetingEvent_RoomMode_Change(int mode) = 0;

	/*************视频监控相关*****************/
	virtual void On_MeetingEvent_StartVideoMonitor() = 0;
	virtual void On_MeetingEvent_StopVideoMonitor() = 0;
	//暂停发送视频
	virtual void On_MeetingEvent_PauseVideo() = 0;
	//恢复发送视频
	virtual void On_MeetingEvent_ResumeVideo() = 0;
	
	virtual void On_MeetingEvent_RecvUserVideoSSRC(uint32_t ssrc,uint64_t sessionId,char* userName)=0;

	//更新软件
	virtual void On_MeetingEvent_UpdateSoft(std::string url) = 0;
    
};
//strServer服务地址，格式 192.168.1.1:7082
bool CreateMeetingFrame(IMeetingFrame** pMeetingFrame,IMeetingFrameEvent* pEvent,HWND mainHWND,
	IMediaStreamEvent* pStreamEvent,std::string strServer);