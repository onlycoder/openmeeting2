#include "MicTestWdg.h"

#include "../stdafx.h"


MicTestWdg::MicTestWdg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.navLabel->setPixmap(QPixmap(":/voiceTest/voiceTest/mic.png"));
	m_pIZYAudioCapture = NULL;
	m_pIZYAudioPlayer = NULL;
	
	progressWdg = new SeanProgressBar(this);
	progressWdg->setGeometry(20,135,200,20);
	connect(this,SIGNAL(EventVolumeChange(int)),progressWdg,SLOT(getSoundVulume(int)));


}

MicTestWdg::~MicTestWdg()
{

}

void MicTestWdg::OnMicChange(int device){

	AppSetting::GetInstance()->SetMicIndex(device);
	StopAudio();
	StartAudio();
}


// 获取声音变化的回调函数
// void MicTestWdg::OnZYAVCaptureEvent_MaxVolume( short nMaxVolume)
// {
// 	qDebug()<<"获取声音变化的回调函数OnZYAVCaptureEvent_MaxVolume"<<nMaxVolume;
// 	//发送信号
// 	emit EventVolumeChange(nMaxVolume/100);
// }

bool MicTestWdg::StartAudio()
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
void MicTestWdg::StopAudio()
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
void MicTestWdg::ShowAndInit()
{
	show();
	char soundChar[256]={0};
	ui.cmbMicList->addItem(("Windows默认设备"));
	int m_micCount =  GetSoundCaptureDeviceCount();
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

	connect(ui.cmbMicList,SIGNAL(currentIndexChanged(int)),this,SLOT(OnMicChange(int)));
	StartAudio();
}

void MicTestWdg::Close()
{
	ui.cmbMicList->disconnect();
	ui.cmbMicList->clear();
	StopAudio();
	hide();
}