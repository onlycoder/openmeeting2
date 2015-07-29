#ifndef CHATLISTMODELDELEGATE_H
#define CHATLISTMODELDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>
#include "LeftWdg.h"
#include <QTextCodec>
class ChatListModelDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	ChatListModelDelegate(QObject *parent = 0);
	~ChatListModelDelegate();

public:
	void SetParentUI(QObject* apObjUI);
public:
	virtual bool eventFilter(QObject *object, QEvent *event);
	virtual void paint(QPainter *painter,const QStyleOptionViewItem &option,const QModelIndex &index) const;
private:
	QObject*        parentUI;
	
};

#endif // CHATLISTMODELDELEGATE_H
