#ifndef MEETINGDETAILDELEGATE_H
#define MEETINGDETAILDELEGATE_H

#include <QStyledItemDelegate>
#include "../stdafx.h"
#include <QPainter>
#include <QDebug>

class MeetingDetailDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	MeetingDetailDelegate(QObject *parent = 0);
	~MeetingDetailDelegate();
protected:
	virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
private:
	
};

#endif // MEETINGDETAILDELEGATE_H
