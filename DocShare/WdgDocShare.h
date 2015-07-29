#ifndef WDGDOCSHARE_H
#define WDGDOCSHARE_H

#include <QWidget>


#include "OcImageLoader.h"
#include "SeanPptProgressWdg.h"
#include "SeanTestWdg.h"
#include "ConvertDocThread.h"
#include <QTimer>
#include <QThread>


class WdgDocShare : public QWidget
{
	Q_OBJECT
public:
	WdgDocShare(QWidget *parent = 0);
	~WdgDocShare();
protected:
	virtual void resizeEvent( QResizeEvent * event );

	//virtual bool	event ( QEvent * event );
public:
	void DisplayLocalDoc(QString strUrl,int pageTotal);
	void DisplayRemoteDoc(QString strUrl,QString DirectoryUrl,int  index ,int total);

//	void DisplayRemoteDoc(QString strUrl,int index,int total);

	void ShareCursor();
	void OnShowRemoteCursor(float x,float y);
	void OnCloseRemoteCursor();
	void ShowPPT(QString fileName);
	void CloseShareDoc();
	//显示进度窗口
	void ShowProgressBar();
	//隐藏进度窗口
	void HideProgressBar();
	//更新进度文字
	void UpdateProgress(QString strProgress);
	void ShowCloseDocbtn(bool bShow);
private:
	void GotoPage(int index);

private slots:
		void OnBtnPrev();
		void OnBtnNext();
		void CaptureMouseTimer();

		

public:
	//ToolBar * m_pToolBar;
	SeanTestWdg *m_toolBar;
	//进度条
	SeanPptProgressWdg * m_pProgressBar;
	//add sean
	QTimer *timer;
private:
	OcImageLoader * m_pImgLoader;
	int m_total;
	int m_index;
	QString m_strDocName;
	QString m_strDocDir;
	bool m_bShareCursor;
	int  m_shareState;             //1显示本地，2，显示远程，3空闲
	
	QLabel *m_pRemoteCursor;

	QTimer m_Timer;
	QPoint m_lastPos;

	ConvertDocThread* m_pConvertDocThread;
	QThread m_thread;
	QString directoryUrl;
	
	public slots:
		void slotOfEnterTestWdg();
		void slotOfLeaveTestWdg();
		void slotOfHiddenTool();
		void slotOfGetToHiddenToolBar();
		void slotOfGetShowToolBar();
		void slotConvertFinish(QString fileName,int total);
		signals:
		void EventConvertDoc(QString fileName);

};

#endif // WDGDOCSHARE_H
