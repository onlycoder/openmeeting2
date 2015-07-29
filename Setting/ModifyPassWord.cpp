#include "ModifyPassWord.h"
#include <QDebug>
#include <QRegExp>
#include "SeanAnimation.h"

ModifyPassWord * ModifyPassWord::m_instance = NULL;
ModifyPassWord *ModifyPassWord::GetInstance(){

	if (m_instance == NULL)
	{
		m_instance = new ModifyPassWord();
	}
	return m_instance;
}

ModifyPassWord::ModifyPassWord(QWidget *parent,CommonWndFlags WndFlag)
	: CCommonWidget(parent, WndFlag, Qt::Dialog)
	
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	CreateSystemTitle(ui.hBoxTitle);
	CreateSystemButton(ui.hBoxSysBtn,enum_GGWndCloseButtonHint);
	SetTitle(L"申请主持人");
	connect(ui.btnOk,SIGNAL(clicked()),this,SLOT(OnOK()));
	connect(ui.btnCancel,SIGNAL(clicked()),this,SLOT(OnCancel()));
	connect(this, SIGNAL(OnClose()), this, SLOT(OnClose()));
	
	ui.lastPassword->clear();
	ui.newPassword->clear();
	ui.againPassword->clear();

}

ModifyPassWord::~ModifyPassWord()
{

}

void ModifyPassWord::judeLastPasswordIsRight(){

}

void ModifyPassWord::judeNewPasswordFormatIsRight(){

	
}

void ModifyPassWord::judePasswordKeepSame(){

	
}
//关闭
void ModifyPassWord::OnClose()
{
	close();
}
//确定
void ModifyPassWord::OnOK()
{


	QRegExp exp1("[a-z][A-Z][0-9]");  
	bool valid=exp1.isValid();//返回true  

	QRegExp rx;
	rx.setPatternSyntax(QRegExp::RegExp);
	rx.setCaseSensitivity(Qt::CaseSensitive); //大小写敏感
	rx.setPattern(QString("^[A-Za-z0-9]+$")); //匹配所有大小写字母和数字组成的字符串

	QString lastWord = ui.lastPassword->text();
	QString newWord = ui.newPassword->text();
	QString aginWord = ui.againPassword->text();

	if (lastWord.length()==0)
	{
		startAnimation("密码不能为空");
		return;
	}


	//返回的值为false，因为s中含有@字符
	if (newWord.length()==0)
	{
		startAnimation("新密码不能为空");
		return;
	}
	else if(!rx.exactMatch(newWord))
	{
		startAnimation("新密码输入有误 必须是字母或数字");
		ui.againPassword->clear();
		return;
		
	}

	if(aginWord.length() == 0){
		startAnimation("请再次输入密码");
		return;
	}else if ( newWord.compare(aginWord)!= 0)
	{
		startAnimation("两次输入的密码不一致，请重新输入！");
		ui.againPassword->clear();
		return;
	}


	close();
}
//取消
void ModifyPassWord::OnCancel()
{
	close();
}

void ModifyPassWord::startAnimation(QString str){

	SeanAnimation *animation = new SeanAnimation(this);
	animation->parentWdg = this;
	animation->setTiptitleWithString(str);
	animation->setAttribute(Qt::WA_ShowModal,true);
	animation->show();
	
}
