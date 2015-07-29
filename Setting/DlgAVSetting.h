#ifndef DLGAVSETTING_H
#define DLGAVSETTING_H

#include <QDialog>
#include "ui_DlgAVSetting.h"
#include <typedef.h>
#include <ZYMediaFrame.h>
#include "../Video/VideoWin.h"
#include "../common/ccommondialog.h"
#include "../common/ccommonwidget.h"
class DlgAVSetting : public CCommonWidget,public IZYVideoCaptureEvent
	,public IZYAudioCaptureEvent,public IZYAudioPlayerEvent
{
	Q_OBJECT

public:
	DlgAVSetting(QWidget *parent = 0, CommonWndFlags WndFlag = enum_GGWndCloseButtonHint);
	~DlgAVSetting();
private:
	static DlgAVSetting* m_instance;
public:
	static DlgAVSetting* GetInstance();

	virtual void OnZYAVCaptureEvent_VideoData(char* pData, int nLen, bool bKeyFrame, VIDEO_SIZE videoSize, VIDEO_CODEC videoCodec){

	};

	/*record*/
	virtual void OnZYAVCaptureEvent_AudioData(char* pData, int nLen, AUDIO_CODEC audioCodec);

	virtual void OnZYAVCaptureEvent_MaxVolume( short nMaxVolume){
	};
	/*play*/
	virtual void OnZYAVPlayerEvent_MaxVolume(short nMaxVolume) {};

	virtual bool OnZYAVPlayerEvent_GetAudioData(char* pAudioData, int& nAudioLen, uint64_t& ulRTPTime, uint64_t& ulDataBufferTime){return true;};

	void InitAudioDevList();
	void InitVideoDevList();
	bool StartAudio();
	void StopAudio();
	bool StartVideo();
	void StopVideo();
	void ShowAndInit();

	signals:
		void EventRestartAudio();
	public slots:
		void OnOK();
		void OnCancel();
		void OnMicChange(int index);
		void OnSpeakerChange(int index);
		void OnCameraChange(int index);
		void OnTabChange(int index);
		void OnRestartAudio();
		void OnClose();
		void OnMicVolumeChange();
		void OnSpeakerVolumeChange();
protected:

	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	
	virtual bool winEvent (MSG* msg, long *result);
private:
	Ui::DlgAVSetting ui;
	IZYVideoCapture* m_pIZYVideoCapture;
	IZYAudioCapture* m_pIZYAudioCapture;
	IZYAudioPlayer * m_pIZYAudioPlayer;
	unsigned int m_micCount;
	unsigned int m_speakerCount;
	unsigned int m_cameraCount;
	bool m_bResumeAudio; //是否需要恢复语音,原来已打开语音，测试语音时暂时关闭
	bool m_bResumeVideo;
	QPoint move_point; //移动的距离
	bool mouse_press; //按下鼠标左键
	VideoWin * m_pVideoWin;
};

#endif // DLGAVSETTING_H
