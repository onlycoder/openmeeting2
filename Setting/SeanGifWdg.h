#ifndef SEANGIFWDG_H
#define SEANGIFWDG_H

#include <QWidget>
#include "ui_SeanGifWdg.h"
#include "SeanButton.h"



class SeanGifWdg : public QWidget
{
	Q_OBJECT

public:
	SeanGifWdg(QWidget *parent = 0);
	~SeanGifWdg();

	static SeanGifWdg *g_Instance;
	static SeanGifWdg *GetInstance();

	
private:
	Ui::SeanGifWdg ui;

	public slots:
		void gifBtnClickedWithBtntag();
signals:
		void sendGifTagWithNum(unsigned long long num);
		void EventEnter();
		void EventLeave();
protected:
	void enterEvent ( QEvent * event );
	void leaveEvent ( QEvent * event );
};

#endif // SEANGIFWDG_H
