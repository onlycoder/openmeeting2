#include "ConvertDocThread.h"
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QDesktopServices>
#include <QMouseEvent>
#include <QDebug>
#include <QAxBase>
#include <QAxObject>
#include <QMessageBox>
#include <QDateTime>
#include <quazip.h>
#include <quazipdir.h>
#include <quazipfile.h>
#include "WdgDocShare.h"
#include "OcImageLoader.h"
#include <Ole2.h>
#include "../stdafx.h"
ConvertDocThread::ConvertDocThread(WdgDocShare * wdgDocShare)
{
	m_pWdgDocShare = wdgDocShare;
}

ConvertDocThread::~ConvertDocThread()
{

}

void ConvertDocThread::uploadProgress(qint64 hasWrite,qint64 total)
{
	int percent = (int)((double)hasWrite*100/(double)total);
	m_pWdgDocShare->UpdateProgress(QString("上传进度%1%").arg(percent));
}

void ConvertDocThread::OnHttpReady()
{
	qDebug()<<"ptt convert complete";
	/*
	m_pWdgDocShare->UpdateProgress(QString("上传文档成功"));
	m_pWdgDocShare->HideProgressBar();
	m_pWdgDocShare->DisplayLocalDoc(m_strUploadName,m_pageTotal);
	*/
	emit EventConvertFinish(m_strUploadName,m_pageTotal);
	return;
}

void ConvertDocThread::OnUploadFinish()
{
	//请求服务器解压,使用Get方法
	QNetworkReply *replyGet;
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	QNetworkRequest request;
	QString unzipUrl = QString("%1/index.php/Home/Index/unzipDoc/fileName/").arg(g_pMeetingFrame->GetBaseUrl())+m_strUploadName;
	
	request.setUrl(QUrl(unzipUrl));
	request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");
	replyGet = manager->get(request);
	connect(replyGet, SIGNAL(readyRead()), this, SLOT(OnHttpReady()),Qt::QueuedConnection);
	connect(replyGet, SIGNAL(error(QNetworkReply::NetworkError)),
		this, SLOT(OnHttpError(QNetworkReply::NetworkError)),Qt::QueuedConnection);
}

void ConvertDocThread::OnHttpError(QNetworkReply::NetworkError networkError)
{

}

void ConvertDocThread::ZipDir(QString strDir,QString strZipFile)
{
	QDir dir = QDir(strDir);
	if(dir.exists() == false)
		return;
	QStringList wrapper = dir.entryList(QDir::Files);
	QuaZip zip(strZipFile);
	zip.setFileNameCodec("IBM866");

	if(!zip.open(QuaZip::mdCreate)){
		qDebug() << "error .... 1";
	}
	QuaZipFile outFile(&zip);
	for(int i = 0 ; i < wrapper.size() ; i++){
		QFileInfo file(strDir+"\\"+wrapper.at(i));
		if(file.exists()){
			QFile inFile ;
			QFile inFileTemp ;			
			inFileTemp.setFileName(file.fileName());
			inFile.setFileName(file.filePath());

			if(!inFile.open(QIODevice::ReadOnly)){
				qDebug() << "error ....2";
				qDebug() << inFile.errorString().toLocal8Bit().constData();
			}

			if(!outFile.open(QIODevice::WriteOnly , QuaZipNewInfo(inFileTemp.fileName() , inFile.fileName()))){
				qDebug() << "error ...3";
			}

			outFile.write(inFile.readAll());

			if(outFile.getZipError() != UNZ_OK){
				qDebug() << "error ...4";
			}

			outFile.close();

			if(outFile.getZipError() != UNZ_OK){
				qDebug() << "error ....5";
			}
			inFile.close();
		}
	}
	zip.close();
	if(zip.getZipError()){
		qDebug() << "error ...6";
	}
}

int ConvertDocThread::ReNameDirFiles(QString strDir)
{
	QDir dir = QDir(strDir);
	QStringList filters;
	filters << "*.JPG";
	QStringList strFileList = dir.entryList(filters,QDir::Files);
	for(int i = 0;i<strFileList.size();i++)
	{
		QString fileName = strFileList[i];
		QFile oldFile(strDir+"\\"+fileName);
		if(oldFile.exists() == false)
			continue;
		QString newFileName = fileName.right(fileName.length()-3);
		oldFile.rename(strDir+"\\"+newFileName);
	}
	return strFileList.size();
}

void ConvertDocThread::StartConvert(QString fileName)
{
	m_pageTotal = 0;
	m_pptFileName = fileName;
	this->run();
}

