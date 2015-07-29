#ifndef SMALLVIDEO_H
#define SMALLVIDEO_H

#include <QWidget>
#include <QMutex>
#include <QTimer>
#include <QMenu>
#include <QActionGroup>
#include "../stdafx.h"
class VideoWin : public QWidget,public IVideoWin
{
	Q_OBJECT

public:
	VideoWin(QWidget *parent = 0);
	~VideoWin();
public:
	virtual HWND GetVideoHWND();
	virtual void SetUserID(unsigned __int64  ulUserId);
	virtual uint64_t GetUserID(){return m_userID;};
	virtual bool isFree();
	virtual void SetFree(bool bFree );
	void SetPos(int x,int y,int x_space,int y_space);
	virtual void SetHidden(bool bHide);
	void DeliveData(int w,int h,unsigned char* pData);
	virtual void StartRender();
	virtual void StopRender();
	//设置视频上显示用户名
	virtual void SetUserName(char* strUserName);

signals:
	void EventDBClick(qint64,bool);
protected:
	void paintEvent(QPaintEvent *event);
	void mouseDoubleClickEvent ( QMouseEvent * event );
	void mousePressEvent ( QMouseEvent * event );
private slots:
		void refreshVideo();
		void onShowUserName();
		void onKeepAspect();
		void onFitWindow();
private:
	bool m_bFree; 
	bool m_bShowUserName;
	int64_t m_userID;
	int m_nWidth,m_nHeight;
	unsigned char * m_pVieoData;
	QMutex m_Mutex;
	QTimer* m_Timer;
	char m_strUserName[64];
	QMenu *m_pMenu;
	QAction * menuKeepAspect;
	QAction * menuFitWindow;
	QAction * menuShowUserName;
	QActionGroup *modeGroup;
public:
	int m_x;
	int m_y;
	int m_xSpace;
	int m_ySpace;
	bool m_bKeepAspect;
	bool m_bFullScreen;
};

#endif // SMALLVIDEO_H
