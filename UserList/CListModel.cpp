#include "CListModel.h"
#include <QString>
#include "../stdafx.h"
CListModel::CListModel(QObject *parent)
	: QAbstractListModel(parent)
{
	m_iRowCount = 0;
}

CListModel::~CListModel()
{

}

QVariant CListModel::data(const QModelIndex & index, int role)const
{
	if(!index.isValid() || role != Qt::UserRole)
	{
		return QVariant();
	}
	QVariant loVarData;
	if(g_pMeetingFrame!=NULL)
	{  /*
		Client_Online_User_Info userInfo;
		if(g_pMeetingFrame->GetUser(index.row(),&userInfo)
			loVarData.setValue((void*)(&userInfo));
		*/
	}
	return loVarData;
}

int CListModel::rowCount(const QModelIndex & parent)const
{
	return m_iRowCount;
}
    
bool CListModel::insertRows(int row, int count, const QModelIndex & parent)
{
	beginInsertRows(parent, row, row + count-1);
	m_iRowCount += count;
	endInsertRows();
	return true;
}

bool CListModel::removeRows(int row, int count, const QModelIndex & parent)
{
	beginRemoveRows(parent, row, row + count-1);
	m_iRowCount -= count;
	endRemoveRows();
	return true;
}

