#ifndef WDGNETINFO_H
#define WDGNETINFO_H

#include <QObject>
#include <QWidget>
class WdgNetInfo : public QWidget
{
	Q_OBJECT

public:
	WdgNetInfo(QWidget *parent = 0);
	~WdgNetInfo();

private:
	static WdgNetInfo * m_instance;
public:
	static WdgNetInfo * GetInstance();

	public slots:
		void OnNetInfo(int delay,int uploadLost,int downloadLost);
protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void enterEvent ( QEvent * event );
	void leaveEvent ( QEvent * event );
private:
	int m_delay;            //网络延时，毫秒
	int m_uploadLost;       //上行丢包，百分数
	int m_downloadLost;     //下行丢包
};

#endif // WDGNETINFO_H
