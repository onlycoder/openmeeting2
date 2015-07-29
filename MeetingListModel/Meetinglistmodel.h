#ifndef MEETINGLISTMODEL_H
#define MEETINGLISTMODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include "MeetingListWdg.h"

class MeetingListModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	static MeetingListModel * m_instance;
	static MeetingListModel * GetInstance();

public:
	MeetingListModel(QObject *parent = 0);
	~MeetingListModel();
	int m_total;
public:
	virtual int rowCount(const QModelIndex &parent = QModelIndex())const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex())const;
	QVariant data(const QModelIndex &index, int role /* = Qt::DisplayRole */)const;
	QVariant headerData(int section, Qt::Orientation orientation, int role /* = Qt::DisplayRole */)const;
	virtual bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
private:
	QStringList headerList;
	


};

#endif // MEETINGLISTMODEL_H
