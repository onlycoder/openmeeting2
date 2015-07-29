#ifndef MEETINGLOADINGWIDGET_H
#define MEETINGLOADINGWIDGET_H

#include <QWidget>
#include <QLabel> 
#include <QTimer>
#include <QString>
#include <QPixmap>
#include <QVBoxLayout>
#include <QPushButton>


class MeetingLoadingWidget : public QWidget
{
	Q_OBJECT

public:
	MeetingLoadingWidget(QWidget *parent = 0);
	~MeetingLoadingWidget();


public:
	static MeetingLoadingWidget *load_instance;
	static MeetingLoadingWidget *GetLoadInstance();

private:
	
	QLabel *backLabel;
	QLabel *loadingLabel;
	QLabel *tipLabel;
	QPushButton *cancelBtn;
	QTimer *timer;
	

	int index;

signals:

	void toHiddenLoadWidget();

public:
	void startWithBtnTitleAndTipString(bool is_start,QString btnStr,QString tipStr);

	public slots:
		void updatePixmap();
		void cancelBtnClicked();
	


};

#endif // MEETINGLOADINGWIDGET_H
