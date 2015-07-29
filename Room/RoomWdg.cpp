#include "RoomWdg.h"
#include <QVBoxLayout>
#include <QAction>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QDebug>
#include "Setting/DlgAVSetting.h"
#include "DlgApplyAdmin.h"
#include "../stdafx.h"
#include "WdgNetInfo.h"
#include <VoiceTestGuide/VoiceTestWdg.h>
#include "../Setting/AboutWidget.h"
#include "../Setting/ModifyPassWord.h"
#include "../VideoMonitor/videomonitormain.h"
#include <QFileDialog>

RoomWdg * RoomWdg::m_instance = NULL;;
RoomWdg * RoomWdg::GetInstance()
{
	if(m_instance == NULL)
	{
		m_instance = new RoomWdg();
	}
	return m_instance;
}


RoomWdg::RoomWdg(QWidget *parent,CommonWndFlags WndFlag)
	: CCommonWidget(parent, WndFlag, Qt::Dialog)
{
	m_bHasStartVideoMonitor = false;
	m_bHasStartPublishVideo = false;
	m_bResumeJK = false;
	m_bStartAudio = false;
	m_pMainMenu = NULL;
	mouse_press = false;
	m_bMicMute = true;
	m_bSpeakerMute = true;
	this->setFocusPolicy(Qt::StrongFocus);
	ui.setupUi(this);
	ui.btnFullScreen->setVisible(true);
	ui.btnShowPPT->setVisible(false);
	m_pVideoMgr = new VideoMgr(ui.videoFrm);
	QVBoxLayout* vBox = new QVBoxLayout();
	vBox->setMargin(0);
	vBox->setSpacing(0);
	ui.leftFrm->setLayout(vBox);
	m_leftWdg = new LeftWdg(ui.leftFrm);
	vBox->addWidget(m_leftWdg);
	//this->setAutoFillBackground(false);

	m_pVideoWin = new VideoWin(this);
	m_pVideoWin->setVisible(false);

	WdgNetInfo::GetInstance()->setParent(this);
	WdgNetInfo::GetInstance()->move(14,this->geometry().height()-105);
	WdgNetInfo::GetInstance()->hide();

	CreateSystemTitle(ui.titleBar);
	CreateSystemButton(ui.sysBtnBar,enum_GGApplication);
	connect(this, SIGNAL(OnClose()), this, SLOT(OnClose()));
	connect(ui.btnTurnLeft,SIGNAL(clicked()),this,SLOT(OnTrunLeft()));
	connect(ui.btnMic,SIGNAL(clicked()),this,SLOT(OnBtnMicClick()));
	//connect(ui.btnNetInfo,SIGNAL(clicked()),this,SLOT(OnBtnNetInfo()));
	connect(ui.btnSpeaker,SIGNAL(clicked()),this,SLOT(OnBtnSpeakerClick()));
	connect(ui.btnShowVideo,SIGNAL(clicked()),this,SLOT(OnShowVideo()));
	connect(ui.btnShowPPT,SIGNAL(clicked()),this,SLOT(OnShowPPT()));
	connect(ui.btnFullScreen,SIGNAL(clicked()),this,SLOT(OnFullScreen()));
	connect(ui.btnCloseVideo,SIGNAL(clicked()),this,SLOT(OnCloseVideo()));
	ui.btnCloseVideo->setVisible(false);

	connect(ui.sliderMic,SIGNAL(sliderReleased()),this,SLOT(OnMicVolumeChange()));
	connect(ui.sliderSpeaker,SIGNAL(sliderReleased()),this,SLOT(OnSpeakerVolumeChange()));

	m_pLabelNetInfo = new LabelNetInfo();
	m_pLabelNetInfo->setPixmap(QPixmap(":/skin/room/wlzl_1.png"));
	ui.bottomToolBar->insertWidget(0,m_pLabelNetInfo);
	connect(m_pLabelNetInfo,SIGNAL(EventEnter()),this,SLOT(OnLabelNetInfoEnter()));
	connect(m_pLabelNetInfo,SIGNAL(EventLeave()),this,SLOT(OnLabelNetInfoLeave()));

	//初始化菜单
	//if(m_pMainMenu == NULL)
	{
		m_pMainMenu= new QMenu();
		menuApplyAdmin = m_pMainMenu->addAction(("申请成为主持人"));
		connect(menuApplyAdmin,SIGNAL(triggered()),this,SLOT(OnApplyAdmin()));
		m_pMainMenu->addSeparator();
		modeGroup = new QActionGroup(this);
		menuModeMeeting = m_pMainMenu->addAction(tr("会议模式"));
		menuModeLive = m_pMainMenu->addAction(tr("直播模式"));
		menuModeMeeting->setCheckable(true);
		menuModeLive->setCheckable(true);
		modeGroup->addAction(menuModeMeeting);
		modeGroup->addAction(menuModeLive);
		if(g_pMeetingFrame && g_pMeetingFrame->GetRoomMode() == 1)
		{
			menuModeMeeting->setChecked(true);
			menuModeLive->setChecked(false);
		}
		else
		{
			menuModeMeeting->setChecked(false);
			menuModeLive->setChecked(true);
		}
		connect(menuModeMeeting,SIGNAL(triggered()),this,SLOT(OnModeMeeting()));
		connect(menuModeLive,SIGNAL(triggered()),this,SLOT(OnModeLive()));
		
		m_pMainMenu->addSeparator();
		menuAVSetting = m_pMainMenu->addAction(tr("音视频参数设置"));
		connect(menuAVSetting,SIGNAL(triggered()),this,SLOT(OnStartAVSetting()));
		menuVoiceTestGuide = m_pMainMenu->addAction(tr("语音测试向导"));
		connect(menuVoiceTestGuide,SIGNAL(triggered()),this,SLOT(OnVoiceTestGuide()));
		menuVideoMonitor = m_pMainMenu->addAction(tr("会场巡视"));
		connect(menuVideoMonitor,SIGNAL(triggered()),this,SLOT(OnVideoMonitorClick()));
		m_pMainMenu->addSeparator();
		modifyPassWord = m_pMainMenu->addAction(tr("修改登录密码"));
		connect(modifyPassWord,SIGNAL(triggered()),this,SLOT(OnModifyPassWord()));
		aboutWidget = m_pMainMenu->addAction(tr("关于"));
		connect(aboutWidget,SIGNAL(triggered()),this,SLOT(OnAbout()));
		quitAction = m_pMainMenu->addAction(tr("退出"));
		connect(quitAction,SIGNAL(triggered()),this,SLOT(OnClose()));
		if(g_pMeetingFrame && g_pMeetingFrame->GetUserRole(0)!=1)
		{
			menuModeMeeting->setDisabled(true);
			menuModeLive->setDisabled(true);
			menuVideoMonitor->setDisabled(true);
		}
	}

	m_pWdgDocShare = new WdgDocShare(ui.videoFrm);
	m_pWdgDocShare->hide();
	m_pVideoMgr->GetLayout()->addWidget(m_pWdgDocShare,0,0,3,3);
 	connect(m_pWdgDocShare->m_toolBar->ui.powerBtn,SIGNAL(clicked()),this,SLOT(OnBtnQuitShareDoc()));
 	connect(m_pWdgDocShare->m_toolBar->ui.penBtn,SIGNAL(clicked()),this,SLOT(OnBtnShareCursor()));
	connect(m_pWdgDocShare->m_pProgressBar->ui.closeBtn
		,SIGNAL(clicked()),this,SLOT(OnCloseDoc()));

	m_pBtnQuitFullScreen = new QPushButton();
	m_pBtnQuitFullScreen->setText(tr(""));
	m_pBtnQuitFullScreen->setObjectName("btnQuitFullScreen");
	m_pBtnQuitFullScreen->setFixedSize(48,48);
	m_pBtnQuitFullScreen->setHidden(true);
	connect(m_pBtnQuitFullScreen,SIGNAL(clicked()),this,SLOT(OnQuitFullScreen()));
}

