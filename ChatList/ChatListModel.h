#ifndef CHATLISTMODEL_H
#define CHATLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>
class ChatListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	ChatListModel(QObject *parent = 0);
	~ChatListModel();
public:  
	virtual bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole)const;
	virtual int rowCount(const QModelIndex & parent = QModelIndex())const;
private:
	int                 totalRowCount;
};

#endif // CHATLISTMODEL_H
