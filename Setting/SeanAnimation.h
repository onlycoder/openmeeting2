#ifndef SEANANIMATION_H
#define SEANANIMATION_H

#include <QWidget>
#include "ui_SeanAnimation.h"
#include <QTimer>
#include <QPropertyAnimation>

class SeanAnimation : public QWidget
{
	Q_OBJECT

public:
	SeanAnimation(QWidget *parent = 0);
	~SeanAnimation();

	QWidget *parentWdg;
	bool isTrue;
public:
	static SeanAnimation *a_instance;
	static SeanAnimation *GetAnimationInstance();

	void initAnimationBySean();
	void setTiptitleWithString(QString tipStr);
	
	public slots:
		void timerUpDate();
	
private:
	Ui::SeanAnimation ui;
	QTimer *timer;


	int getHeigh;
	
	

};

#endif // SEANANIMATION_H
