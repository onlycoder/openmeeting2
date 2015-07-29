#include "OcImageLoader.h"
#include <QPainter>
#include <QDebug>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
OcImageLoader *OcImageLoader::instance = NULL;
OcImageLoader *OcImageLoader::GetInstance(){

	return instance;
}
OcImageLoader::OcImageLoader(QWidget *parent)
	: QWidget(parent)
{
	instance = this;
	m_bIsHttp = false;
	
	QString path =	QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)+"\\VideoMeeting\\";
	m_strCacheDir = path+"\\cache\\";
	if(QDir(m_strCacheDir).exists()==false)
	{
		QDir(path).mkpath("cache");
	}
	m_downloadProgress = 0 ;
	m_bDownloading = false;
	m_bCanShowImg = false;

	setMouseTracking(true);
	
}

OcImageLoader::~OcImageLoader()
{

}


void OcImageLoader::paintEvent ( QPaintEvent * event )
{
	QPainter painter(this);
	QPen pen;
	pen.setColor(QColor(102,0,0));  //设置画笔为红色
	painter.setPen(pen);  //选择画笔
	//painter.drawRect(0,0,this->width()-1,this->height()-1);
	painter.fillRect(0,0,this->width(),this->height(),QColor(238,238,238));
	if(m_imgMain.isNull()==false&&m_bCanShowImg==true)
	{
		int imgWidth = m_imgMain.width();
		int imgHeight = m_imgMain.height();
		painter.drawImage((width()-imgWidth)/2,(height()-imgHeight)/2,m_imgMain);
	}

	if(m_bDownloading)
	{
		//显示进度
		pen.setColor(QColor(255,255,255));  //设置画笔为红色
		painter.setPen(pen);  //选择画笔
		painter.drawText(16,17,QString("载入进度: %1%").arg(m_downloadProgress));
	}
	painter.end(); 
}

int OcImageLoader::GetImageWidth()
{
	if(m_imgMain.isNull()==false)
	{
		return m_imgMain.width();
	}
	return 0;
}

int OcImageLoader::GetImageHeight()
{
	if(m_imgMain.isNull()==false)
	{
		return m_imgMain.height();
	}
	return 0;
}

QString OcImageLoader::MD5(QString strOrgi)
{
	QByteArray byte_array;
	byte_array.append(strOrgi);
	QByteArray hash_byte_array = QCryptographicHash::hash(byte_array, QCryptographicHash::Md5);
	QString md5 = hash_byte_array.toHex();
	return md5;
}
void OcImageLoader::Clear()
{
	m_bCanShowImg = false;
}
void OcImageLoader::Display(QString url,QString DirectoryUrl,int index,int total)
{
	//http://211.151.17.177/meeting//UploadDoc/c878feaf43247aff65a3b14b896c4a65/2.JPG
	seanDirectoryUrl = DirectoryUrl;
	seanIndex = index;
	seanTotal = total;
	if(url.length()<2)
		return;
	m_strUrl = url;
	qDebug()<<"m_strUrl"<<m_strUrl;
	if(url.startsWith("http://",Qt::CaseInsensitive)==true)
	{
		m_bIsHttp = true;
		m_strLocalFileName = m_strCacheDir+"\\"+MD5(m_strUrl);
// 		//看缓存目录中是否已有文件，如果没有向网络请求数据
		
		if(QFile::exists(m_strLocalFileName)== false)
		{
			m_bCanShowImg = false;
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
			return;
		}
	}
	else
	{
		m_bIsHttp = false;
		m_strLocalFileName = url;	
	}
	
	//本地文件或者缓存中已经有这个图片就直接加载
	m_bCanShowImg = true;
	m_imgOrgi = QImage(m_strLocalFileName);
	if(m_imgOrgi.isNull()==true)
	{
		//QMessageBox::warning(NULL,QString("tip"),m_strLocalFileName);
	}
	m_imgMain = m_imgOrgi.scaled(this->width(),this->height(),Qt::KeepAspectRatio,
		Qt::SmoothTransformation);
	this->update();
}

//下载进度
void OcImageLoader::downloadProgress(qint64 hasRead,qint64 total)
{
	m_bDownloading = true;
	double percent = (double)(hasRead*100)/total;
	m_downloadProgress = (unsigned int)percent;
	this->update();
}

void OcImageLoader::OnHttpError(QNetworkReply::NetworkError networkError)
{
	m_bDownloading = false;
	m_bCanShowImg = false;
}

//文件下载完成
void OcImageLoader::OnDownloadReady()
{
	qDebug()<<"m_strLocalFileName == "<<m_strLocalFileName;
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
	m_bCanShowImg = true;
	m_imgOrgi = QImage(m_strLocalFileName);
	m_imgMain = m_imgOrgi.scaled(this->width()-2,this->height()-2,Qt::KeepAspectRatio,
		Qt::SmoothTransformation);
	this->update();
	for (int i = 0;i < seanTotal+1;i ++)
	{
		myThread.start();
	}
	
}

void OcImageLoader::resizeEvent ( QResizeEvent * event )
{
	if(m_imgOrgi.isNull()== false)
	{
		m_imgMain = m_imgOrgi.scaled(this->width()-2,this->height()-2,Qt::KeepAspectRatio,
			Qt::SmoothTransformation);
	}
	//this->update();
}
void OcImageLoader::mouseMoveEvent(QMouseEvent *event){

	QPoint p1 = event->pos();

	if (p1.y()>this->height() - 60 && p1.y()<this->height()- 4)
	{
		 emit signalOfShowToolBar();
	}
	else
	{
		emit signalOfHiddenToolBar();
	 
	}
}
