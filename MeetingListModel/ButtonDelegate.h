#ifndef BUTTONDELEGATE_H
#define BUTTONDELEGATE_H

#include <QPushButton>
#include <QStyledItemDelegate>
#include <QEvent>
#include <QObject>

class ButtonDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	ButtonDelegate(QObject *parent = 0);
	~ButtonDelegate();
public:
protected:
	virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
	virtual bool eventFilter(QObject *object, QEvent *event);
 
private:
	void goinBtnPressed(QObject *object, QEvent *event);

signals:
	void sendTableViewNumberRowAndWorth(int row,int isBool);	
};

#endif // BUTTONDELEGATE_H
