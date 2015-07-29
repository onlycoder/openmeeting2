#include "CListItemDelegate.h"
#include <QPainter>
#include <QEvent>
#include <QCoreApplication>
#include <QListView>
#include <QMouseEvent>
#include <QToolTip>
#include <QApplication>
#include "../stdafx.h"
#include "../Room/LeftWdg.h"
CListItemDelegate::CListItemDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{

}

CListItemDelegate::~CListItemDelegate()
{

}
void CListItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	__super::paint(painter, option, index);
	if(!index.isValid() || NULL == painter)
	{
		return;
	}
	QVariant loVar = index.data(Qt::UserRole);
	Client_Online_User_Info userInfo;
	QString strUserName;
	if(g_pMeetingFrame->GetUser(index.row(),&userInfo))
		strUserName = QString::fromUtf8(userInfo.strUserName);
	else
		return;
	//得到当前Item数据
	painter->save();
	QRect rcItem = option.rect;
	QPen pen;
	pen.setColor(QColor(102,102,102)); //#fe0002
	painter->setPen(pen);
	if(userInfo.userRole == 1)
	{
		painter->drawImage(QPoint(rcItem.left()+4,rcItem.top()+9),QImage(tr(":/skin/room/user_type.png")),
			QRect(0,0,17,17));
	}
	else
	{
		painter->drawImage(QPoint(rcItem.left()+4,rcItem.top()+9),QImage(tr(":/skin/room/user_type.png")),
			QRect(34,0,17,17));
	}
	
	painter->drawText(rcItem.left()+28,rcItem.top()+10,rcItem.right()-rcItem.left()-60,35,Qt::TextWrapAnywhere,strUserName);
	if(userInfo.ulAudioSSRC>0)
	{
		short audioVolume = LeftWdg::GetInstance()->GetAudioValue(userInfo.ulAudioSSRC);
		if(audioVolume>0)
		{
		qDebug()<<"audio value: "<<audioVolume<<"rcItem.top()"<<rcItem.top();

		pen.setColor(QColor(220,220,220));  //设置画笔色
		painter->setPen(pen);  //选择画笔
		pen.setBrush(Qt::gray);//填充颜色
		pen.setStyle(Qt::SolidLine);//画笔样式
		pen.setCapStyle(Qt::RoundCap); 
		for (int i = 0;i < 5;i ++)
		{
			painter->drawRoundRect(rcItem.right()-35,rcItem.bottom() - 4*i -9,8,2,0,0);//
		}
		pen.setColor(QColor(0,191,255));
		painter->setPen(pen); 
		pen.setBrush(Qt::darkBlue);
		pen.setStyle(Qt::SolidLine); 
		pen.setCapStyle(Qt::RoundCap); 
		audioVolume = audioVolume/327/20;
		for (int j = 0;j < audioVolume;j ++)
		{
			painter->drawRoundRect(rcItem.right()-35,rcItem.bottom() - 4*j -9,8,2,0,0);//圆角矩形,99,99 rcItem
		}
			/*painter->drawText(rcItem.right()-62,rcItem.top()+10,30,35,Qt::TextWrapAnywhere,QString("v:%1").arg(audioVolume/327));*/
		}
		painter->drawImage(QPoint(rcItem.right()-16,rcItem.top()+11),QImage(":/skin/button/btnSoundTip2.png"));
	}

	painter->restore();
}

void CListItemDelegate::SetParentUI(QObject* apObjUI)
{
	if(NULL == apObjUI)
	{
		return;
	}

	m_objParentUI = apObjUI;
}

bool CListItemDelegate::eventFilter(QObject *object, QEvent *event)
{
	switch(event->type())
	{
	case QEvent::MouseMove:
		//OnMouseMoveEvent(object, event);
		break;

	case QEvent::Leave:
		//OnMouseLeaveEvent(object, event);
		break;

	case QEvent::MouseButtonPress:
		{
			QListView* pView    =   qobject_cast<QListView*>(object->parent());
			QMouseEvent* pEvent =   static_cast<QMouseEvent*>(event);
			QModelIndex loModelIndex = pView->indexAt(pEvent->pos());
			int x = pEvent->x();
			int y = pEvent->y();
			int x1 = pEvent->globalX();
			int y2 = pEvent->globalY();
			QPoint  p1 = pEvent->pos();
			QPoint p2 = pEvent->globalPos();
		}
		//OnMousePressEvent(object, event);
		break;

	case QEvent::MouseButtonDblClick:
		//OnMouseDblClickEvent(object, event);
		break;

	}

	return __super::eventFilter(object, event);
}
