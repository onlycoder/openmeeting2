#ifndef CLISTITEMDELEGATE_H
#define CLISTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>
class CListItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	CListItemDelegate(QObject *parent = 0);
	~CListItemDelegate();
public:
	void SetParentUI(QObject* apObjUI);
protected:
	virtual bool eventFilter(QObject *object, QEvent *event);
	virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
private:
	bool            m_bMouseMove;
	int           m_bHoverUserID;
	QObject*        m_objParentUI;
	bool			m_bShow;
};

#endif // CLISTITEMDELEGATE_H