RoomWdg::~RoomWdg()
{

}
//启动视频监控界面
void RoomWdg::OnVideoMonitorClick()
{
	VideoMonitorMain::GetInstance()->show();

}

void RoomWdg::OnShowRemoteCursor(float x,float y)
{
	if(m_pWdgDocShare)
		m_pWdgDocShare->OnShowRemoteCursor(x,y);
}

void RoomWdg::OnCloseRemoteCursor()
{
	if(m_pWdgDocShare)
		m_pWdgDocShare->OnCloseRemoteCursor();
}


void RoomWdg::OnBtnShareCursor()
{
	//qDebug()<<"OnBtnShareCursor";
	m_pWdgDocShare->ShareCursor();
}

void RoomWdg::OnBtnQuitShareDoc()
{
	qDebug()<<"OnBtnQuitShareDoc";
	m_pVideoMgr->ShowAll();
	m_pWdgDocShare->CloseShareDoc();
	if(g_pMeetingFrame)
	{
		g_pMeetingFrame->CloseDoc();
	}
}

//打开远程服务器上的文档
void RoomWdg::OnOpenDoc(QString url,int index,int total)
{
	//qDebug()<<"open url2:"<<url.data()<<"index:"<<index<<" total:"<<total;
	if(g_pMeetingFrame->HasStartVideo()){
		OnCloseVideo();
	}

	if(m_pWdgDocShare->isVisible()==false)
	{
		m_pVideoMgr->HideAll();
		m_pWdgDocShare->show();
	}
	QString newUrl =QString("%1/UploadDoc/%2/%3.JPG").arg(g_pMeetingFrame->GetBaseUrl()).arg(url).arg(index);
	//http://211.151.17.177/meeting//UploadDoc/c878feaf43247aff65a3b14b896c4a65/2.JPG
	m_pWdgDocShare->DisplayRemoteDoc(newUrl,url,index,total);

}

