#ifndef SEANTHREAD_H
#define SEANTHREAD_H

#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QCryptographicHash>
#include <QMutex>
class SeanThread : public QThread
{
	Q_OBJECT

public:
	SeanThread();
	~SeanThread();
   static SeanThread *instance;
   static SeanThread *GetInstance();
   void run(); 
   bool isTrue;
  QString MD5(QString strOrgi);
private:
	QNetworkReply *replyGet;
	QString m_strCacheDir;
	QString m_strLocalFileName;

	QString unUrl;
	int currentPage;
	int totalPage;
	 QMutex mutex;

protected:
	  
	    
	
public slots:
			void downloadProgress(qint64 hasRead,qint64 total);
			void OnHttpError(QNetworkReply::NetworkError networkError);
			void OnDownloadReady();
		
};

#endif // SEANTHREAD_H
