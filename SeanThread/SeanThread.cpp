#include "StdAfx.h"
#include "SeanThread.h"
#include <QDesktopServices>
#include <QDir>
#include <QDebug>
#include "OcImageLoader.h"

SeanThread *SeanThread::instance = NULL;
SeanThread *SeanThread::GetInstance(){
	if (instance == NULL)
	{
		instance= new SeanThread();
	}
	return instance;
}
SeanThread::SeanThread()
{
	QString path =	QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)+"\\VideoMeeting\\";
	m_strCacheDir = path+"\\cache\\";
	if(QDir(m_strCacheDir).exists()==false)
	{
		QDir(path).mkpath("cache");
	}
	isTrue = true;
	currentPage = 0;
 	totalPage = 0;
	unUrl = "";
}

SeanThread::~SeanThread()
{

}
void SeanThread::run()
{
	//mutex.lock();
	qDebug()<<"run QTreadId == "<<QThread::currentThreadId();
	currentPage = OcImageLoader::GetInstance()->seanIndex;
	totalPage = OcImageLoader::GetInstance()->seanTotal;
	unUrl = OcImageLoader::GetInstance()->seanDirectoryUrl;
	while(isTrue){
		qDebug()<<"currentPage == "<<currentPage<<"totalPage "<<totalPage;
		if (currentPage > totalPage)
		{
			currentPage = totalPage;
			break;
		}else{
			QString urlStr = QString("%1/UploadDoc/%2/%3.JPG").arg(g_pMeetingFrame->GetBaseUrl()).arg(unUrl).arg(currentPage);   
			m_strLocalFileName = m_strCacheDir+"\\"+MD5(urlStr);
			if(QFile::exists(m_strLocalFileName)== false){
				QNetworkAccessManager *manager = new QNetworkAccessManager();
				QNetworkRequest request;
				qDebug()<<"urlStr == "<<urlStr;
				request.setUrl(QUrl(urlStr));
				request.setRawHeader("User-Agent", "OcBrowser 1.0");
				replyGet = manager->get(request);
				connect(replyGet, SIGNAL(finished()), this, SLOT(OnDownloadReady()));
				connect(replyGet, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
				connect(replyGet, SIGNAL(error(QNetworkReply::NetworkError)),
					this, SLOT(OnHttpError(QNetworkReply::NetworkError)));
				this->exec();

 			}
			else{
				currentPage ++;
			}
 		}
	}
	//mutex.unlock();
}

//下载进度
void SeanThread::downloadProgress(qint64 hasRead,qint64 total)
{
	//mutex.lock();
	double percent = (double)(hasRead*100)/total;
	qDebug()<<"percent == "<<percent;
	//mutex.unlock();
}

void SeanThread::OnHttpError(QNetworkReply::NetworkError networkError)
{


}

//文件下载完成
void SeanThread::OnDownloadReady()
{
	//mutex.lock();
	qDebug()<<"OnDownloadReady QTreadId == "<<QThread::currentThreadId();
	//QMutexLocker locker(&mutex);
	qDebug()<<"m_strCacheDir == "<<m_strLocalFileName;
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
	currentPage ++;
	//mutex.unlock();
	//run();
	quit();
}


QString SeanThread::MD5(QString strOrgi)
{
	QByteArray byte_array;
	byte_array.append(strOrgi);
	QByteArray hash_byte_array = QCryptographicHash::hash(byte_array, QCryptographicHash::Md5);
	QString md5 = hash_byte_array.toHex();
	return md5;
}
