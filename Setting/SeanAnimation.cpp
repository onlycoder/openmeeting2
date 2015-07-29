#include "SeanAnimation.h"
#include <QFile>
#include <QDebug>
#define  LABELHEIGH 17

SeanAnimation *SeanAnimation::a_instance = NULL;
SeanAnimation *SeanAnimation::GetAnimationInstance(){
	if(a_instance == NULL)
		a_instance = new SeanAnimation();
	return a_instance;
}



SeanAnimation::SeanAnimation(QWidget *parent)
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
	setWindowFlags(Qt::FramelessWindowHint);//这个是widget的标题栏和边框去掉
	setAttribute(Qt::WA_TranslucentBackground);
    ui.warningLabel->setPixmap(QPixmap(":/setting/setting/warning_orange.png"));
	isTrue = true;
}



void SeanAnimation::timerUpDate(){



	QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
	animation->setDuration(500);
	animation->setStartValue(QRect((parentWdg->width() - this->width())/2,5,this->width(),getHeigh));
	animation->setEndValue(QRect(parentWdg->width(),5, this->width(),getHeigh));
	animation->start();
	if (timer->isActive())
	{
		timer->stop();
	}
	//界面动画，改变透明度的方式消失1 - 0渐变
	
}

SeanAnimation::~SeanAnimation()
{

}
void SeanAnimation::setTiptitleWithString(QString tipStr)
{

	ui.tipLabel->setText(tipStr);
	int tWidth = fontMetrics().width(tipStr) + 13;
	int tHeigh = LABELHEIGH + 13;
	if (tWidth / 250 > 1)
	{
	  tHeigh = LABELHEIGH*(tWidth/250 + 3) +13;
	}

	this->setGeometry((parentWdg->width() - this->width())/2,5,this->width(),tHeigh);

	ui.animationLabel->setGeometry(0,0,300,tHeigh);
	ui.tipLabel->setGeometry(30,0,250,tHeigh);
	
	QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
	animation->setDuration(500);
	animation->setStartValue(QRect(-250,5, this->width(),tHeigh+ 13));
	animation->setEndValue(QRect((parentWdg->width() - this->width())/2,5,this->width(),tHeigh));
	animation->start();

	getHeigh = tHeigh;
	timer = new QTimer(this);
	connect(timer,SIGNAL(timeout()),this,SLOT(timerUpDate()));
	timer->start(2000);
}
