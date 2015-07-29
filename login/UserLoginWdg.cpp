#include "UserLoginWdg.h"
#include <QFile>
#include <QPixmap>
#include <QSound>
#include "../Room/RoomWdg.h"
#include "../stdafx.h"
#include "ServerAddressSet.h"
#include "../MeetingListModel/Meetinglistmodel.h"
#include "Setting/AppSetting.h"
#include "../eventcenter.h"
UserLoginWdg * UserLoginWdg::m_instance = NULL;;
UserLoginWdg * UserLoginWdg::GetInstance()
{
	if(m_instance == NULL)
	{
		m_instance = new UserLoginWdg();
	}
	return m_instance;
}

UserLoginWdg::UserLoginWdg(QWidget *parent): QWidget(parent)
{
	m_loginType = LOGIN_TYPE_USER;
	ui.setupUi(this);
	QFile qssFile(":/qss/login.qss");
	QString qss;
	qssFile.open(QFile::ReadOnly);
	if(qssFile.isOpen())
	{  
		qss = QString(qssFile.readAll());
		this->setStyleSheet(qss);
		qssFile.close();
	}
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	ui.logoLabel->setPixmap(QPixmap(":/login/login/login_bg1.png"));
	ui.logoLabel->setScaledContents(true);
	/*连接信号槽*/
	connect(ui.countBtn,SIGNAL(clicked()),this,SLOT(countBtnClicked()));
	connect(ui.meetingBtn,SIGNAL(clicked()),this,SLOT(meetingBtnClicked()));
	connect(ui.closeBtn,SIGNAL(clicked()),this,SLOT(close()));
	connect(ui.loginBtn,SIGNAL(clicked()),this,SLOT(loginBtnClicked()));
	
	connect(ui.setBtn,SIGNAL(clicked()),this,SLOT(setBtnClicked()));
	connect(ui.saveCheckBox,SIGNAL(stateChanged(int)),this,SLOT(checkBoxClicked(int)));
	connect(ui.freeCheckBox,SIGNAL(stateChanged(int)),this,SLOT(checkBoxClicked(int)));
	ui.freeCheckBox->setVisible(false);

	/*从网页直接登录*/
	connect(this,SIGNAL(loginFromWebWithUserId()),this,SLOT(countBtnClicked()));
	connect(this,SIGNAL(loginFromWebWithMeetingRoom()),this,SLOT(meetingBtnClicked()));
	connect(this,SIGNAL(loginFromWebToLogin()),this,SLOT(loginBtnClicked()));
	isWeb = false;


	/*initGodsAnyThingBySean();*/
}
void UserLoginWdg::initGodsAnyThingBySean(){

   QPalette countPale;
   QPalette meetingPale;
   countPale.setColor(QPalette::ButtonText,QColor(0, 0, 0));
   meetingPale.setColor(QPalette::ButtonText,QColor(255,255,255));
   ui.countBtn->setPalette(countPale);
   ui.meetingBtn->setPalette(meetingPale);

	QPalette linePale;
	linePale.setColor(QPalette::Text,QColor(255,255,255));

	nikeNameLineEdit = new QLineEdit(this);
	nikeNameLineEdit->setGeometry(140,135,200,60);
	nikeNameLineEdit->setPlaceholderText("请输入您的昵称");
	nikeNameLineEdit->setObjectName("acccountLineEdit");
	nikeNameLineEdit->setPalette(linePale);
	nikeNameLineEdit->setHidden(true);
	connect(nikeNameLineEdit,SIGNAL(editingFinished()),this,SLOT(lineEditeFinish()));

	acccountLineEdit = new QLineEdit(this);
	acccountLineEdit->setGeometry(140,158,200,60);
	acccountLineEdit->setPlaceholderText("请输入您的账号");
	acccountLineEdit->setObjectName("acccountLineEdit");
	acccountLineEdit->setPalette(linePale);

	passWordLineEdit = new QLineEdit(this);
	passWordLineEdit->setGeometry(140,195,200,60);
	passWordLineEdit->setPlaceholderText("请输入您的密码");
	passWordLineEdit->setObjectName("acccountLineEdit");
	passWordLineEdit->setEchoMode(QLineEdit::Password);
	passWordLineEdit->setPalette(linePale);

	connect(passWordLineEdit,SIGNAL(returnPressed()),this,SLOT(loginBtnClicked()));
	addLoadingWidget();


	if (AppSetting::GetInstance()->GetLoginRemberAPWordStatus()  == 2 && AppSetting::GetInstance()->GetLoginStyle() == 1)
	{
		acccountLineEdit->setText(AppSetting::GetInstance()->GetLoginAccount());
		passWordLineEdit->setText(AppSetting::GetInstance()->GetLoginPWord());
		ui.saveCheckBox->setChecked(true);
	}else{
		 ui.saveCheckBox->setChecked(false);
	}
	  
	//web登录
	if (isWeb)
	{
		if (logType.toInt() ==  1)
		{
			//账号登录
			emit loginFromWebWithUserId();
			acccountLineEdit->setHidden(false);
			passWordLineEdit->setHidden(false);
			acccountLineEdit->setText(userId);
			passWordLineEdit->setText(passWord);
			emit loginFromWebToLogin();


		}
		else 
		{
			//会议号
			emit loginFromWebWithMeetingRoom();
			nikeNameLineEdit->setText(userId);
			acccountLineEdit->setText(roomNum);
			passWordLineEdit->setText(passWord);
			emit loginFromWebToLogin();

		}
	}

	QString serverStr  = AppSetting::GetInstance()->GetServerAddress();
	if (serverStr.length() < 2)
	{
		loginAnimation("请点击设置,来设置服务器地址！");
	}
}


