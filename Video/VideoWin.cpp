#include "VideoWin.h"
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include "VideoMgr.h"
#include "../Room/RoomWdg.h"
VideoWin::VideoWin(QWidget *parent)
	: QWidget(parent)
	,m_bFree(true)
{
	this->setObjectName("VideoWinBG");
	this->setUpdatesEnabled(true);
	m_nHeight = m_nWidth = 0;
	m_pVieoData = NULL;
	m_userID = -1;
	m_bKeepAspect = true;
	m_bFullScreen = false;
	m_bShowUserName = true;
	m_Timer = new QTimer(this);
	m_Timer->setInterval(40);
	memset(m_strUserName,0,sizeof(m_strUserName));
	connect(m_Timer,SIGNAL(timeout()),this,SLOT(refreshVideo()));

	m_pMenu = new QMenu();
	menuKeepAspect = m_pMenu->addAction((tr("保持比例")));
	menuFitWindow = m_pMenu->addAction((tr("适应窗口")));
	menuShowUserName = m_pMenu->addAction((tr("显示用户名")));

	menuShowUserName->setCheckable(true);
	menuKeepAspect->setCheckable(true);
	menuFitWindow->setCheckable(true);

	menuShowUserName->setChecked(m_bShowUserName);
	menuKeepAspect->setChecked(m_bKeepAspect);

	modeGroup = new QActionGroup(this);
	modeGroup->addAction(menuKeepAspect);
	modeGroup->addAction(menuFitWindow);
	connect(menuKeepAspect,SIGNAL(triggered()),this,SLOT(onKeepAspect()));
	connect(menuFitWindow,SIGNAL(triggered()),this,SLOT(onFitWindow()));
	connect(menuShowUserName,SIGNAL(triggered()),this,SLOT(onShowUserName()));
}

VideoWin::~VideoWin()
{
	if(m_pVieoData!=NULL)
	{
		delete m_pVieoData;
		m_pVieoData = NULL;
	}
}


void VideoWin::mousePressEvent ( QMouseEvent * event ){
	if(event->button() == Qt::RightButton){
		m_pMenu->exec(event->globalPos());
	}
	
}


void VideoWin::onShowUserName(){
	m_bShowUserName=!m_bShowUserName;
	RoomWdg::GetInstance()->OnQuitFullScreen();
}

void VideoWin::onKeepAspect(){
	m_bKeepAspect = true;
	
}

void VideoWin::onFitWindow(){
	m_bKeepAspect = false;
}

void VideoWin::DeliveData(int w,int h,unsigned char* pData){
	QMutexLocker autoLock(&m_Mutex);
	if(pData == NULL)
		return;
	if(m_pVieoData == NULL){
		m_pVieoData = new unsigned char[w*h*3];
		m_nWidth = w;
		m_nHeight = h;
	}
	memcpy(m_pVieoData,pData,w*h*3);
	
}

void VideoWin::StartRender(){
	QMutexLocker autoLock(&m_Mutex);
		m_Timer->start();
}

void VideoWin::StopRender(){
	QMutexLocker autoLock(&m_Mutex);
	m_userID = -1;
	m_Timer->stop();
	if(m_pVieoData != NULL)
	{
		delete[] m_pVieoData;
		m_pVieoData = NULL;
		memset(m_strUserName,0,sizeof(m_strUserName));
	}
	this->update();
}

void VideoWin::SetUserID(unsigned __int64  ulUserId)
{ 
	QMutexLocker autoLock(&m_Mutex);
	m_userID = ulUserId;

}

HWND VideoWin::GetVideoHWND()
{
	return (HWND)this->winId();
}

void VideoWin::SetFree(bool bFree )
{
	m_bFree = bFree;
	StopRender();
	
}

bool VideoWin::isFree()
{ 
	return m_bFree;
}


void VideoWin::SetPos(int x,int y,int x_space,int y_space)
{ 
	m_x = x;
	m_y = y;
	m_xSpace = x_space;
	m_ySpace = y_space;
}

void VideoWin::SetHidden(bool bHide)
{
	/*
	if(bHide){
		if(m_Timer->isActive())
			m_Timer->stop();
	}
	else{
		if(m_pVieoData!=NULL&&m_Timer->isActive()==false)
			m_Timer->start();
	}
	*/
	this->setHidden(bHide);
}

void VideoWin::refreshVideo(){
	this->update();
	
}

void VideoWin::SetUserName(char* strUserName)
{
	QMutexLocker autoLock(&m_Mutex);
	if(strUserName!=NULL&&strlen(strUserName)>0){
		strcpy(m_strUserName,strUserName);
	}else
	{
		memset(m_strUserName,0,sizeof(m_strUserName));
	}

}

