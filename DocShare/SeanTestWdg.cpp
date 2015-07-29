#include "../stdafx.h"
#include "SeanTestWdg.h"
#include <QDebug>
#include <QFile>
#include <QBitmap.h>
#include <QPainter.h>

SeanTestWdg::SeanTestWdg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	
	QFile qssFile(":/qss/setting.qss");
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
 	setMouseTracking(true);

	ui.pageLabel->setText("1/20");
	this->setWindowOpacity(0.2);
	timer = new QTimer(this);
	connect(timer,SIGNAL(timeout()),this,SLOT(timeoutToHiddenToolWdg()));
}

SeanTestWdg::~SeanTestWdg()
{

}

void SeanTestWdg::enterEvent( QEvent * event ){
	
	emit signalOfEnterTestWdg();
}
void SeanTestWdg::leaveEvent( QEvent * event ){
	emit signalOfLeaveTestWdg();
}
void SeanTestWdg::mouseMoveEvent(QMouseEvent* event){
}
//»­±Ê×´Ì¬ÉèÖÃ
void SeanTestWdg::setPenBntStats(){
	
	if (penBtnChecked)
	{
		ui.penBtn->setChecked(false);
	}else{
		
		ui.penBtn->setChecked(true);
	}
	penBtnChecked = !penBtnChecked;
}
void SeanTestWdg::timeoutToHiddenToolWdg(){
	ui.contentWdg->setHidden(true);
	timer->stop();
}

void SeanTestWdg::JustShowLabel()
{
	ui.pageLabel->show();
	ui.penBtn->hide();
	ui.lastBtn->hide();
	ui.nextBtn->hide();
	//if(g_pMeetingFrame!=NULL&&g_pMeetingFrame->GetUserRole(0)!=1)
	ui.powerBtn->hide();
	
}
void SeanTestWdg::ShowAll()
{
	ui.pageLabel->show();
	ui.penBtn->show();
	ui.lastBtn->show();
	ui.nextBtn->show();
	ui.powerBtn->show();
}

void SeanTestWdg::ShowCloseDocBtn(bool bShow){
	if(bShow)
		ui.powerBtn->show();
	else 
		ui.powerBtn->hide();
}