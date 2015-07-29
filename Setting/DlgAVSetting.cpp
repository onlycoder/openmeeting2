#include "DlgAVSetting.h"
#include "AppSetting.h"
#include "../stdafx.h"
#include <QFile>
#include <QPalette>
#include <QMouseEvent>
#include "../Video/VideoMgr.h"
DlgAVSetting*DlgAVSetting::m_instance = NULL;
DlgAVSetting*DlgAVSetting::GetInstance()
{
	if(m_instance == NULL)
	{
		m_instance = new DlgAVSetting();
	}
	return m_instance;
}

DlgAVSetting::DlgAVSetting(QWidget *parent,CommonWndFlags WndFlag)
	: CCommonWidget(parent, WndFlag, Qt::Dialog)
{
	m_pIZYVideoCapture = NULL;
	m_pIZYAudioCapture = NULL;
	m_pIZYAudioPlayer = NULL;
	m_micCount = 0;
	m_speakerCount = 0;
	m_cameraCount = 0;
	m_bResumeVideo = false;
	m_bResumeAudio = false;
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	QFile qssFile(":/qss/setting.qss");
	QString qss;
	qssFile.open(QFile::ReadOnly);
	if(qssFile.isOpen())
	{  
		qss = QString(qssFile.readAll());
		this->setStyleSheet(qss);
		qssFile.close();
	}

	ui.micImageLabel->setPixmap(QPixmap(":/setting/setting/SeanVideoMic.png"));
	ui.soundImageLabel->setPixmap(QPixmap(":/setting/setting/SeanVideoSpk.png"));

	CreateSystemTitle(ui.hBoxTitle);
	CreateSystemButton(ui.hBoxSysBtn,enum_GGWndCloseButtonHint);


	SetTitle(L"音视频设置");
	//this->setModal(true);
	ui.tabAVSetting->setCurrentIndex(0);
	connect(ui.btnOK,SIGNAL(clicked()),this,SLOT(OnOK()));
	connect(ui.btnCancel,SIGNAL(clicked()),this,SLOT(OnCancel()));
	connect(this, SIGNAL(OnClose()), this, SLOT(OnClose()));

	connect(ui.sliderMic,SIGNAL(sliderReleased()),this,SLOT(OnMicVolumeChange()));
	connect(ui.sliderSpeaker,SIGNAL(sliderReleased()),this,SLOT(OnSpeakerVolumeChange()));
	m_pVideoWin = new VideoWin(ui.lblVideo);
	m_pVideoWin->setGeometry(0,0,231,211);
	m_pVideoWin->SetHidden(true);
}


DlgAVSetting::~DlgAVSetting()
{
	
}


void DlgAVSetting::OnOK()
{
	AppSetting::GetInstance()->Save();
	OnClose();
}

void DlgAVSetting::OnCancel()
{
	OnClose();
}