QPixmap* NinePatch(QString& picName,int iHorzSplit,int iVertSplit, int DstWidth, int DstHeight,QPainter* painter)
{
	if(DstWidth>1920||DstHeight>1080)
		return NULL;
	QPixmap* pix = new QPixmap(picName);
	int pixWidth = pix->width();
	int pixHeight = pix->height();

	QPixmap pix_1 = pix->copy(0, 0, iHorzSplit, iVertSplit);
	QPixmap pix_2 = pix->copy(iHorzSplit, 0, pixWidth-iHorzSplit*2, iVertSplit);
	QPixmap pix_3 = pix->copy(pixWidth-iHorzSplit, 0, iHorzSplit, iVertSplit);

	QPixmap pix_4 = pix->copy(0, iVertSplit, iHorzSplit, pixHeight-iVertSplit*2);
	QPixmap pix_5 = pix->copy(iHorzSplit, iVertSplit, pixWidth-iHorzSplit*2, pixHeight-iVertSplit*2);
	QPixmap pix_6 = pix->copy(pixWidth-iHorzSplit, iVertSplit, iHorzSplit, pixHeight-iVertSplit*2);

	QPixmap pix_7 = pix->copy(0, pixHeight-iVertSplit, iHorzSplit, iVertSplit);
	QPixmap pix_8 = pix->copy(iHorzSplit, pixHeight-iVertSplit, pixWidth-iHorzSplit*2, pixWidth - iHorzSplit*2);
	QPixmap pix_9 = pix->copy(pixWidth-iHorzSplit, pixHeight-iVertSplit, iHorzSplit, iVertSplit);

	pix_2 = pix_2.scaled(DstWidth-iHorzSplit*2,iVertSplit, Qt::IgnoreAspectRatio);//保持高度拉宽
	pix_4 = pix_4.scaled(iHorzSplit, DstHeight-iVertSplit*2, Qt::IgnoreAspectRatio);//保持宽度拉高
	pix_5 = pix_5.scaled(DstWidth-iHorzSplit*2,DstHeight-iVertSplit*2, Qt::IgnoreAspectRatio);
	pix_6 = pix_6.scaled(iHorzSplit, DstHeight-iVertSplit*2, Qt::IgnoreAspectRatio);//保持宽度拉高
	pix_8 = pix_8.scaled(DstWidth-iHorzSplit*2, iVertSplit);//保持高度拉宽


	QPixmap* resultImg = NULL;
	try
	{
		resultImg = new QPixmap(DstWidth, DstHeight);
	}
	catch (...)
	{
	}
	
	//QPainter* painter = new QPainter(resultImg);
	if (resultImg!=NULL&&!resultImg->isNull()) {
		painter->drawPixmap(0,0,pix_1);
		painter->drawPixmap(iHorzSplit, 0, pix_2);
		painter->drawPixmap(DstWidth-iHorzSplit,0,pix_3);

		painter->drawPixmap(0, iVertSplit, pix_4);
		painter->drawPixmap(iHorzSplit, iVertSplit, pix_5);
		painter->drawPixmap(DstWidth-iHorzSplit, iVertSplit, pix_6);

		painter->drawPixmap(0, DstHeight-iVertSplit, pix_7);
		painter->drawPixmap(iHorzSplit, DstHeight-iVertSplit, pix_8);
		painter->drawPixmap(DstWidth-iHorzSplit, DstHeight-iVertSplit, pix_9);
		//painter->end();
	}
	return resultImg;
}
void VideoWin::mouseDoubleClickEvent ( QMouseEvent * event ){
	if(m_bFree==false)
	{
		emit EventDBClick(m_userID,m_bFullScreen);
		m_bFullScreen=!m_bFullScreen;
	}
}

void VideoWin::paintEvent ( QPaintEvent * event )
{
	QPainter painter(this);
	QPen pen;
	int w = this->width();
	int h = this->height();
	
	QMutexLocker autoLock(&m_Mutex);
	
	if(m_pVieoData!=NULL){
		if(m_bKeepAspect == true){
			float x = (float)m_nWidth/(float)m_nHeight;
			int w2 = h*x;
			//画背景
			QPen pen;
			pen.setColor(QColor(102,0,0));  //设置画笔为红色
			painter.setPen(pen);  //选择画笔
			//painter.drawRect(0,0,w,h);
			painter.fillRect(0,0,w,h,QColor(0,0,0));
			
			QImage videoFrame(m_pVieoData,m_nWidth,m_nHeight,QImage::Format_RGB888);
			painter.drawImage(QRect((w-w2)/2,0,w2,h),videoFrame,QRect(0,0,m_nWidth,m_nHeight));
		}else{
			QImage videoFrame(m_pVieoData,m_nWidth,m_nHeight,QImage::Format_RGB888);
			painter.drawImage(QRect(0,0,w,h),videoFrame,QRect(0,0,m_nWidth,m_nHeight));
		}
		
	}else{
		//没有视频数据，画背景图
		NinePatch(QString(":/skin/video/smallVideoBG.png"),5,5,w,h,&painter);
		QPixmap imgLogo(":/skin/video/smallvideoLogo2.png");
		if(imgLogo.isNull()== false){
			painter.drawPixmap(abs((w-imgLogo.width())/2),abs((h-imgLogo.height())/2),imgLogo);
		}
		
	}
	if(strlen(m_strUserName)>0&&m_bShowUserName)
	{
		pen.setColor(QColor(0,0,0));  //设置画笔为红色
		painter.drawText(5,15,QString::fromUtf8(m_strUserName));
	}
	
	painter.end(); 

}