#include "AppSetting.h"
#include <QDebug>

AppSetting*AppSetting::m_instance = NULL;
AppSetting*AppSetting::GetInstance()
{
	if(m_instance == NULL)
	{
		m_instance = new AppSetting();

		QDir *inDoc = new QDir;
		QString path =	QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
		path += "//VideoMeeting";
		bool exist = inDoc->exists(path);
		if(exist)
			qDebug()<<"文件夹已经存在";
		else
		{
			bool ok = inDoc->mkdir(path);
			if( ok )
				qDebug()<<"文件夹创建成功";
		}
		m_instance->ReadValues();

	}
	return m_instance;
}


AppSetting::AppSetting(void)
{
	m_micIndex = 0;
	m_speakerIndex = 0;
	m_cameraIndex = 0;
}


AppSetting::~AppSetting(void)
{

}

int AppSetting::GetMicIndex()
{
	return m_micIndex;
}

void AppSetting::SetMicIndex(int index)
{
	m_micIndex = index;
}

int AppSetting::GetSpeakerIndex()
{
	return m_speakerIndex;
}

void AppSetting::SetSpeakerIndex(int index)
{
	m_speakerIndex = index;
}

int AppSetting::GetCameraIndex()
{
	return m_cameraIndex;
}

void AppSetting::SetCameraIndex(int index)
{
	m_cameraIndex = index;
}
QString AppSetting::GetServerAddress(){
	return serverUrl;
}
void AppSetting::SetServerAddress(QString serverAddress){
	serverUrl =  serverAddress;
}

QString AppSetting::GetLoginAccount(){
	return loginAccount;
}
void AppSetting::SetLoginAccount(QString logAccount){
	loginAccount = logAccount;
}

QString AppSetting::GetLoginPWord(){
	return loginPWord;
}
void AppSetting::SetLoginPWord(QString logPWord){
	loginPWord = logPWord;
}

QString AppSetting::GetLoginNikeName(){
	return loginNickName;
}
void AppSetting::SetLoginNikeName(QString logNikeName){

	loginNickName = logNikeName;
}

int AppSetting::GetLoginRemberAPWordStatus(){
	return remberAPWord;
}
void AppSetting::SetLoginRemberAPWordStatus(int isTrue){
	remberAPWord = isTrue;
}


int AppSetting::GetLoginStyle(){
	return isAccount;
}
void AppSetting::SetLoginStyle(int logStyle){
	isAccount = logStyle;
}


void AppSetting::Save()
{
	// 当前目录的INI文件
	QDir *inDoc = new QDir;
	QString path =	QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
	path += "//VideoMeeting";
	bool exist = inDoc->exists(path);
	if (exist)
	{
		QSettings settings(path+"/AppConfig.ini", QSettings::IniFormat);
		settings.beginGroup("Device");
		settings.setValue("Mic",this->m_micIndex);
		settings.setValue("Speaker",this->m_speakerIndex);
		settings.setValue("Camera",this->m_cameraIndex);
		settings.setValue("serverUrl",this->serverUrl);
		settings.setValue("loginAccount",this->loginAccount);
		settings.setValue("loginPWord",this->loginPWord);
		settings.setValue("loginNickName",this->loginNickName);
		settings.setValue("remberAPWord",this->remberAPWord);
		settings.setValue("isAccount",this->isAccount);
		settings.endGroup();
	}

	/*QStringList QStandardPaths::standardLocations(StandardLocation type);*/
	//qDebug()<<"QStringList = "<<path;
	
}

void AppSetting::ReadValues()
{
	QDir *inDoc = new QDir;
	QString path =	QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
	path += "//VideoMeeting";
	bool exist = inDoc->exists(path);
	if (exist)
	{
		QSettings settings(path+"/AppConfig.ini", QSettings::IniFormat);
		m_micIndex = settings.value("Device/Mic",0).toInt();
		m_speakerIndex = settings.value("Device/Speaker",0).toInt();
		m_cameraIndex = settings.value("Device/Camera",0).toInt();
		serverUrl = settings.value("Device/serverUrl","127.0.0.1:7082").toString();
		loginAccount = settings.value("Device/loginAccount",0).toString();
		loginPWord = settings.value("Device/loginPWord",0).toString();
		loginNickName = settings.value("Device/loginNickName",0).toString();
		remberAPWord = settings.value("Device/remberAPWord",0).toInt();
		isAccount = settings.value("Device/isAccount",0).toInt();
	}
	
}
