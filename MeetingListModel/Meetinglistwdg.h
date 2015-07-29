#ifndef MEETINGLISTWDG_H
#define MEETINGLISTWDG_H

#include <QWidget>
#include "ui_Meetinglistwdg.h"

#include "MeetingListModel/Meetinglistmodel.h"
#include "MeetingListModel/ButtonDelegate.h"
#include <QMouseEvent>
#include <QPoint>
#include "MeetingRoomWordWdg.h"
#include "MeetingDetailDelegate.h"
class MeetingListWdg : public QWidget
{
	Q_OBJECT

public:
	MeetingListWdg(QWidget *parent = 0);
	~MeetingListWdg();
	void InitDeviceList();

public:
	static MeetingListWdg *t_instance;
	static MeetingListWdg *GetTableViewInstance();

private:
	QPoint mouse_movePoint;
	bool mouse_pressed;
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
public slots:
		void getFromButtonDelegateWithRowAndBool(int row,int isBool);
		void getRoomIdToEnterRoom(uint32_t roomId);
		
private:
	Ui::MeetingListWdg ui;
};

#endif // MEETINGLISTWDG_H