//通知房间成员关闭文档共享功能

void RoomWdg::OnCloseDoc()
{
	m_pVideoMgr->ShowAll();
	m_pWdgDocShare->CloseShareDoc();

}

//共享PPT,弹出窗口，选择文件，转换文件，上传文件，通知房间成员打开服务器上的JPG图片
void RoomWdg::OnShowPPT()
{
	//打开选择文件对话框
	
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("打开文档"), "", tr("文档 (*.ppt *.pptx)"));

	if(fileName.length()>0)
	{
		
		m_pVideoMgr->HideAll();
		m_pWdgDocShare->show();
		QApplication::processEvents();

		m_pWdgDocShare->ShowPPT(fileName);
	}
	
	/*
	m_pVideoMgr->HideAll();
	m_pWdgDocShare->show();
	QApplication::processEvents();
	m_pWdgDocShare->DisplayLocalDoc("857b549cdb88290f079e2858f7b4226d",61);
	*/
}



void RoomWdg::OnNetInfo(int delay,int uploadLost,int downloadLost)
{
	int total = uploadLost+downloadLost;
	switch(total)
	{
	case 0:
		{
			if(delay>200)
				m_pLabelNetInfo->setPixmap(QPixmap(":/skin/room/wlzl_2.png"));
			else
				m_pLabelNetInfo->setPixmap(QPixmap(":/skin/room/wlzl_1.png"));
		}
		
		break;
	case 1:
	case 2:
		m_pLabelNetInfo->setPixmap(QPixmap(":/skin/room/wlzl_2.png"));
		break;
	case 3:
	case 4:
	case 5:
		m_pLabelNetInfo->setPixmap(QPixmap(":/skin/room/wlzl_3.png"));
		break;
	case 6:
	case 7:
	case 8:
		m_pLabelNetInfo->setPixmap(QPixmap(":/skin/room/wlzl_4.png"));
		break;
	default:
		m_pLabelNetInfo->setPixmap(QPixmap(":/skin/room/wlzl_5.png"));
	}
	
}

