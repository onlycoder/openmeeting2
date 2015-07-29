#ifndef SEANTESTWDG_H
#define SEANTESTWDG_H

#include <QWidget>
#include "ui_SeanTestWdg.h"
#include <QMouseEvent>
#include <QTimer>

class SeanTestWdg : public QWidget
{
	Q_OBJECT

public:
	SeanTestWdg(QWidget *parent = 0);
	~SeanTestWdg();
	Ui::SeanTestWdg ui;
	void JustShowLabel();
	void ShowAll();
	void ShowCloseDocBtn(bool bShow);
private:
	
	QTimer *timer;
	bool penBtnChecked;
protected:
	
	void mouseMoveEvent(QMouseEvent* event);
	void enterEvent ( QEvent * event );
	void leaveEvent ( QEvent * event );
signals:
	void signalOfEnterTestWdg();
	void signalOfLeaveTestWdg();
public slots:
    void setPenBntStats();
	void timeoutToHiddenToolWdg();
};

#endif // SEANTESTWDG_H