void UserLoginWdg::addLoadingWidget(){

	contentWidget = new QWidget(this);
	contentWidget->setGeometry(4,85,442,250);
	loadWidget = new MeetingLoadingWidget(contentWidget);
	contentWidget->setHidden(true);
	connect(loadWidget, SIGNAL(toHiddenLoadWidget()), this, SLOT(getSignalToHiddenLoadWidget()));
	loadWidget->startWithBtnTitleAndTipString(true,"取消","正在登录请稍后...");
}
void UserLoginWdg::mouseMoveEvent(QMouseEvent *event){

	if (mouse_pressed)
	{
		QPoint movePoint = event->globalPos();
		this->move(movePoint - mouse_movePoint);
	}
}

void UserLoginWdg::mousePressEvent(QMouseEvent *event){

	if (event->button() == Qt::LeftButton)
	{
		mouse_pressed = true;
		mouse_movePoint = event->pos();
	}
}

void UserLoginWdg::mouseReleaseEvent(QMouseEvent *event){

	mouse_pressed = false;
}
/************账号登录*****************/
void UserLoginWdg::countBtnClicked(){

	m_loginType = LOGIN_TYPE_USER;

	if (isWeb)
	{
		nikeNameLineEdit->setHidden(true);
		nikeNameLineEdit->clear();
		acccountLineEdit->setGeometry(140,158,200,60);
		passWordLineEdit->setGeometry(140,195,200,60);

	}else{
		nikeNameLineEdit->setHidden(true);
		nikeNameLineEdit->clear();
		acccountLineEdit->setGeometry(140,158,200,60);
		acccountLineEdit->clear();
		acccountLineEdit->setPlaceholderText("请输入您的账号");
		passWordLineEdit->setGeometry(140,195,200,60);
		passWordLineEdit->clear();
	}
    ui.saveCheckBox->setVisible(true);
	AppSetting::GetInstance()->ReadValues();
	if ( AppSetting::GetInstance()->GetLoginStyle() == 1 && AppSetting::GetInstance()->GetLoginRemberAPWordStatus()  == 2 )
	{
		acccountLineEdit->setText(AppSetting::GetInstance()->GetLoginAccount());
		passWordLineEdit->setText(AppSetting::GetInstance()->GetLoginPWord());
		ui.saveCheckBox->setChecked(true);
	}else ui.saveCheckBox->setChecked(false);

	ui.logoLabel->setPixmap(QPixmap(":/login/login/login_bg1.png"));
    ui.countBtn->setChecked(true);
	ui.meetingBtn->setChecked(false);

	QPalette pal1;
	QPalette pal2;
	pal1.setColor(QPalette::ButtonText,QColor(255,255,255));
	pal2.setColor(QPalette::ButtonText,QColor(0, 0, 0));
	ui.meetingBtn->setPalette(pal1);
	ui.countBtn->setPalette(pal2);
	
}
/************会议号登录*****************/
void UserLoginWdg::meetingBtnClicked(){
	m_loginType = LOGIN_TYPE_ROOMNO;
	if (isWeb)
	{
		nikeNameLineEdit->setHidden(false);
		acccountLineEdit->setGeometry(140,170,200,60);
		acccountLineEdit->setPlaceholderText("请输入您会议号");
		passWordLineEdit->setGeometry(140,210,200,60);
	}else{
		nikeNameLineEdit->setHidden(false);
		acccountLineEdit->setGeometry(140,170,200,60);
		acccountLineEdit->clear();
		acccountLineEdit->setPlaceholderText("请输入您会议号");
		passWordLineEdit->setGeometry(140,210,200,60);
		passWordLineEdit->clear();
	}
	ui.saveCheckBox->setVisible(false);
	ui.logoLabel->setPixmap(QPixmap(":/login/login/login_bg2.png"));
	ui.countBtn->setChecked(false);
	ui.meetingBtn->setChecked(true);

	QPalette pal1;
	QPalette pal2;
	pal1.setColor(QPalette::ButtonText,QColor(255,255,255));
	pal2.setColor(QPalette::ButtonText,QColor(0, 0, 0));
	ui.meetingBtn->setPalette(pal2);
	ui.countBtn->setPalette(pal1);

}

