#include "SeanTableViewDelegate.h"
#include <QPainter>
SeanTableViewDelegate::~SeanTableViewDelegate()
{

}
SeanTableViewDelegate::SeanTableViewDelegate(QTableView* tableView)
{
	int gridHint = tableView->style()->styleHint(QStyle::SH_Table_GridLineColor, new QStyleOptionViewItemV4());
	QColor gridColor = static_cast<QRgb>(gridHint);
	pen = QPen(gridColor, 0, tableView->gridStyle());
	view = tableView;
	
}

void SeanTableViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,const QModelIndex& index)const
{

	view->setRowHeight(index.row(),60);
	QStyleOptionViewItem itemOption(option);
	if(itemOption.state & QStyle::State_HasFocus)
		itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
	QStyledItemDelegate::paint(painter,itemOption,index);
	QPen oldPen = painter->pen();
	painter->setPen(pen);
	painter->setPen(oldPen);
}

//  QSize SeanTableViewDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index )const
// { 
// 		return QSize(450, 100);  
// 
// }