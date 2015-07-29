#ifndef DLGAPPLYADMIN_H
#define DLGAPPLYADMIN_H

#include <QDialog>
#include "ui_DlgApplyAdmin.h"
#include "ccommonwidget.h"
class DlgApplyAdmin : public CCommonWidget, public Ui::DlgApplyAdmin
{
	Q_OBJECT

public:
	DlgApplyAdmin(QWidget *parent = 0,CommonWndFlags WndFlag = enum_GGWndTitlebarHint | enum_GGWndCloseButtonHint);
	~DlgApplyAdmin();

private:
	static DlgApplyAdmin* m_instance;
public:
	static DlgApplyAdmin* GetInstance();
	bool IsAdmin();
	public slots:
		void OnOK();
		void OnCancel();
		void OnClose();
private:
	bool m_isAdmin;
};

#endif // DLGAPPLYADMIN_H