void RoomWdg::OnStartVideoMonitor()
{
	if(g_pMeetingFrame!=NULL&&!m_bHasStartPublishVideo){
		m_bResumeJK = false;
		m_bHasStartVideoMonitor = true;
		uint32_t ssrc = g_pMeetingFrame->StartPublishVideo2(0,AppSetting::GetInstance()->GetCameraIndex(),m_pVideoWin);
		m_pVideoWin->SetFree(false);
		if(ssrc>0)
			g_pMeetingFrame->SendVideoSSRCToAdmin(0,ssrc);
	}
}

void RoomWdg::OnStopVideoMonitor()
{
	if(g_pMeetingFrame!=NULL&&m_bHasStartVideoMonitor){
		m_bHasStartVideoMonitor = false;
		g_pMeetingFrame->StopPublishVideo2(0);
		m_pVideoWin->SetFree(true);
	}
}

//如果已打开监控视频，关闭监控视频
void RoomWdg::OnShowVideo()
{
	if(m_bHasStartVideoMonitor){
		m_bResumeJK = true;
		OnStopVideoMonitor();
	}

	ui.btnCloseVideo->setVisible(true);
	ui.btnShowVideo->setVisible(false);
	//打开本地视频
	VIDEO_FORMAT vf;
	//HWND videoWnd;

	vf.videoCodec =H264;
	vf.videoSize = VIDEO_SIZE_320X240;
	vf.videoFrameRate = 15;
	vf.videoQuality = 312;
	IVideoWin *videoWnd = (m_pVideoMgr->GetFreeVideo());
	if(videoWnd == NULL)
	{
		MessageBox(NULL,L"视频位置已占满",NULL,NULL);
		return;
	}
	if(g_pMeetingFrame!=NULL)
	{
		g_pMeetingFrame->StartPublishVideo(0,AppSetting::GetInstance()->GetCameraIndex(),videoWnd);
		m_bHasStartPublishVideo = true;
	}
	
}


void RoomWdg::OnCloseVideo()
{
	if(m_bHasStartPublishVideo == false)
		return ;
	ui.btnCloseVideo->setVisible(false);
	ui.btnShowVideo->setVisible(true);
	if(g_pMeetingFrame)
	{
		VideoMgr::GetInstance()->GivebackVideo(0); //回收视频窗口
		g_pMeetingFrame->StopPublishVideo(0);
		m_bHasStartPublishVideo = false;
	}
	this->setFocus();
	if(m_bResumeJK == true)
	{
		OnStartVideoMonitor();
	}
	
}

void RoomWdg::OnTrunLeft()
{
	if(ui.leftFrm->isHidden())
		ui.leftFrm->show();
	else
		ui.leftFrm->hide();

}

void RoomWdg::OnLabelNetInfoEnter()
{
	
	WdgNetInfo::GetInstance()->setParent(this);
	WdgNetInfo::GetInstance()->move(14,this->geometry().height()-105);
	WdgNetInfo::GetInstance()->show();
	
}

void RoomWdg::OnLabelNetInfoLeave()
{
	WdgNetInfo::GetInstance()->hide();
}

void RoomWdg::OnBtnMicClick()
{
	
	if(g_pMeetingFrame)
	{
		g_pMeetingFrame->SetMicMute(m_bMicMute);
		m_bMicMute=!m_bMicMute;
	}
}

void RoomWdg::OnBtnSpeakerClick()
{  
	if(g_pMeetingFrame)
	{
		g_pMeetingFrame->SetSpeakerMute(m_bSpeakerMute);
		m_bSpeakerMute=!m_bSpeakerMute;	
	}
	
	
}
void RoomWdg::OnClose()
{
	hide();
	if(g_pMeetingFrame)
	{
		g_pMeetingFrame->LeaveRoom();
		g_pMeetingFrame->Logout();
		g_pMeetingFrame->Release();
	}
	qApp->quit();
}

