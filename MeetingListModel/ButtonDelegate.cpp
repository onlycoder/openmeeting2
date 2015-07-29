#include "ButtonDelegate.h"
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QTableView>
#include "../stdafx.h"

ButtonDelegate::ButtonDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{

}

ButtonDelegate::~ButtonDelegate()
{

}

void ButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)const{

__super::paint(painter, option, index);
	if(!index.isValid() || NULL == painter)
	{
		return;
	}
	QVariant loVar = index.data(Qt::UserRole);
	QString strUserName = loVar.toString();
	//得到当前Item数据
	/*
	if (!loVar.canConvert<STRU_TTXGUI_UL_USER>())
	{
		return;
	}
	*/
	painter->save();
	QRect rcItem = option.rect;
	
	//QRect rcItem = option.rect;
	qDebug()<<"option.rect=== "<<option.rect;
	QPen pen;
	pen.setColor(QColor(102,102,102)); //#fe0002
	painter->setPen(pen);
	painter->drawImage(QPoint(rcItem.left()+18,rcItem.top()+20),QImage(tr(":/login/login/list_jr.png")),
		QRect(0,0,50,20));
	//qDebug()<<"column four"<<strUserName;
	//painter->drawText(rcItem.left()+28,rcItem.top()+10,100,35,Qt::TextWrapAnywhere,strUserName);

	painter->restore();

	//int gridHint = tableView->style()->styleHint(QStyle::SH_Table_GridLineColor, new QStyleOptionViewItemV4());
	//QColor gridColor = static_cast<QRgb>(gridHint);
	//pen = QPen(gridColor, 0, tableView->gridStyle());

	//QStyleOptionViewItem itemOption(option);
	//if(itemOption.state & QStyle::State_HasFocus)
	//	itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
	//QStyledItemDelegate::paint(painter,itemOption,index);
	//QPen oldPen = painter->pen();
	//painter->setPen(pen);
	//painter->setPen(oldPen);

	
}
bool ButtonDelegate::eventFilter(QObject *object, QEvent *event){

	switch (event->type()){
	case QEvent::MouseButtonPress:
			goinBtnPressed(object, event);
			break;
	}
	return __super::eventFilter(object, event);
}
//鼠标点击事件
void ButtonDelegate::goinBtnPressed(QObject *object, QEvent *event){

		QTableView* pView    =   qobject_cast<QTableView*>(object->parent());
		QMouseEvent* pEvent =   static_cast<QMouseEvent*>(event);
		QModelIndex loModelIndex = pView->indexAt(pEvent->pos());

		/*获取model中的数据*/
		int column = loModelIndex.column();
		int row = loModelIndex.row();
		if (row < 0)
		{
			return;
		}
		//int row = loModelIndex.row();
		QVariant varint = loModelIndex.data(Qt::DisplayRole);
		QString str = varint.toString();

		/*获取鼠标点击坐标*/
		/*	int x = pEvent->x();
		int y = pEvent->y();
		int x1 = pEvent->globalX();
		int y2 = pEvent->globalY();
		QPoint  p1 = pEvent->pos();
		QPoint p2 = pEvent->globalPos();
		*/
		PROOM_INFO roomInfo = g_pMeetingFrame->GetRoomInfo(row);
		if(roomInfo==NULL)
			return;
		QString pStr = QString(roomInfo->strPassword);
		int pBool = 0;
		if (pStr.length()>0)
		{
			pBool = 1;
		}else
			pBool = 0;

		QPoint  p1 = pEvent->pos();
		if (p1.x()>350&&p1.x()<405)
		{
			emit sendTableViewNumberRowAndWorth(row,pBool);
		}else
			return;

}