//线程主体
void ConvertDocThread::run ()
{
	
	HRESULT r;
	r = ::OleInitialize(0);
	if (r != S_OK && r != S_FALSE) {
		qWarning("Qt: Could not initialize OLE (error %x)", (unsigned int)r);
	}

		QString path =	QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)+"//VideoMeeting//";
		//随机文件名
		//QString docName = QString("doc_%1").arg(QDateTime::currentMSecsSinceEpoch());
		QString docName = OcImageLoader::MD5(m_pptFileName);
		m_strUploadName = docName;
		//转换PPT
		QAxObject *ppt = NULL;
		QAxObject *Presentations = NULL;
		QAxObject *Presentation = NULL;
		QAxObject *activePresentation=NULL;
		ppt = new QAxObject("Powerpoint.Application");
		if (ppt->isNull()) {//网络中很多使用excel==NULL判断，是错误的
			QMessageBox::critical(0, "错误信息", "没有找到powerpoint应用程序");
			return;
		}
		QApplication::processEvents();
		ppt->querySubObject("SetTop(QVariant)",-9999);
		ppt->querySubObject("SetVisible(QVariant)",false);
		ppt->querySubObject("SetHeight(QVariant)",0);
		ppt->querySubObject("SetWidth(QVariant)",0);
		Presentations = ppt->querySubObject("Presentations");
		QApplication::processEvents();
		Presentation = Presentations->querySubObject("Open(QString,QVariant,QVariant,QVariant)",
			m_pptFileName,
			true,false,false);
		QApplication::processEvents();
		if(Presentation!=NULL)
		{
			QAxObject *slides = Presentations->querySubObject("Slides");
			if(slides!=NULL)
			{
				m_pageTotal = slides->dynamicCall("getCount()").toInt();
			}
			Presentation->dynamicCall("Export(QString,QString,QVariant,QVariant)",
				path+docName,"JPG",800,600);
			//Presentation->dynamicCall("Save()");
			Presentation->dynamicCall("Close()");
		}
		ppt->dynamicCall("Quit()");
		delete ppt;
		
		//修改文件名，把"幻灯片1.JPG"修改成"1.JPG"
		m_pageTotal = ReNameDirFiles(path+docName);
		//压缩成zip包
		QString zipFileName = path+docName+"\\"+docName+".zip";
		ZipDir(path+docName,zipFileName);
		m_pWdgDocShare->ShowProgressBar();
		m_pWdgDocShare->UpdateProgress(QString("开始上传文档"));
		//上传到Http服务器
		{
			QNetworkReply *replyGet;
			QNetworkAccessManager *manager = new QNetworkAccessManager(this);
			bool isAborted = false;
			QByteArray boundaryRegular(QString("--"+QString::number(qrand(), 10)).toAscii());
			QByteArray boundary("\r\n--"+boundaryRegular+"\r\n");
			QByteArray boundaryLast("\r\n--"+boundaryRegular+"--\r\n");

			QString siteurl = QString("%1/index.php/Home/Index/uploadDoc/").arg(g_pMeetingFrame->GetBaseUrl());
			QUrl url(siteurl);
			QNetworkRequest request(url);
			request.setRawHeader("Host", url.encodedHost());
			request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows; U; Windows NT 5.1; ru; rv:1.9.1.3) Gecko/20090824 Firefox/3.5.3 (.NET CLR 3.5.30729)");
			request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
			request.setRawHeader("Accept-Language", "ru,en-us;q=0.7,en;q=0.3");
			request.setRawHeader("Accept-Encoding", "gzip,deflate");
			request.setRawHeader("Accept-Charset", "windows-1251,utf-8;q=0.7,*;q=0.7");
			request.setRawHeader("Keep-Alive", "300");
			request.setRawHeader("Connection", "keep-alive");
			request.setRawHeader("Referer", siteurl.toAscii());
			request.setRawHeader("Content-Type", QByteArray("multipart/form-data; boundary=").append(boundaryRegular));

			QByteArray mimedata1("--"+boundaryRegular+"\r\n");
			mimedata1.append("Content-Disposition: form-data; name=\"action\"\r\n\r\n");
			mimedata1.append("file_upload");
			mimedata1.append(boundary);
			mimedata1.append("Content-Disposition: form-data; name=\"sfile\"; filename=\""+zipFileName.toUtf8()+"\"\r\n");
			mimedata1.append("Content-Type: application/octet-stream\r\n\r\n");

			QByteArray mimedata2(boundary);
			mimedata2.append("Content-Disposition: form-data; name=\"password\"\r\n\r\n");
			//mimedata2.append(passw.toUtf8());
			mimedata2.append(boundary);
			mimedata2.append("Content-Disposition: form-data; name=\"description\"\r\n\r\n");
			//mimedata2.append(descr.toUtf8());
			mimedata2.append(boundary);
			mimedata2.append("Content-Disposition: form-data; name=\"agree\"\r\n\r\n");
			mimedata2.append("1");
			mimedata2.append(boundaryLast);

			QUpFile* upf = new QUpFile(zipFileName, mimedata1, mimedata2, this);
			if (upf->openFile())
			{
				replyGet = manager->post(request, upf);
				connect(replyGet, SIGNAL(error(QNetworkReply::NetworkError)),
					this, SLOT(OnHttpError(QNetworkReply::NetworkError)),Qt::QueuedConnection);
				connect(replyGet, SIGNAL(uploadProgress(qint64,qint64)), this,SLOT(uploadProgress(qint64,qint64)),Qt::QueuedConnection);
				connect(replyGet, SIGNAL(finished()), this, SLOT(OnUploadFinish()),Qt::QueuedConnection);
				//isInProgress = true;
				//emit started();
			} else
			{
				//emit finished(true, false, tr("Error: can't open file %1").arg(filename));
			}
		}

		OleUninitialize();
}
