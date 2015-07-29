#include "Meetinglistmodel.h"
#include "../stdafx.h"
#include "Meetinglistwdg.h"

MeetingListModel * MeetingListModel::m_instance = NULL;;
MeetingListModel * MeetingListModel::GetInstance()
{
	if(m_instance == NULL)
	{
		m_instance = new MeetingListModel();
	}
	return m_instance;
}

MeetingListModel::MeetingListModel(QObject *parent)
	: QAbstractTableModel(parent)
{
	m_total = 0;
	headerList<<"房间编号"<<"会议名称"<<"在线/总共"<<"密码"<<"";
    
}
MeetingListModel::~MeetingListModel()
{
}
int MeetingListModel::columnCount(const QModelIndex &parent /* = QModelIndex */)const{

	return headerList.count();
}
int MeetingListModel::rowCount(const QModelIndex &parent /* = QModelIndex */)const{
	return m_total;
}
QVariant MeetingListModel::data(const QModelIndex &index, int role )const{
	if (!index.isValid())
		return QVariant();

	/*文字居中显示*/
	if( Qt::TextAlignmentRole == role )  
		return Qt::AlignCenter;     
	if(index.row()<0)
		return QVariant();
	PROOM_INFO roomInfo = g_pMeetingFrame->GetRoomInfo(index.row());
	if(roomInfo == NULL)
		return NULL;
	
	if(role == Qt::DisplayRole)
	{
		switch(index.column()){
		case 0:
			return roomInfo->ulRoomID;
			break;
		case 1:
			return "";/*QString::fromUtf8(roomInfo->strRoomName)*/;
			break;
		case 2:
			 return QString("%1/%2").arg(roomInfo->onlineUser).arg(roomInfo->maxUser);
			break;
		case 3:
			{
				QString pStr = QString(roomInfo->strPassword);
			  if (pStr.length() > 0)
			  {
				  return "是";
			  }
			  else
				  return "否";
			}
			break;
		case 4:
			return "";
			break;
		default:
			return QVariant();

		}
	}
	return QVariant();
}
QVariant MeetingListModel::headerData(int section, Qt::Orientation orientation, int role )const
{
	if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
		return headerList[section];
	return QAbstractTableModel::headerData(section,orientation,role);

}
bool MeetingListModel::insertRows(int row, int count, const QModelIndex & parent)
{
	beginInsertRows(parent, row, row + count-1);
	m_total += count;
	endInsertRows();
	return true;
}

bool MeetingListModel::removeRows(int row, int count, const QModelIndex & parent)
{
	beginRemoveRows(parent, row, row + count-1);
	m_total -= count;
	endRemoveRows();
	return true;
}
