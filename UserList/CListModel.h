#ifndef CLISTMODEL_H
#define CLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>
class CListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	CListModel(QObject *parent = 0);
	~CListModel();
public:  
	virtual bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole)const;
	virtual int rowCount(const QModelIndex & parent = QModelIndex())const;
private:
	int                 m_iRowCount;
};

#endif // CLISTMODEL_H