/*checkbox 选择*/
void UserLoginWdg::checkBoxClicked(int status){


	if (QCheckBox* box = dynamic_cast<QCheckBox*>(sender())){
		//send_key(btn->whatsThis());
		if (box->text() == "保存密码")
		{
		
			qDebug()<<"保存密码"<<"status"<<status;

			if (m_loginType == LOGIN_TYPE_USER)
			{
				AppSetting::GetInstance()->SetLoginStyle(1);
				if (status == 2)//选中状态
				{
					AppSetting::GetInstance()->SetLoginRemberAPWordStatus(2);

				}else
					AppSetting::GetInstance()->SetLoginRemberAPWordStatus(0);

			}
		
		}
		else
			qDebug()<<"免费体验"<<"status"<<status;

	}

}
/*设置*/
void UserLoginWdg::setBtnClicked(){

	ServerAddressSet *setWdg = new ServerAddressSet();
	setWdg->setAttribute(Qt::WA_ShowModal,true);
	setWdg->show();

}
bool UserLoginWdg::judgeLoadingStatus(LOGIN_TYPE loinType,QString userNme,QString passWord,QString nikeName){


	if (loinType == LOGIN_TYPE_USER )
	{

		if (userNme.length() == 0)
		{
			loginAnimation("请输入用户名!");
			return false;
		}
		if (passWord.length() == 0)
		{

			loginAnimation("密码不能为空!");
			return false;
		}
		return true;

	}else{

		if (nikeName.length() == 0)
		{

			loginAnimation("请输入昵称!");
			return false;
		}
		if (userNme.length() == 0)
		{
			loginAnimation("请输入用户名!");
			return false;
		}
		if (passWord.length() == 0)
		{
			//loginAnimation("密码不能为空!");
			//return false;
		}
		return true;
	}
}

