#ifndef MODIFYPASSWORD_H
#define MODIFYPASSWORD_H

#include "ccommonwidget.h"
#include "ui_ModifyPassWord.h"
#include <QTimer>

class ModifyPassWord : public CCommonWidget, public Ui::ModifyPassWord
{
	Q_OBJECT

public:
	ModifyPassWord(QWidget *parent = 0,CommonWndFlags WndFlag = enum_GGWndTitlebarHint | enum_GGWndCloseButtonHint);
	~ModifyPassWord();
public:

	static ModifyPassWord *m_instance;
	static ModifyPassWord  *GetInstance();
	void startAnimation(QString str);
private:
	Ui::ModifyPassWord ui;

	QTimer *timer;
	public slots:
		void OnOK();
		void OnCancel();
		void OnClose();

		void judeLastPasswordIsRight();
		void judeNewPasswordFormatIsRight();
		void judePasswordKeepSame();
		
};

#endif // MODIFYPASSWORD_H