void DlgAVSetting::InitAudioDevList()
{
	//载入音频设备
	char soundChar[256]={0};
	ui.cmbMicList->addItem(("Windows默认设备"));
	m_micCount = GetSoundCaptureDeviceCount();
	for(int i = 0;i <m_micCount;i ++){
		GetSoundCaptureDeviceName(i,soundChar);
		QString str = QString::fromWCharArray((wchar_t*)soundChar);
		ui.cmbMicList->addItem(str);
	}
	ui.cmbMicList->addItem(("禁用"));
	int index = AppSetting::GetInstance()->GetMicIndex();
	if(index == -1)
	{
		ui.cmbMicList->setCurrentIndex(m_micCount+1);
	}
	else 
	{
		if(index>m_micCount)
		{
			ui.cmbMicList->setCurrentIndex(0);
			AppSetting::GetInstance()->SetMicIndex(0);
		}
		else
			ui.cmbMicList->setCurrentIndex(index);
	}
	
	ui.cmbSpeakerList->addItem(("Windows默认设备"));
	char soundOutChar[256]={0};
	m_speakerCount = GetSoudPlayDeviceCount();
	for(int i = 0;i < m_speakerCount;i ++){
		GetSoundPlayDeviceName(i,soundOutChar);
		QString outStr = QString::fromWCharArray((wchar_t*)soundOutChar);
		ui.cmbSpeakerList->addItem(outStr);
	}
	ui.cmbSpeakerList->addItem(("禁用"));
	index = AppSetting::GetInstance()->GetSpeakerIndex();
	if(index == -1)
	{
		ui.cmbSpeakerList->setCurrentIndex(m_speakerCount+1);
	}
	else 
	{
		if(index>m_speakerCount)
		{
			ui.cmbSpeakerList->setCurrentIndex(0);
			AppSetting::GetInstance()->SetSpeakerIndex(0);
		}
		else
			ui.cmbSpeakerList->setCurrentIndex(index);
	}
	
	connect(ui.cmbSpeakerList,SIGNAL(currentIndexChanged(int)),this,
		SLOT(OnSpeakerChange(int)),Qt::QueuedConnection);
	connect(ui.cmbMicList,SIGNAL(currentIndexChanged(int)),this,SLOT(OnMicChange(int)),Qt::QueuedConnection);
	
}
void DlgAVSetting::InitVideoDevList()
{
	
	char videoDeviceStr[256]={0};
	m_cameraCount =  GetVideoCaptureDeviceCount();
	for(int i = 0;i < m_cameraCount;i++){
		GetVideoCaptureDeviceName(i,videoDeviceStr);
		QString videoStr = QString::fromWCharArray((wchar_t*)videoDeviceStr);
		ui.cmbCamList->addItem(videoStr);
	}
	ui.cmbCamList->addItem(("禁用"));
	int index = AppSetting::GetInstance()->GetCameraIndex();
	if(index == -1)
	{
		ui.cmbCamList->setCurrentIndex(m_cameraCount);
	}
	else 
	{
		if(index>m_speakerCount-1)
		{
			ui.cmbCamList->setCurrentIndex(0);
			AppSetting::GetInstance()->SetCameraIndex(0);
		}
		else
			ui.cmbCamList->setCurrentIndex(index);
	}
	connect(ui.cmbCamList,SIGNAL(currentIndexChanged(int)),this,SLOT(OnCameraChange(int)));
}
bool DlgAVSetting::StartAudio()
{
	if(g_pMeetingFrame->HasStartAudio() == true)
	{
		g_pMeetingFrame->StopPublishAudio(0);
	}

	HWND hWnd = (HWND)(this->winId());
	bool ret = CreateZYAudioCapture(&m_pIZYAudioCapture,this);
	AUDIO_FORMAT af;
	af.audioCodec = PCM_16;
	if(ret)
	{
		ret = CreateZYAudioPlayer(&m_pIZYAudioPlayer,this);
		if(ret)
		{
			ret = m_pIZYAudioPlayer->Connect(false,hWnd,&af,AppSetting::GetInstance()->GetSpeakerIndex()-1);
			if(ret)
			{
				ret = m_pIZYAudioCapture->Connect(hWnd,AppSetting::GetInstance()->GetMicIndex()-1,&af);
				return ret;
			}

		}
	}
	return false;
}

void DlgAVSetting::StopAudio()
{
	if(m_pIZYAudioCapture)
	{
		m_pIZYAudioCapture->ReleaseConnections();
		m_pIZYAudioCapture = NULL;
	}
	if(m_pIZYAudioPlayer)
	{
		m_pIZYAudioPlayer->ReleaseConnections();
		m_pIZYAudioPlayer = NULL;
	}
}

bool DlgAVSetting::StartVideo()
{
	m_bResumeVideo = g_pMeetingFrame->HasStartVideo();
	if(g_pMeetingFrame->HasStartVideo())
	{
		VideoMgr::GetInstance()->GivebackVideo(0);
		g_pMeetingFrame->StopPublishVideo(0);
	}

	
	bool ret =::CreateZYVideoCapture(&m_pIZYVideoCapture,this);
	if(m_pVideoWin!=NULL&&ret&&m_pIZYVideoCapture)
	{
		VIDEO_FORMAT vf;
		vf.videoCodec =H264;
		vf.videoSize = VIDEO_SIZE_320X240;
		vf.videoFrameRate = 15;
		vf.videoQuality = 312;
		m_pVideoWin->SetHidden(false);
		if(m_pIZYVideoCapture->Connect(m_pVideoWin,AppSetting::GetInstance()->GetCameraIndex(),
			&vf,ZYCOLORSPACE_RGB24))
		{
			m_pVideoWin->SetUserID(0);
			return true;
		}
		
	}
	return false;
}

void DlgAVSetting::StopVideo()
{
	if(m_pIZYVideoCapture!=NULL)
	{
		m_pIZYVideoCapture->ReleaseConnections();
		m_pIZYVideoCapture = NULL;
		m_pVideoWin->SetHidden(true);
	}
}

void DlgAVSetting::OnZYAVCaptureEvent_AudioData(char* pData, int nLen, AUDIO_CODEC audioCodec)
{
	if(m_pIZYAudioPlayer!=NULL)
	{
		m_pIZYAudioPlayer->DeliverAudioData(pData,nLen,0);
	}
}

