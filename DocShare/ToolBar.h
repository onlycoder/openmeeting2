#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QObject>
#include <QWidget>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

class ToolBar :public QWidget
{
	Q_OBJECT

public:
	ToolBar(QWidget *parent);
	~ToolBar();

public:
	QPushButton * m_pBtnShareCursor;
	QPushButton * m_pBtnQuit;
	//上一页
	QPushButton * m_pBtnPrev; 
	QPushButton * m_pBtnNext;
	//显示当前页的序号/总页数
	QLabel      * m_pLblIndex;
};

#endif // TOOLBAR_H
