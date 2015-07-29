#ifndef SEANTABLEVIEWDELEGATE_H
#define SEANTABLEVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include <QTableView>
#include <QPen>
#include <QSize>

class SeanTableViewDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	SeanTableViewDelegate(QTableView* tableView);
	~SeanTableViewDelegate();
protected:
	void paint(QPainter* painter,const QStyleOptionViewItem& option,const QModelIndex& index) const;
	//QSize sizeHint( const QStyleOptionViewItem &option,  const QModelIndex &index ) const;  
	
private:
	QPen         pen;
	QTableView*  view;
private:
	
};

#endif // SEANTABLEVIEWDELEGATE_H
