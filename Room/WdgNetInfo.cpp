#include "WdgNetInfo.h"
#include <QPainter>
#include <QDebug>
WdgNetInfo * WdgNetInfo::m_instance = NULL;;
WdgNetInfo * WdgNetInfo::GetInstance()
{
	if(m_instance == NULL)
	{
		m_instance = new WdgNetInfo();
	}
	return m_instance;
}
WdgNetInfo::WdgNetInfo(QWidget *parent)
	: QWidget(parent)
{
	m_delay = 0;
	m_uploadLost = 0;
	m_downloadLost = 0;
	setWindowFlags(Qt::FramelessWindowHint);//这个是widget的标题栏和边框去掉
	//setAttribute(Qt::WA_TranslucentBackground);
	this->setGeometry(0,0,122,60);
}

WdgNetInfo::~WdgNetInfo()
{

}

void WdgNetInfo::OnNetInfo(int delay,int uploadLost,int downloadLost)
{
	m_delay = delay;
	m_uploadLost  = uploadLost;
	m_downloadLost = downloadLost;
	qDebug()<<"**OnStream Netinfo delay:"<<delay<<"send lost:"<<uploadLost<<"recv lost:"<<downloadLost;
	update();
}

void WdgNetInfo::paintEvent ( QPaintEvent * event )
{
	QPainter painter(this);
	QPen pen;
	pen.setColor(QColor(102,0,0));  //设置画笔为红色
	painter.setPen(pen);  //选择画笔
	painter.drawRect(2,2,118,56);
	painter.fillRect(3,3,117,55,QColor(239,255,191));
	pen.setColor(QColor(0,0,0));  //设置画笔为红色
	painter.setPen(pen);  //选择画笔
	//上行丢包率:%2,下行丢包率:%3
	painter.drawText(16,17,QString("网络延时: %1毫秒").arg(m_delay));
	painter.drawText(16,34,QString("上行丢包率: %1%").arg(m_uploadLost));
	painter.drawText(16,51,QString("下行丢包率: %1%").arg(m_downloadLost));
	painter.end(); 
}

void WdgNetInfo::mousePressEvent(QMouseEvent *event)
{
	this->hide();
}

void WdgNetInfo::enterEvent ( QEvent * event )
{
	this->show();
}

void WdgNetInfo::leaveEvent ( QEvent * event )
{
	this->hide();
}