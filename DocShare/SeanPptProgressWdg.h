#ifndef SEANPPTPROGRESSWDG_H
#define SEANPPTPROGRESSWDG_H

#include <QWidget>
#include "ui_SeanPptProgressWdg.h"
#include <QPixmap>
#include <QTimer>


class SeanPptProgressWdg : public QWidget
{
	Q_OBJECT

public:
	SeanPptProgressWdg(QWidget *parent = 0);
	~SeanPptProgressWdg();
	void updateProgressWithFloat(QString uploadStr);
     Ui::SeanPptProgressWdg ui;
private:
	

	QTimer *timer;
	int index;

	public slots:
		void closeWidget();
		void updatePixmap();
};

#endif // SEANPPTPROGRESSWDG_H
