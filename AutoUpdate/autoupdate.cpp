#include "autoupdate.h"
#include <QProcess>
#include <QDesktopServices>
AutoUpdate::AutoUpdate(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	ui.setupUi(this);
	m_downloadThread = new DownloadThread(this);
	connect(ui.btnCancel,SIGNAL(clicked()),this,SLOT(OnBtnCancelClick()));
	connect(m_downloadThread,SIGNAL(EventDownloadFinish()),this,SLOT(OnDownloadFinish()));
	connect(m_downloadThread,SIGNAL(EventDownloadProgress(unsigned int)),this,SLOT(OnDownloadProgress(unsigned int)));
}

AutoUpdate::~AutoUpdate()
{

}

void AutoUpdate::OnBtnCancelClick()
{
	QApplication::quit();
}
void AutoUpdate::StartUpdate(QString url)
{
	m_downloadThread->moveToThread(&m_thread);
	m_thread.start();
	m_downloadThread->StartDownload(url);
}

void AutoUpdate::OnDownloadProgress(unsigned int percent)
{
	ui.progressBarDownload->setValue(percent);
}
void AutoUpdate::OnDownloadFinish()
{
	QString strDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)+"\\VideoMeeting\\";
	QProcess *pProcess = new QProcess(this);  
	QStringList t2;  
	t2.append("/s");  
	pProcess->startDetached(strDir+"setup.exe",t2);
	QApplication::quit();
}

