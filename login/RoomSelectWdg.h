#ifndef ROOMSELECTWDG_H
#define ROOMSELECTWDG_H

#include <QWidget>
#include "ui_RoomSelectWdg.h"

class RoomSelectWdg : public QWidget
{
	Q_OBJECT

public:
	RoomSelectWdg(QWidget *parent = 0);
	~RoomSelectWdg();

private:
	Ui::RoomSelectWdg ui;
};

#endif // ROOMSELECTWDG_H
