#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include <QtGui/QDialog>
#include "ui_autoupdate.h"
#include <QThread>
#include "downloadthread.h"
class AutoUpdate : public QDialog
{
	Q_OBJECT

public:
	AutoUpdate(QWidget *parent = 0, Qt::WFlags flags = 0);
	~AutoUpdate();

	void  StartUpdate(QString url);
	private slots:
		void OnBtnCancelClick();
		//ÏÂÔØ½ø¶È£¬12%
		void OnDownloadProgress(unsigned int percent);
		void OnDownloadFinish();
private:
	Ui::AutoUpdateClass ui;
	QThread m_thread;
	DownloadThread *m_downloadThread;
	QString m_strUrl;
};

#endif // AUTOUPDATE_H
