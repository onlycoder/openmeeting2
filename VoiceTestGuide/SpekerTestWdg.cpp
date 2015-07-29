#include "SpekerTestWdg.h"
#include "../stdafx.h"
SpekerTestWdg::SpekerTestWdg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.navLabel->setPixmap(QPixmap(":/voiceTest/voiceTest/voice.png"));
	m_pIZYAudioPlayer = NULL;
	connect(ui.btnStartTest,SIGNAL(clicked()),this,SLOT(OnStartTest()));

	progressWdg = new SeanProgressBar(this);
	progressWdg->setGeometry(100,132,200,20);
	connect(this,SIGNAL(EventVolumeChange(int)),progressWdg,SLOT(getSoundVulume(int)));


}

SpekerTestWdg::~SpekerTestWdg()
{

}

//返回最大音量
void SpekerTestWdg::OnZYAVPlayerEvent_MaxVolume(short nMaxVolume){
	emit EventVolumeChange(nMaxVolume/100);
}

bool SpekerTestWdg::OnZYAVPlayerEvent_GetAudioData(char* pAudioData, int& nAudioLen, uint64_t& ulRTPTime, uint64_t& ulDataBufferTime){
	if(m_HasRead<(m_WaveFile.datalength))
	{
		if(m_WaveFile.datalength - m_HasRead>=640)
		{
			memcpy(pAudioData,(char*)(m_WaveFile.Data)+m_HasRead,640);
			m_HasRead+=640;
			nAudioLen=640;
		}
		else
		{
			memcpy(pAudioData,(char*)(m_WaveFile.Data)+m_HasRead,m_WaveFile.datalength - m_HasRead);
			//nAudioLen=m_WaveFile.datalength - m_HasRead;
			nAudioLen=640;
			m_HasRead+=640;
		}
		
		return true;
	}
	else
	{
		return false;
	}
}

void SpekerTestWdg::OnStartTest()
{
	m_WaveFile.WavRead("test.wav");
	int i = m_WaveFile.head.samplerate;
	m_HasRead = 0;
	StopAudio();
	StartAudio();
}

bool SpekerTestWdg::StartAudio()
{

	if(g_pMeetingFrame->HasStartAudio() == true)
	{
		//return false;
	}

	HWND hWnd = (HWND)(this->winId());
	AUDIO_FORMAT af;
	af.audioCodec = PCM_16;
	
	bool ret = CreateZYAudioPlayer(&m_pIZYAudioPlayer,this);
	if(ret)
	{
		ret = m_pIZYAudioPlayer->Connect(true,hWnd,&af,AppSetting::GetInstance()->GetSpeakerIndex()-1);
		return ret;
	}
	return false;
}
void SpekerTestWdg::StopAudio()
{
	if(m_pIZYAudioPlayer)
	{
		m_pIZYAudioPlayer->ReleaseConnections();
		m_pIZYAudioPlayer = NULL;
	}
}

void SpekerTestWdg::OnSpeakerChange(int device)
{
	AppSetting::GetInstance()->SetSpeakerIndex(device);
}

void SpekerTestWdg::ShowAndInit()
{
	show();
	ui.cmbSpeakerList->addItem(("Windows默认设备"));
	char soundOutChar[256]={0};
	int m_speakerCount = GetSoudPlayDeviceCount();
	for(int i = 0;i < m_speakerCount;i ++){
		GetSoundPlayDeviceName(i,soundOutChar);
		QString outStr = QString::fromWCharArray((wchar_t*)soundOutChar);
		ui.cmbSpeakerList->addItem(outStr);
	}
	ui.cmbSpeakerList->addItem(("禁用"));
	int index = AppSetting::GetInstance()->GetSpeakerIndex();
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
	
}

void SpekerTestWdg::Close()
{
	ui.cmbSpeakerList->disconnect();
	ui.cmbSpeakerList->clear();
	StopAudio();
	hide();
}