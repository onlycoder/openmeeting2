#include "downloadthread.h"
#include <QThread>
#include <QDebug>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
DownloadThread::DownloadThread(QObject *parent)
	: QObject(parent)
{
	m_strDir =	QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)+"\\VideoMeeting\\";
	m_strLocalFileName = QString(m_strDir+"setup.exe");
}

DownloadThread::~DownloadThread()
{

}

void DownloadThread::StartDownload(QString url)
{
	m_strUrl = url;
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	QNetworkRequest request;
	request.setUrl(QUrl(m_strUrl));
	request.setRawHeader("User-Agent", "OcBrowser 1.0");
	replyGet = manager->get(request);
	connect(replyGet, SIGNAL(finished()), this, SLOT(OnDownloadReady()));
	connect(replyGet, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
	connect(replyGet, SIGNAL(error(QNetworkReply::NetworkError)),
		this, SLOT(OnHttpError(QNetworkReply::NetworkError)));
	m_bDownloading = true;
}

//下载进度
void DownloadThread::downloadProgress(qint64 hasRead,qint64 total)
{
	m_bDownloading = true;
	double percent = (double)(hasRead*100)/total;
	m_downloadProgress = (unsigned int)percent;
	emit EventDownloadProgress(m_downloadProgress);
}

void DownloadThread::OnHttpError(QNetworkReply::NetworkError networkError)
{
	m_bDownloading = false;
}

//文件下载完成
void DownloadThread::OnDownloadReady()
{
	QDir dir(m_strDir);
	if(dir.exists() == false)
	{
		dir.mkpath(m_strDir);
	}

	m_bDownloading = false;
	//写入缓存
	QFile imgFile(m_strLocalFileName);
	if(!imgFile.open(QIODevice::WriteOnly))  
	{  
		qDebug()<<"open file error";
		return;
	}  
	char buf[1024];
	int len = 0;
	while((len=replyGet->read(buf,1024))>0)
	{
		imgFile.write(buf,len);
	}
	imgFile.close();
	emit EventDownloadFinish();
}
