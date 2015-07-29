#ifndef SPEKERTESTWDG_H
#define SPEKERTESTWDG_H

#include <QWidget>
#include "ui_SpekerTestWdg.h"
#define TCHAR wchar_t
#include <typedef.h>
#include <ZYMediaFrame.h>
#include <QDebug>
#include "WaveFile.h"
#include "SeanProgressBar.h"
class SpekerTestWdg : public QWidget,public IZYAudioPlayerEvent
{
	Q_OBJECT

public:
	SpekerTestWdg(QWidget *parent = 0);
	~SpekerTestWdg();

	//返回最大音量
	virtual void OnZYAVPlayerEvent_MaxVolume(short nMaxVolume); 
    virtual bool OnZYAVPlayerEvent_GetAudioData(char* pAudioData, int& nAudioLen, uint64_t& ulRTPTime, uint64_t& ulDataBufferTime);
public:
	bool StartAudio();
	void StopAudio();
	void ShowAndInit();
	void Close();

signals:
	 void EventVolumeChange(int value);
	 public slots:

		 void OnSpeakerChange(int device);
		 void OnStartTest();
private:
	Ui::SpekerTestWdg ui;

	IZYAudioPlayer *m_pIZYAudioPlayer;

	WaveFile m_WaveFile;
	//已播放字节数
	int m_HasRead;

	SeanProgressBar *progressWdg;
};

#endif // SPEKERTESTWDG_H
