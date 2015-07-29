#include "VoiceTestWdg.h"
#include <QFile>
#include "../stdafx.h"
#include "../Setting/AppSetting.h"
VoiceTestWdg *VoiceTestWdg::v_instance = NULL;
VoiceTestWdg *VoiceTestWdg::GetVoiceInstance(){
	if(v_instance == NULL)
		v_instance = new VoiceTestWdg();

	return v_instance;
}

VoiceTestWdg::VoiceTestWdg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QFile qssFile(":/qss/voiceTestGuide.qss");
	QString qss;
	qssFile.open(QFile::ReadOnly);
	if(qssFile.isOpen())
	{  
		qss = QString(qssFile.readAll());
		this->setStyleSheet(qss);
		qssFile.close();
	}

	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);

	isNext = true;
	micTestWdg = new MicTestWdg(ui.contentWidget);
	spekerTestWdg = new SpekerTestWdg(ui.contentWidget);
	

	connect(ui.nextBtn,SIGNAL(clicked()),this,SLOT(nextBtnClicked()));
	connect(ui.cancelBtn,SIGNAL(clicked()),this,SLOT(cancelBtnClicked()));
	connect(ui.closeBtn,SIGNAL(clicked()),this,SLOT(OnClose()));
}

VoiceTestWdg::~VoiceTestWdg()
{

}
void VoiceTestWdg::ShowAndInit()
{
	this->show();
	spekerTestWdg->hide();
	micTestWdg->ShowAndInit();
	m_bResumeAudio = g_pMeetingFrame->HasStartAudio();
}
void VoiceTestWdg::nextBtnClicked(){


	if (isNext)
	{
		//下一步
		micTestWdg->Close();
		spekerTestWdg->ShowAndInit();
		ui.nextBtn->setText("上一步");
		ui.cancelBtn->setText("完成");
	} 
	else
	{
		//上一步
		spekerTestWdg->Close();
		micTestWdg->ShowAndInit();
		ui.nextBtn->setText("下一步");
		ui.cancelBtn->setText("取消");
	}
	isNext = !isNext;
}

void VoiceTestWdg::cancelBtnClicked()
{
	OnClose();
}

void VoiceTestWdg::mouseMoveEvent(QMouseEvent *event)
{

	if (mouse_pressed)
	{
		QPoint movePoint = event->globalPos();
		this->move(movePoint - mouse_movePoint);
	}
}

void VoiceTestWdg::mousePressEvent(QMouseEvent *event){

	if (event->button() == Qt::LeftButton)
	{
		mouse_pressed = true;
		mouse_movePoint = event->pos();
	}
}

void VoiceTestWdg::mouseReleaseEvent(QMouseEvent *event){

	mouse_pressed = false;
}

void VoiceTestWdg::OnClose()
{
	micTestWdg->StopAudio();
	spekerTestWdg->StopAudio();
	micTestWdg->setHidden(false);
	spekerTestWdg->setHidden(true);
	ui.nextBtn->setText("下一步");
	ui.cancelBtn->setText("取消");
	isNext = !isNext;
	this->close();
	if(m_bResumeAudio)
	{
		g_pMeetingFrame->StartPublishAudio(0,AppSetting::GetInstance()->GetMicIndex()-1);
	}
}
