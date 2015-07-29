#ifndef SEANPROGRESSBAR_H
#define SEANPROGRESSBAR_H

#include <QWidget>

class SeanProgressBar : public QWidget
{
	Q_OBJECT

public:
	SeanProgressBar(QWidget *parent = 0);
	~SeanProgressBar();

private:
	int vValue;
public slots:
	void getSoundVulume(int voiceValue);

protected:
	void paintEvent(QPaintEvent *event);

private:
	
};

#endif // SEANPROGRESSBAR_H
