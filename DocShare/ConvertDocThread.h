#ifndef CONVERTDOCTHREAD_H
#define CONVERTDOCTHREAD_H

#include <QObject>
#include <QThread>
#include "qupfile.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
class WdgDocShare;
class ConvertDocThread : public QObject
{
	Q_OBJECT

public:
	ConvertDocThread(WdgDocShare * wdgDocShare);
	~ConvertDocThread();
public slots:
	void StartConvert(QString fileName);
protected:
	virtual void run ();

	private slots:
		void OnUploadFinish();
		void OnHttpReady();
		void OnHttpError(QNetworkReply::NetworkError networkError);
		//上传进度
		void uploadProgress(qint64 hasWrite,qint64 total);
private:
	void ZipDir(QString strDir,QString strZipFile);
	int ReNameDirFiles(QString strDir);

signals:
	void EventConvertFinish(QString fileName,int total);

private:
	QString m_strUploadName;
	
	unsigned int m_pageTotal;
	WdgDocShare * m_pWdgDocShare;
	QString m_pptFileName;
};

#endif // CONVERTDOCTHREAD_H
