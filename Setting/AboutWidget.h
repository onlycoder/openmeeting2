#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include <QWidget>
#include "ui_AboutWidget.h"
#include <QMouseEvent>
#include <QPoint>

class AboutWidget : public QWidget
{
	Q_OBJECT

public:
	AboutWidget(QWidget *parent = 0);
	~AboutWidget();
	static AboutWidget *a_instance;
	static AboutWidget *GetInstance();

private:
	Ui::AboutWidget ui;

	bool mouse_pressed;
	QPoint mouse_movePoint;

	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event); 
	

};

#endif // ABOUTWIDGET_H
