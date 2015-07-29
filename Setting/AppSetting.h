#pragma once
#include <QString>
#include <QSettings>
#include <QDesktopServices>  
#include <QDir>


//QStandardPaths
class AppSetting
{
public:
	AppSetting(void);
	~AppSetting(void);
private:
	static AppSetting* m_instance;
public:
	static AppSetting* GetInstance();
public:
	 void Save();
	 void ReadValues();
	 int GetMicIndex();
	 void SetMicIndex(int index);
	 int GetSpeakerIndex();
	 void SetSpeakerIndex(int index);
	 int GetCameraIndex();
	 void SetCameraIndex(int index);
	 QString GetServerAddress();
	 void SetServerAddress(QString serverAddress);

	 QString GetLoginAccount();
	 void SetLoginAccount(QString logAccount);
	 QString GetLoginPWord();
	 void SetLoginPWord(QString logPWord);
	 QString GetLoginNikeName();
	 void SetLoginNikeName(QString logNikeName);

	 int GetLoginRemberAPWordStatus();
	 void SetLoginRemberAPWordStatus(int isTrue);

     int GetLoginStyle();
     void SetLoginStyle(int logStyle);


private:
	 int m_micIndex;  //麦克风
	 int m_speakerIndex; //声音输出设备，音箱，耳机之类
	 int m_cameraIndex;  //摄像头
	 QString serverUrl; //设置服务器地址

	 QString loginAccount; //登录账号
	 QString loginPWord;//登录密码
	 QString loginNickName;//登录昵称

	 int remberAPWord;//账号登录是否记住密码
   
     int isAccount;//是否是账号登录

	// QDir *inDoc;
	
};

