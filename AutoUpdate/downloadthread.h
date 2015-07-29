#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>

class DownloadThread : public QObject
{
	Q_OBJECT

public:
	DownloadThread(QObject *parent);
	~DownloadThread();

	public slots:
		void downloadProgress(qint64 hasRead,qint64 total);
		void OnHttpError(QNetworkReply::NetworkError networkError);
		void OnDownloadReady();
public :
	void StartDownload(QString url);
signals:
	void EventDownloadFinish();
	void EventDownloadProgress(unsigned int percent);
private:
	unsigned int m_downloadProgress;
	bool m_bDownloading; //œ¬‘ÿ÷–
	QNetworkReply *replyGet;
	QString m_strUrl;
	QString m_strLocalFileName;
	QString m_strDir;
};

#endif // DOWNLOADTHREAD_H
