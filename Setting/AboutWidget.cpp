#include "AboutWidget.h"
#include "../stdafx.h"
#include <QFile>
AboutWidget * AboutWidget::a_instance = NULL;
AboutWidget *AboutWidget::GetInstance(){

	if (a_instance == NULL)
		a_instance = new AboutWidget();
	return a_instance;
	
}

AboutWidget::AboutWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
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
	connect(ui.closeBtn,SIGNAL(clicked()),this,SLOT(close()));
	ui.lblVersion->setText(QString("<span style=\" color:#ffffff;\">版本号：%1</span>").arg(g_pMeetingFrame->GetVersion()));
}

AboutWidget::~AboutWidget()
{

}
void AboutWidget::mousePressEvent(QMouseEvent *event){

	if (event->button() == Qt::LeftButton)
	{
		mouse_pressed = true;
		mouse_movePoint = event->pos();
	}
}

void AboutWidget::mouseMoveEvent(QMouseEvent *event){

	if (mouse_pressed)
	{
		QPoint movePoint = event->globalPos();
		this->move(movePoint - mouse_movePoint);
	}
}

void AboutWidget::mouseReleaseEvent(QMouseEvent *event){

	mouse_pressed = false;
}