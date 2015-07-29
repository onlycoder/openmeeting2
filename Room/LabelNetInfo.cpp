#include "LabelNetInfo.h"
#include <QPainter>
#include <QDebug>
#include <QDateTime>
LabelNetInfo::LabelNetInfo(QWidget *parent)
	: QLabel(parent)
{
	m_lastEventTime = 0;
}

LabelNetInfo::~LabelNetInfo()
{

}

void LabelNetInfo::enterEvent ( QEvent * event )
{
	qint64 t2 = QDateTime::currentMSecsSinceEpoch();
	if(t2-m_lastEventTime>500){
		emit EventEnter();
		m_lastEventTime = t2;
	}
}

void LabelNetInfo::leaveEvent ( QEvent * event )
{
	
	qint64 t2 = QDateTime::currentMSecsSinceEpoch();
	if(t2-m_lastEventTime>500){
		emit EventLeave();
		m_lastEventTime = t2;
	}
}