void DlgAVSetting::OnMicChange(int index)
{
	if(index == m_micCount+1)
		AppSetting::GetInstance()->SetMicIndex(-1);
	else
		AppSetting::GetInstance()->SetMicIndex(index);
	emit EventRestartAudio();
}

void DlgAVSetting::OnSpeakerChange(int index)
{
	if(index == m_speakerCount+1)
		AppSetting::GetInstance()->SetSpeakerIndex(-1);  //禁用
	else
		AppSetting::GetInstance()->SetSpeakerIndex(index);
	emit EventRestartAudio();
}

void DlgAVSetting::OnRestartAudio()
{
	StopAudio();
	StartAudio();
}

void DlgAVSetting::OnCameraChange(int index)
{
	if(index == m_cameraCount)
		AppSetting::GetInstance()->SetCameraIndex(-1);
	else
		AppSetting::GetInstance()->SetCameraIndex(index);
	StopVideo();
	StartVideo();
}

void DlgAVSetting::ShowAndInit()
{
	show();
	m_bResumeAudio = g_pMeetingFrame->HasStartAudio();
	
	connect(ui.tabAVSetting,SIGNAL(currentChanged(int)),this,SLOT(OnTabChange(int)));
	if(ui.tabAVSetting->currentIndex() == 0)
	{
		connect(this,SIGNAL(EventRestartAudio()),this,SLOT(OnRestartAudio()),Qt::QueuedConnection);
		InitAudioDevList();
		emit EventRestartAudio();
	}
	else
	{
		InitVideoDevList();
		StartVideo();
	}
}

void DlgAVSetting::OnTabChange(int index)
{
	if(index == 0)
	{
		ui.cmbCamList->disconnect();
		ui.cmbCamList->clear();
		InitAudioDevList();
		StopVideo();
		StartAudio();
	}
	else
	{
		
		ui.cmbMicList->disconnect();
		ui.cmbMicList->clear();

		
		ui.cmbSpeakerList->disconnect();
		ui.cmbSpeakerList->clear();

		InitVideoDevList();
		StopAudio();
		StartVideo();
	}
}

void DlgAVSetting::OnClose()
{
	ui.cmbSpeakerList->disconnect();
	ui.cmbMicList->disconnect();
	ui.cmbCamList->disconnect();
	ui.tabAVSetting->disconnect();
	ui.tabAVSetting->setCurrentIndex(0);
	ui.cmbMicList->clear();
	ui.cmbSpeakerList->clear();
	ui.cmbCamList->clear();
	StopAudio();
	StopVideo();
	close();
	if(m_bResumeAudio)
	{
		g_pMeetingFrame->StartPublishAudio(0,AppSetting::GetInstance()->GetMicIndex()-1);
	}

	if(m_bResumeVideo)
	{
		IVideoWin *videoWnd = (VideoMgr::GetInstance()->GetFreeVideo());
		if(videoWnd == NULL)
		{
			MessageBox(NULL,L"视频位置已占满",NULL,NULL);
			return;
		}
		if(g_pMeetingFrame!=NULL)
		{
			g_pMeetingFrame->StartPublishVideo(0,AppSetting::GetInstance()->GetCameraIndex(),videoWnd);
		}
	}
}


bool DlgAVSetting::winEvent (MSG* msg, long *result)
{
	return false;
}

void DlgAVSetting::mousePressEvent(QMouseEvent *event)
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

void DlgAVSetting::mouseReleaseEvent(QMouseEvent *event)
{
	//只能是鼠标左键移动和改变大小
	if(event->button() == Qt::LeftButton) 
	{
		mouse_press = false;
	}

	//窗口移动距离
	move_point = event->globalPos() - pos(); 
}

void DlgAVSetting::mouseMoveEvent(QMouseEvent *event)
{
	//移动窗口
	if(mouse_press)   
	{
		QPoint move_pos = event->globalPos();
		move(move_pos - move_point);
	}
	//qDebug()<<"mouse move";
}

void DlgAVSetting::OnMicVolumeChange()
{
	if(m_pIZYAudioCapture)
	{
		m_pIZYAudioCapture->SetVolume(ui.sliderMic->value());
	}
}
void DlgAVSetting::OnSpeakerVolumeChange()
{
	if(m_pIZYAudioPlayer)
	{
		m_pIZYAudioPlayer->SetVolume(ui.sliderSpeaker->value());
	}
}