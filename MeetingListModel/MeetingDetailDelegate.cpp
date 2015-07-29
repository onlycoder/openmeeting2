#include "MeetingDetailDelegate.h"
#include <QDateTime>


MeetingDetailDelegate::MeetingDetailDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{

}

MeetingDetailDelegate::~MeetingDetailDelegate()
{

}

void MeetingDetailDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index)const{


	__super::paint(painter, option, index);
	if(!index.isValid() || NULL == painter)
	{
		return;
	}
	
	PROOM_INFO roomInfo = g_pMeetingFrame->GetRoomInfo(index.row());


	painter->save();
	QRect rcItem = option.rect;
	QPen pen;
	pen.setColor(QColor(0,0,0)); //#fe0002
	QFont senderFont("微软雅黑",10);
	painter->setFont(senderFont);
	painter->setPen(pen);
	//qDebug()<<"column four"<<strUserName;
	painter->drawText(rcItem.left()+10,rcItem.top()+5,rcItem.right()-rcItem.left()-20,20,Qt::AlignLeft,"会议名称："+QString::fromUtf8(roomInfo->strRoomName));

	pen.setColor(QColor(190,190,190)); //#fe0002
	QFont sFont("微软雅黑",8);
	painter->setFont(sFont);
	painter->setPen(pen);

	QDateTime sTime = QDateTime::fromTime_t(roomInfo->startTime);
	QDateTime eTime = QDateTime::fromTime_t(roomInfo->endTime);

	painter->drawText(rcItem.left()+10,rcItem.top()+25,rcItem.right()-rcItem.left()-20,15,Qt::AlignLeft,"开始时间："+sTime.toString("yyyy-MM-dd hh:mm:ss"));
	painter->drawText(rcItem.left()+10,rcItem.top()+40,rcItem.right()-rcItem.left()-20,15,Qt::AlignLeft,"结束时间："+eTime.toString("yyyy-MM-dd hh:mm:ss"));
	painter->restore();
}