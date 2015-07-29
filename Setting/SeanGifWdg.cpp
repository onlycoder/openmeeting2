#include "SeanGifWdg.h"
#include <QFile>
#include <QPixmap>
#include <QDebug>
#include "LeftWdg.h"

SeanGifWdg* SeanGifWdg::g_Instance = NULL;
SeanGifWdg* SeanGifWdg::GetInstance(){
	if (g_Instance == NULL)
	{
		g_Instance = new SeanGifWdg();
	}
	return g_Instance;
}

SeanGifWdg::SeanGifWdg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	//setAttribute(Qt::WA_TranslucentBackground);
	int j = 0;
	for (int i = 0;i < 135;i ++)
	{
		j = i / 14;
		SeanButton *btn = new SeanButton(this);
		btn->setGeometry((i - 14 * j)*28+8,28*j+ 10,24,24);
		qDebug()<<"03502+i == \n"<<3502+i;
		btn->bTag = i;
		btn->setImageWithTag(i);
		btn->setIconSize(QSize(24, 24));   
		btn->setFixedSize(24, 24);   
		connect(btn,SIGNAL(clicked()),this,SLOT(gifBtnClickedWithBtntag()));
	}
	connect(this,SIGNAL(sendGifTagWithNum(unsigned long long)),LeftWdg::GetInstance(),SLOT(slotOfGifNumberGet(unsigned long long )));
}

SeanGifWdg::~SeanGifWdg()
{

}

void SeanGifWdg::enterEvent ( QEvent * event )
{
	emit EventEnter();
}

void SeanGifWdg::leaveEvent ( QEvent * event )
{
	emit EventLeave();
}

void SeanGifWdg::gifBtnClickedWithBtntag(){

	SeanButton *btn = static_cast<SeanButton*>(sender());
	qDebug()<<"btn.bTag == "<<btn->bTag;
	emit sendGifTagWithNum(btn->bTag);
}
