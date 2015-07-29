#include "eventcenter.h"
#include <QtGui/QApplication>
#include <QTextCodec>
#include <QFile>
#include "login/UserLoginWdg.h"
#include "../stdafx.h"
#include <iostream>
#include <QMessageBox>
#include <QTranslator>
#include <Windows.h>
#include <process.h>
#include "CrashRpt.h" // Include CrashRpt header

LPVOID lpvState = NULL; // Not used, deprecated

int main(int argc, char *argv[])
{
	CR_INSTALL_INFO info;
	memset(&info, 0, sizeof(CR_INSTALL_INFO));
	info.cb = sizeof(CR_INSTALL_INFO);
	info.pszAppName = L"open meeting2";
	info.pszAppVersion = L"16";
	info.pszUrl = L"http://www.onlycoder.net/meeting/index.php/Home/Index/uploadCrash";
	
	int nInstResult = crInstall(&info);            
	
	if(nInstResult!=0)
	{
		TCHAR buff[256];
		crGetLastErrorMsg(buff, 256); // Get last error
		//_tprintf(_T("%s\n"), buff); // and output it to the screen
		return FALSE;
	}
	crAddScreenshot2(CR_AS_MAIN_WINDOW|CR_AS_USE_JPEG_FORMAT, 95);

	QApplication a(argc, argv);
	QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
	QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
	QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
	QTranslator translator;
	translator.load(":/qt_zh_CN.qm");
	a.installTranslator(&translator);
	QFile qssFile(":/qss/style");
	QString qss;
	qssFile.open(QFile::ReadOnly);
	if(qssFile.isOpen())
	{  
		qss = QString(qssFile.readAll());
		qApp->setStyleSheet(qss);
		qssFile.close();
	}

	if (argc == 1)
	{
		UserLoginWdg::GetInstance()->initGodsAnyThingBySean();
		EventCenter w;
		return a.exec();
	}
	else{
		EventCenter w;
		if(strlen(argv[1])<24)
		{
			QMessageBox message(QMessageBox::NoIcon, "提示","参数太短"); 
			return message.exec();
		}
		QString strUrl = QString(argv[1]+11);
		QByteArray ba =  QByteArray::fromBase64(QByteArray(strUrl.toUtf8()));//base64解码 解密
		QString qStr = QString::fromUtf8(ba);

		QString tipStr;
		QStringList list;
		list = qStr.split("&");

		if (list.count() != 6)
		{
			tipStr = QString("数据格式有误:%1").arg(argv[1]);
			QMessageBox message(QMessageBox::NoIcon, "提示",tipStr); 
			return message.exec();
		}

		UserLoginWdg::GetInstance()->logType = qStr.section('&',1,1);//类型区分 账号 会议号登录
		UserLoginWdg::GetInstance()->roomNum = qStr.section('&',2,2);//房间号
		UserLoginWdg::GetInstance()->userId = qStr.section('&',3,3);//  用户ID 或 用户名
		UserLoginWdg::GetInstance()->passWord = qStr.section('&',4,4);// 密码
		UserLoginWdg::GetInstance()->isWeb = true;
		UserLoginWdg::GetInstance()->initGodsAnyThingBySean();
		a.exec();
		int nUninstRes = crUninstall();
		return nUninstRes;
	}
}