/*登录*/
void UserLoginWdg::loginBtnClicked()
{

	
	QString serverStr  = AppSetting::GetInstance()->GetServerAddress();
	if (serverStr.length() < 2)
	{
		loginAnimation("请点击设置,来设置服务器地址！");
		return;
	}
 	contentWidget->setHidden(false);
	if(g_pMeetingFrame!=NULL)
	{
		if(m_loginType == LOGIN_TYPE_USER)
		{
			QString strAccount = acccountLineEdit->text();
			QString strPassword = passWordLineEdit->text();
			AppSetting::GetInstance()->SetLoginAccount(strAccount);
			AppSetting::GetInstance()->SetLoginPWord(strPassword);
			if (judgeLoadingStatus(m_loginType,strAccount,strPassword,passWordLineEdit->text()))
			{
				g_pMeetingFrame->Login(strAccount.toLatin1().data(),
					strPassword.toLatin1().data(),0);
			}
			else
				contentWidget->setHidden(true);
		}
		else
		{
			QString strNickName = nikeNameLineEdit->text();
			QString strAccount = acccountLineEdit->text();
			QString strPassword = passWordLineEdit->text();

			if (judgeLoadingStatus(m_loginType,strAccount,strPassword,strNickName))
			{
				g_pMeetingFrame->LoginWithRoomNo(strNickName.toUtf8().data(),
					strPassword.toLatin1().data(),0,strAccount.toInt());
			}
			else
			contentWidget->setHidden(true);
			
		}
	}
		AppSetting::GetInstance()->Save();
	

}


UserLoginWdg::~UserLoginWdg()
{
	

}

void UserLoginWdg::OnClose()
{
	hide();
	if(g_pMeetingFrame)
	{
		g_pMeetingFrame->Logout();
		g_pMeetingFrame->Release();
	}
	qApp->quit();
}
//点击取消登陆回调
void UserLoginWdg::getSignalToHiddenLoadWidget(){
	contentWidget->setHidden(true);
	//this->hide();
	//MeetingListWdg::GetTableViewInstance()->show();
}


//登录失败
void UserLoginWdg::OnLoginFailed()
{

	contentWidget->setHidden(true);

	loginAnimation("用户名或密码错误，请重新输入！");
	if (isWeb)
	{
		isWeb = false;
	}

}

//登录成功
void UserLoginWdg::OnLoginSuccess()
{
	qDebug()<<QString("登录成功");
	if(m_loginType == LOGIN_TYPE_USER)
	{
		contentWidget->setHidden(true);
		if(g_pMeetingFrame!=NULL)
		{
			if (isWeb)
			{
				this->hide();
				g_pMeetingFrame->EnterRoom(roomNum.toUInt());
				EventCenter::GetInstance()->setRoomId(roomNum.toUInt());
				RoomWdg::GetInstance()->show();
				
			}
			else
			{
				g_pMeetingFrame->GetRoomList();
				this->hide();
				MeetingListWdg::GetTableViewInstance()->show();
			}
		}
		
	}
	else
	{

		this->hide();
		if(g_pMeetingFrame!=NULL)
		{
			QString strAccount = acccountLineEdit->text();
			g_pMeetingFrame->EnterRoom(strAccount.toUInt());
			EventCenter::GetInstance()->setRoomId(strAccount.toUInt());
		}
		RoomWdg::GetInstance()->show();
	}
}

void UserLoginWdg::OnGetRoomList(int total)
{
	MeetingListModel::GetInstance()->insertRows(0,total);
}

void UserLoginWdg::loginAnimation(QString animationStr){
	animation = new SeanAnimation(this);
	animation->parentWdg = this;
	animation->setTiptitleWithString(animationStr);
	animation->setAttribute(Qt::WA_ShowModal,true);
	animation->show();
}
