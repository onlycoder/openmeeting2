#ifndef VOICETESTWDG_H
#define VOICETESTWDG_H

#include <QWidget>
#include "ui_VoiceTestWdg.h"
#include <QMouseEvent>
#include <QPoint>
#include "MicTestWdg.h"
#include "SpekerTestWdg.h"

class VoiceTestWdg : public QWidget
{
	Q_OBJECT

public:
	VoiceTestWdg(QWidget *parent = 0);
	~VoiceTestWdg();

	static VoiceTestWdg *v_instance;
	static VoiceTestWdg *GetVoiceInstance();
	void ShowAndInit();
private:
	Ui::VoiceTestWdg ui;

	bool isNext;
	MicTestWdg *micTestWdg;
	SpekerTestWdg *spekerTestWdg;

	QPoint mouse_movePoint;
	bool mouse_pressed;



	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	public slots:
		void nextBtnClicked();//确定
		void cancelBtnClicked();//取消
		void OnClose();
private:
	bool m_bResumeAudio; //是否需要恢复语音,原来已打开语音，测试语音时暂时关闭
};

#endif // VOICETESTWDG_H
