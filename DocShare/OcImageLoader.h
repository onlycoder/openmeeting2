#ifndef OCIMAGELOADER_H
#define OCIMAGELOADER_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QCryptographicHash>
#include <QMouseEvent>
#include <QTimer>
#include "SeanThread/SeanThread.h"

/************************************************************************/
/*    显示本地图片和网络图片，带缓存功能
/*/
/************************************************************************/
class OcImageLoader : public QWidget
{
	Q_OBJECT

public:
	OcImageLoader(QWidget *parent);
	~OcImageLoader();
public:
	//显示图片
	void Display(QString url,QString DirectoryUrl,int index,int total);
	//add by sean
	SeanThread myThread;
	QString seanDirectoryUrl;
	int seanIndex;
	int seanTotal;

	int GetImageWidth();
	int GetImageHeight();

	static OcImageLoader* instance;
	static OcImageLoader* GetInstance();

	void Clear();

protected:
	void paintEvent ( QPaintEvent * event );
	void resizeEvent ( QResizeEvent * event );
	void mouseMoveEvent(QMouseEvent *event);
	
	public slots:
		void downloadProgress(qint64 hasRead,qint64 total);
		void OnHttpError(QNetworkReply::NetworkError networkError);
		void OnDownloadReady();
		
public:
	static QString MD5(QString strOrgi);

private:
	QString m_strUrl;
	QImage m_imgMain;
	QImage m_imgOrgi;   //原图
	QString m_strCacheDir;
	QString m_strLocalFileName;
	bool m_bIsHttp;    //是否是远程文件
	unsigned int m_downloadProgress;
	bool m_bDownloading; //下载中
	bool m_bCanShowImg;  //是否可以显示图片
	QNetworkReply *replyGet;

signals:
	void signalOfHiddenToolBar();
	void signalOfShowToolBar();
	
};

#endif // OCIMAGELOADER_H