void RoomWdg::OnRoomModeChange(int mode)
{
	if(g_pMeetingFrame->HasStartVideo())
	{
		g_pMeetingFrame->StopPublishVideo(0);
		VideoMgr::GetInstance()->GivebackVideo(0); //回收视频窗口
	}

	if(mode == 1 || g_pMeetingFrame->GetUserRole(0)==1)
	{
		ui.btnShowVideo->setVisible(true);
		ui.btnCloseVideo->setVisible(false);
	}
	else
	{
		ui.btnShowVideo->setVisible(false);
		ui.btnCloseVideo->setVisible(false);
	}
}

//底层数据有变化，通知界面更新
void RoomWdg::UpdateUI()
{
	if(g_pMeetingFrame!=NULL)
	{
		ROOM_INFO roomInfo;
		if(g_pMeetingFrame->GetCurrentRoomInfo(&roomInfo)&&strlen(roomInfo.strRoomName)>0)
		{
			ui.lblRoomName->setText(QString("[")+QString::fromUtf8(roomInfo.strRoomName)+
				QString("]"));
		}
		int mode = g_pMeetingFrame->GetRoomMode();
		if(mode == 1 || g_pMeetingFrame->GetUserRole(0)==1)
		{
			if(g_pMeetingFrame->HasStartVideo())
			{
				ui.btnShowVideo->setVisible(false);
				ui.btnCloseVideo->setVisible(true);
			}
			else
			{
				ui.btnShowVideo->setVisible(true);
				ui.btnCloseVideo->setVisible(false);
			}
			
		}
		else
		{
			ui.btnShowVideo->setVisible(false);
			ui.btnCloseVideo->setVisible(false);
		}

		if(g_pMeetingFrame->GetUserRole(0) == 1)
		{
			m_pWdgDocShare->ShowCloseDocbtn(true);
			ui.btnShowPPT->setVisible(true);
		}
		else
		{
			ui.btnShowPPT->setVisible(false);
			m_pWdgDocShare->ShowCloseDocbtn(false);
		}
	}
	//this->update();
}

void RoomWdg::OnSysMenuClick()
{
	if(g_pMeetingFrame &&g_pMeetingFrame->GetUserRole(0)==2)
	{
		menuApplyAdmin->setText(("申请成为主持人"));
	}
	else
	{
		menuApplyAdmin->setText(("取消成为主持人"));
	}

	if(g_pMeetingFrame && g_pMeetingFrame->GetRoomMode() == 1)
	{
		menuModeMeeting->setChecked(true);
		menuModeLive->setChecked(false);
	}
	else
	{
		menuModeMeeting->setChecked(false);
		menuModeLive->setChecked(true);
	}

	if(g_pMeetingFrame && g_pMeetingFrame->GetUserRole(0)==1)
	{
		menuModeMeeting->setDisabled(false);
		menuModeLive->setDisabled(false);
		menuVideoMonitor->setDisabled(false);
	}
	else
	{
		menuModeMeeting->setDisabled(true);
		menuModeLive->setDisabled(true);
		menuVideoMonitor->setDisabled(true);
	}
	QPoint curPos = QCursor::pos();
	curPos-=QPoint(60,-10);
	m_pMainMenu->exec(curPos);
}
 //会议模式
void RoomWdg::OnModeMeeting()
{
	if(g_pMeetingFrame->GetRoomMode()==1)
		return;
	if(g_pMeetingFrame)
	{
		g_pMeetingFrame->SetRoomMode(1);
		OnRoomModeChange(1);
		VideoMgr::GetInstance()->OnRoomModeChange(1);
	}
}
 //直播模式
void RoomWdg::OnModeLive()
{
	if(g_pMeetingFrame->GetRoomMode()==2)
		return;

	if(g_pMeetingFrame)
	{
		g_pMeetingFrame->SetRoomMode(2);
		OnRoomModeChange(2);
		VideoMgr::GetInstance()->OnRoomModeChange(2);
	}
}



