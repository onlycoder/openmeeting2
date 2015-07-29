#include "ChatListModel.h"
#include <QString>
#include "../stdafx.h"
ChatListModel::ChatListModel(QObject *parent)
	: QAbstractListModel(parent)
{
	totalRowCount = 0;
}

ChatListModel::~ChatListModel()
{

}

QVariant ChatListModel::data(const QModelIndex & index, int role)const
{
	if(!index.isValid() || role != Qt::UserRole)
	{
		return QVariant();
	}
	QVariant chatData;
	if(g_pMeetingFrame!=NULL)
	{
		 PMsgItem chatInfo = g_pMeetingFrame->GetChatMsgItem(index.row());
		chatData.setValue((void*)(chatInfo));
		qDebug()<<"chatData == "<< QString::fromUtf8(chatInfo->msg);
	}
	
	return chatData;
}

int ChatListModel::rowCount(const QModelIndex & parent)const
{
	return totalRowCount;
}

bool ChatListModel::insertRows(int row, int count, const QModelIndex & parent)
{
	beginInsertRows(parent, row, row + count-1);
	totalRowCount += count;
	endInsertRows();
	return true;
}



