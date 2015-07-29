#ifndef ROOMWDG_H
#define ROOMWDG_H

#include <QWidget>
#include <QMenu>
#include <QCursor>
#include <QSlider>
#include "ui_RoomWdg.h"
#include "ccommonwidget.h"
#include "LeftWdg.h"
#include "LabelNetInfo.h"
#include "../Video/VideoMgr.h"
#define TCHAR wchar_t
#include <typedef.h>
#include "../Video/VideoWin.h"
#include "../DocShare/WdgDocShare.h"
class RoomWdg : public CCommonWidget,public IZYVideoCaptureEvent
{
	Q_OBJECT

public:
	RoomWdg(QWidget *parent = 0,CommonWndFlags WndFlag = enum_GGWndTitlebarHint | enum_GGWndCloseButtonHint|enum_GGWndResizeHint);
	~RoomWdg();
public:
	static RoomWdg * m_instance;
	static RoomWdg * GetInstance();
	void UpdateUI();
public:
	virtual void OnZYAVCaptureEvent_VideoData(char* pData, int nLen, bool bKeyFrame, VIDEO_SIZE videoSize, VIDEO_CODEC videoCodec){

	};

	void OnRoomModeChange(int mode);

	public slots:
		void OnOpenDoc(QString url,int index,int total);
		void OnCloseDoc();
		public slots:
			void OnClose();
			virtual void OnSysMenuClick();
			void OnTrunLeft();
			void OnStartAVSetting();
			void OnBtnMicClick();
			void OnBtnSpeakerClick();
			void OnShowVideo();
			void OnCloseVideo();
			//add by xuluorong
			void OnVoiceTestGuide();
			void OnApplyAdmin();
			void OnMicVolumeChange();
			void OnSpeakerVolumeChange();
			void OnModeMeeting();    //会议模式
			void OnModeLive();       //直播模式
			void OnLabelNetInfoEnter();
			void OnLabelNetInfoLeave();
			void OnNetInfo(int delay,int uploadLost,int downloadLost);
			void OnModifyPassWord();
			void OnAbout();
			//共享PPT
			void OnShowPPT();

			void OnBtnShareCursor();
			void OnBtnQuitShareDoc();

			void OnShowRemoteCursor(float x,float y);
			void OnCloseRemoteCursor();
			//视频监控
			void OnVideoMonitorClick();

			void OnQuitFullScreen();
			
			void OnFullScreen();
public:
			void OnStopVideoMonitor();
			void OnStartVideoMonitor();
			
protected:

	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

	virtual void mouseDoubleClickEvent( QMouseEvent * event );

	virtual void keyPressEvent(QKeyEvent *e);
	virtual void keyReleaseEvent(QKeyEvent *e);

	virtual bool winEvent (MSG* msg, long *result);
private:
	Ui::RoomWdg ui;
	LeftWdg * m_leftWdg;
	VideoMgr * m_pVideoMgr;
	QMenu *m_pMainMenu; //主菜单
	QAction* menuApplyAdmin;
	QActionGroup* modeGroup;
	QAction* menuModeMeeting;
	QAction* menuModeLive;
	QAction* menuAVSetting;
	QAction* menuVideoMonitor;//视频监控
	QAction* menuVoiceTestGuide;
	QAction*modifyPassWord;
	QAction *aboutWidget;
	QAction* quitAction;

	IZYVideoCapture *videoCapture;
	bool createVideoCapture;

	QPoint move_point; //移动的距离
	bool mouse_press; //按下鼠标左键
	bool m_bStartAudio;
	bool m_bMicMute;
	bool m_bSpeakerMute;
	//int  m_roomMode;       //房间类型 1，会议 2，直播
	LabelNetInfo* m_pLabelNetInfo;

	//共享文档窗口
	WdgDocShare*  m_pWdgDocShare;

	VideoWin *m_pVideoWin;
	//是否已启动视频监控
	bool m_bHasStartVideoMonitor;
	bool m_bHasStartPublishVideo;
	//恢复监控
	bool m_bResumeJK;
	
	QPushButton * m_pBtnQuitFullScreen;  
};

#endif // ROOMWDG_H