void RoomWdg::OnApplyAdmin()
{
	if(g_pMeetingFrame)
	{
		if(g_pMeetingFrame->GetUserRole(0) == 2)
		{
			//DlgApplyAdmin::GetInstance()->show();
			DlgApplyAdmin *applyAdmin = DlgApplyAdmin::GetInstance();
			applyAdmin->SetTitle(L"申请成为主讲人");
			applyAdmin->lblPwd->setText("请输入密码");
			applyAdmin->show();
		}
		else
			g_pMeetingFrame->SetAsAdmin(0,false);
	}
}

void RoomWdg::OnStartAVSetting()
{
	//::GetInstance()->ShowAndInit();
	DlgAVSetting::GetInstance()->ShowAndInit();
}
//add by xuluorong
void RoomWdg::OnVoiceTestGuide()
{
	VoiceTestWdg::GetVoiceInstance()->ShowAndInit();
}
void RoomWdg::OnModifyPassWord(){

	

	ModifyPassWord *modiPassword = new ModifyPassWord(this);// ModifyPassWord::GetInstance();
	modiPassword->SetTitle(L"修改密码");
	modiPassword->setAttribute(Qt::WA_ShowModal,true);
	modiPassword->show();

// 	DlgApplyAdmin *applyAdmin = DlgApplyAdmin::GetInstance();
// 	applyAdmin->SetTitle(L"修改密码");
// 	applyAdmin->lblPwd->setText("请输入新密码");
// 	applyAdmin->show();

}
void RoomWdg::OnAbout(){

	AboutWidget::GetInstance()->setAttribute(Qt::WA_ShowModal,true);
	AboutWidget::GetInstance()->show();
}
#if 1


void RoomWdg::mousePressEvent(QMouseEvent *event)
{
	//只能是鼠标左键移动和改变大小
	int y = event->pos().y();
	int y2 = event->globalY();
	int y3= event->globalPos().y();
	if((event->button() == Qt::LeftButton) && (event->pos().y()<88)&&!this->isMaximized())
	{
		mouse_press = true;
	}

	//窗口移动距离
	move_point = event->globalPos() - pos(); 
}

void RoomWdg::mouseReleaseEvent(QMouseEvent *event)
{
	//只能是鼠标左键移动和改变大小
	if(event->button() == Qt::LeftButton) 
	{
		mouse_press = false;
	}

	//窗口移动距离
	move_point = event->globalPos() - pos(); 
}

void RoomWdg::mouseMoveEvent(QMouseEvent *event)
{
	//移动窗口
	if(mouse_press)   
	{
		QPoint move_pos = event->globalPos();
		move(move_pos - move_point);
	}
	//qDebug()<<"mouse move";
}
#endif

#if 1
void RoomWdg::keyPressEvent(QKeyEvent *e)
{
	 int key = e->key();
	 ROOM_INFO roomInfo;
	 if(g_pMeetingFrame->GetCurrentRoomInfo(&roomInfo)&&roomInfo.speakMode == 2)
	 {
		if(Qt::Key_F2 == key)
		 {
			  //qDebug()<<"key press:"<<e->isAutoRepeat();
			 if(m_bStartAudio == false&&e->isAutoRepeat() == false)
			 {
				
				 g_pMeetingFrame->StartPublishAudio(0,AppSetting::GetInstance()->GetMicIndex()-1);
				 m_bStartAudio = true;
			 }
		 }
	 }

	 
}

void RoomWdg::OnQuitFullScreen(){
	
	ui.videoFrm->setWindowFlags(Qt::SubWindow);
	ui.centerFrm->layout()->addWidget(ui.videoFrm);
	//ui.videoFrm->resize(800,600);
	m_pBtnQuitFullScreen->setHidden(true);
}

