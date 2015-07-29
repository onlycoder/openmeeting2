#ifndef MEETINGROOMWORDWDG_H
#define MEETINGROOMWORDWDG_H

#include <QWidget>
#include "ui_MeetingRoomWordWdg.h"
#include "ccommonwidget.h"
#include "../stdafx.h"
class MeetingRoomWordWdg : public CCommonWidget
{
	Q_OBJECT

public:
	MeetingRoomWordWdg(QWidget *parent = 0,CommonWndFlags WndFlag = enum_GGWndTitlebarHint | enum_GGWndCloseButtonHint);
	~MeetingRoomWordWdg();

	static MeetingRoomWordWdg *r_instance;
	static MeetingRoomWordWdg *GetInstance();
	int  indexRow;

private:
	Ui::MeetingRoomWordWdg ui;
	
	public slots:
		void OnOK();
		void OnCancel();
		void OnClose();
		
signals:
		void toEnterRoomWithRoomId(uint32_t roomId);
};

#endif // MEETINGROOMWORDWDG_H
