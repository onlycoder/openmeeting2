#include "DlgApplyAdmin.h"
#include "../stdafx.h"
#include "RoomWdg.h"
DlgApplyAdmin*DlgApplyAdmin::m_instance = NULL;
DlgApplyAdmin*DlgApplyAdmin::GetInstance()
{
	if(m_instance == NULL)
	{
		m_instance = new DlgApplyAdmin();
	}
	return m_instance;
}

DlgApplyAdmin::DlgApplyAdmin(QWidget *parent,CommonWndFlags WndFlag)
	: CCommonWidget(parent, WndFlag, Qt::Dialog)
{
	setupUi(this);
	//setModal(true);
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	CreateSystemTitle(hBoxTitle);
	CreateSystemButton(hBoxSysBtn,enum_GGWndCloseButtonHint);
	SetTitle(L"申请主持人");
	connect(btnOK,SIGNAL(clicked()),this,SLOT(OnOK()));
	connect(btnCancel,SIGNAL(clicked()),this,SLOT(OnCancel()));
	connect(this, SIGNAL(OnClose()), this, SLOT(OnClose()));
}
DlgApplyAdmin::~DlgApplyAdmin()
{

}

void DlgApplyAdmin::OnClose()
{
	close();
}
void DlgApplyAdmin::OnOK()
{
	if(editPassword->text()!="")
	{
		if(editPassword->text() == QString::fromUtf8(g_pMeetingFrame->GetAdminPassword()))
		{
			g_pMeetingFrame->SetAsAdmin(0,true);
			close();
		}
		else
		{
			lblTip->setText("密码错误");
		}
		
	}
	else
	{
		lblTip->setText("密码不能为空");
	}
}
void DlgApplyAdmin::OnCancel()
{
	close();
}

bool DlgApplyAdmin::IsAdmin()
{
	return m_isAdmin;
}