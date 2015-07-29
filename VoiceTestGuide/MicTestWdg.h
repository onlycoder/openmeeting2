#ifndef MICTESTWDG_H
#define MICTESTWDG_H

#include <QWidget>
#include "ui_MicTestWdg.h"

#define TCHAR wchar_t
#include <typedef.h>
#include <ZYMediaFrame.h>
#include <QDebug>
#include "SeanProgressBar.h"

class MicTestWdg : public QWidget, public IZYAudioCaptureEvent,public IZYAudioPlayerEvent
{
	Q_OBJECT

public:
	MicTestWdg(QWidget *parent = 0);
	~MicTestWdg();

	virtual void OnZYAVCaptureEvent_AudioData(char* pData, int nLen, AUDIO_CODEC audioCodec){
		if(m_pIZYAudioPlayer)
			m_pIZYAudioPlayer->DeliverAudioData(pData,nLen,0);
	};

	//获取声音变化的回调函数
	virtual void OnZYAVCaptureEvent_MaxVolume( short nMaxVolume){
		
	};
	//返回最大音量
	virtual void OnZYAVPlayerEvent_MaxVolume(short nMaxVolume) {
		qDebug()<<"获取声音变化的回调函数OnZYAVCaptureEvent_MaxVolume"<<nMaxVolume;
		//发送信号
		emit EventVolumeChange(nMaxVolume/100);
	}

	virtual bool OnZYAVPlayerEvent_GetAudioData(char* pAudioData, int& nAudioLen, uint64_t& ulRTPTime, uint64_t& ulDataBufferTime) {
		//qDebug()<<"pAudioData"<<pAudioData;
		return true;
	}


public:
	bool StartAudio();
	void StopAudio();
	void ShowAndInit();
	void Close();
private:
	Ui::MicTestWdg ui;

	IZYAudioCapture *m_pIZYAudioCapture;
	IZYAudioPlayer *m_pIZYAudioPlayer;

	SeanProgressBar *progressWdg;
signals:
		void EventVolumeChange(int value);
	public slots:
		void OnMicChange(int device);
};

#endif // MICTESTWDG_H