void RoomWdg::OnFullScreen(){
		
		m_pBtnQuitFullScreen->setParent(ui.videoFrm);
		m_pBtnQuitFullScreen->setHidden(false);
		m_pBtnQuitFullScreen->move(1280-74,800-74);
		m_pBtnQuitFullScreen->setWindowOpacity(0.3);
		ui.videoFrm->setWindowFlags(Qt::Dialog);
		ui.videoFrm->showFullScreen();
}

void RoomWdg::keyReleaseEvent(QKeyEvent *e)
{
	int key = e->key();
	if(Qt::Key_F2 == key)
	{
		//qDebug()<<"key Release:"<<e->isAutoRepeat();
		if(m_bStartAudio == true && e->isAutoRepeat() == false)
		{
			g_pMeetingFrame->StopPublishAudio(0);
			m_bStartAudio = false;
		}
	}
}
#endif

void RoomWdg::mouseDoubleClickEvent( QMouseEvent * event )
{
	
	if(event->button() == Qt::LeftButton && (event->pos().y()<88)) 
	{
		if(isMaximized())
		{
			showNormal();
		}
		else if(isMinimized())
		{
			showNormal();
		}
		else
		{
			showMaximized();
		}
	}
}


bool RoomWdg::winEvent (MSG* msg, long *result)
{
	switch(msg->message)
	{
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *)msg->lParam;
			const QRect rect = QApplication::desktop()->availableGeometry();
			RECT clientRect,wndRect;
			POINT point = {0,0};
			::GetClientRect(winId(),&clientRect);
			::ClientToScreen(winId(),&point);
			::OffsetRect(&clientRect,point.x,point.y);
			::GetWindowRect(winId(),&wndRect);
			if(clientRect.right - clientRect.left > 0)
			{
				int leftOffset = clientRect.left - wndRect.left;
				int topOffset = clientRect.top - wndRect.top;
				int rightOffset = wndRect.right - clientRect.right;
				int bottomOffset = wndRect.bottom - clientRect.bottom;
				m_oMaxMargins.setLeft(leftOffset);
				m_oMaxMargins.setTop(topOffset);
				m_oMaxMargins.setRight(rightOffset);
				m_oMaxMargins.setBottom(bottomOffset);
				mmi->ptMaxPosition.x = rect.left() - leftOffset;
				mmi->ptMaxPosition.y = rect.top()  - topOffset;
				mmi->ptMaxSize.x = rect.width() + leftOffset + rightOffset;
				mmi->ptMaxSize.y = rect.height() + topOffset + bottomOffset;
				mmi->ptMaxTrackSize.x = mmi->ptMaxSize.x;
				mmi->ptMaxTrackSize.y = mmi->ptMaxSize.y;
				qDebug()<<mmi->ptMaxSize.y<<":"<<mmi->ptMaxPosition.y<<":"<<mmi->ptMaxTrackSize.x<<":"<<mmi->ptMaxPosition.x;
			}
			else
			{
				mmi->ptMaxPosition.x = rect.left() - m_oMaxMargins.left();
				mmi->ptMaxPosition.y = rect.top()  - m_oMaxMargins.top();
				mmi->ptMaxSize.x = rect.width() + m_oMaxMargins.left() + m_oMaxMargins.right();
				mmi->ptMaxSize.y = rect.height() + m_oMaxMargins.top() + m_oMaxMargins.bottom();
				mmi->ptMaxTrackSize.x = mmi->ptMaxSize.x;
				mmi->ptMaxTrackSize.y = mmi->ptMaxSize.y;
			}
			
			//return true;
		}
	}
	return false;
}

void RoomWdg::OnMicVolumeChange()
{
	if(g_pMeetingFrame)
	{
		g_pMeetingFrame->SetMicVolume(ui.sliderMic->value());
	}
}
void RoomWdg::OnSpeakerVolumeChange()
{
	if(g_pMeetingFrame)
	{
		g_pMeetingFrame->SetSpeakerVolume(ui.sliderSpeaker->value());
	}
}

