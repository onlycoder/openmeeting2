#include "ChatListModelDelegate.h"

#include "../stdafx.h"
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QListView>
#include <QBrush>
#include <QDateTime>

ChatListModelDelegate::ChatListModelDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{

}

ChatListModelDelegate::~ChatListModelDelegate()
{

}

void ChatListModelDelegate::SetParentUI(QObject* apObjUI){

	if (apObjUI == NULL)
	{
		return;
	}
	parentUI = apObjUI;
}
void ChatListModelDelegate::paint(QPainter *painter,const QStyleOptionViewItem &option,const QModelIndex &index)const{

	__super::paint(painter, option, index);
	if(!index.isValid() || NULL == painter)
	{
		return;
	}

	QVariant loVar = index.data(Qt::UserRole);
	PMsgItem chatInfo =(PMsgItem)(loVar.value<void*>());

	QString chatMessage;
	QString nameStr;
	if(chatInfo!=NULL){
	 chatMessage = QString::fromLocal8Bit(chatInfo->msg);//中文
	 nameStr = QString::fromUtf8(chatInfo->userName);
	}
	else
		return;

	QRect rcItem = option.rect;
	//画矩形背景
	QLinearGradient linearGrad(QPointF(0, 0), QPointF(300, 110));
	linearGrad.setColorAt(0, Qt::white);
	linearGrad.setColorAt(1, Qt::green);
	painter->setRenderHint(QPainter::Antialiasing);  //消除锯齿  
	painter->setBrush(QBrush(linearGrad));
	painter->fillRect(rcItem,Qt::white);

	//得到当前Item数据
	QPen pen;

	if (chatInfo->ulUserID == 0)
	{
		/*自己发送的消息*/
		pen.setColor(QColor(0,100,0)); //#fe0002
		painter->setPen(pen);
		QFont senderFont("微软雅黑",10);
		painter->setFont(senderFont);
		painter->drawText(rcItem.left()+5,rcItem.top(),rcItem.right()-rcItem.left()-20,35,Qt::AlignRight,"我"+QString(" 说:"));

		
		
		pen.setColor(QColor::fromRgba(chatInfo->color));
		painter->setPen(pen);
		QFont font(QString::fromLocal8Bit(chatInfo->familly),chatInfo->font_size); //使用wenquanyi字体 
		painter->setFont(font);

		painter->drawText(rcItem.left()+10,rcItem.top()+15,rcItem.right()-rcItem.left()-20,35,Qt::AlignRight,chatMessage);

	}else{

		/*收到对方消息*/
		pen.setColor(QColor(0,0,255)); //#fe0002
		painter->setPen(pen);
		QFont senderFont("微软雅黑",10);
		painter->setFont(senderFont);
		painter->drawText(rcItem.left()+5,rcItem.top(),rcItem.right()-rcItem.left()-20,35,Qt::AlignLeft,nameStr+QString(" 说:"));

		
		pen.setColor(QColor::fromRgba(chatInfo->color));
		painter->setPen(pen);
		QFont font(QString::fromLocal8Bit(chatInfo->familly),chatInfo->font_size); //使用wenquanyi字体 
		painter->setFont(font);

		painter->drawText(rcItem.left()+10,rcItem.top()+15,rcItem.right()-rcItem.left()-20,35,Qt::AlignLeft,chatMessage);
	}
	painter->restore();

}
bool ChatListModelDelegate::eventFilter(QObject *object, QEvent *event)
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

