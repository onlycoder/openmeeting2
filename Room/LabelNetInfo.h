#ifndef LABLENETINFO_H
#define LABLENETINFO_H

#include <QObject>
#include <QWidget>
#include <QLabel>
class LabelNetInfo : public QLabel
{
	Q_OBJECT

public:
	LabelNetInfo(QWidget *parent = 0);
	~LabelNetInfo();
signals:
	void EventEnter();
	void EventLeave();

protected:
	void enterEvent ( QEvent * event );
	void leaveEvent ( QEvent * event );
private:
	qint64 m_lastEventTime;
};

#endif // WDGNETINFO_H
