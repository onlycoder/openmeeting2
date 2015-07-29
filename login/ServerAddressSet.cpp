#include "ServerAddressSet.h"
#include <QFile>
#include "Setting/AppSetting.h"
#include "../Room/RoomWdg.h"
#include "../eventcenter.h"
#include "../stdafx.h"
ServerAddressSet *ServerAddressSet::s_instance = NULL;
ServerAddressSet *ServerAddressSet::GetServerSetInstance(){

	if(s_instance == NULL)
		s_instance = new ServerAddressSet();

	return s_instance;

}


ServerAddressSet::ServerAddressSet(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QFile qssFile(":/qss/setting.qss");
	QString qss;
	qssFile.open(QFile::ReadOnly);
	if(qssFile.isOpen())
	{  
		qss = QString(qssFile.readAll());
		this->setStyleSheet(qss);
		qssFile.close();
	}
 	setWindowFlags(Qt::FramelessWindowHint);//这个是widget的标题栏和边框去掉
 	setAttribute(Qt::WA_TranslucentBackground);
	
	ui.logoLabel->setPixmap(QPixmap(":/login/login/logo.png"));

    ui.serverLineEdit->setText(AppSetting::GetInstance()->GetServerAddress());
	connect(ui.closeBtn,SIGNAL(clicked()),this,SLOT(close()));
	connect(ui.queDingBtn,SIGNAL(clicked()),this,SLOT(serverQueDingBtnClicked()));
	connect(ui.serverLineEdit,SIGNAL(returnPressed()),this,SLOT(serverQueDingBtnClicked()));
	connect(ui.cancelBtn,SIGNAL(clicked()),this,SLOT(serverCancelBtnClicked()));
}
/*确认*/
void ServerAddressSet::serverQueDingBtnClicked(){

	AppSetting::GetInstance()->SetServerAddress(ui.serverLineEdit->text());
	AppSetting::GetInstance()->Save();
	if(g_pMeetingFrame!=NULL)
	{
		g_pMeetingFrame->Release();
		CreateMeetingFrame(&g_pMeetingFrame,EventCenter::GetInstance(),(HWND)RoomWdg::GetInstance()->winId(),
			EventCenter::GetInstance(),AppSetting::GetInstance()->GetServerAddress().toStdString());
	}
	this->close();
}
/*取消*/
void ServerAddressSet::serverCancelBtnClicked()
{
	this->close();
}




ServerAddressSet::~ServerAddressSet()
{

}
