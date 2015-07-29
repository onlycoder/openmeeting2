#include "WdgDocShare.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include "../stdafx.h"
#include "../Room/RoomWdg.h"
WdgDocShare::WdgDocShare(QWidget *parent)
	: QWidget(parent)
{
	m_pImgLoader = new OcImageLoader(this);
	m_pImgLoader->setGeometry(0,0,800,600);
	m_toolBar = new SeanTestWdg(this);
	m_toolBar->move((width()-m_toolBar->width())/2,(height()-m_toolBar->height())-4);
	timer = new QTimer(this);
	connect(m_pImgLoader,SIGNAL(signalOfShowToolBar()),this,SLOT(slotOfGetShowToolBar()));
	connect(m_pImgLoader,SIGNAL(signalOfHiddenToolBar()),this,SLOT(slotOfGetToHiddenToolBar()));
	connect(timer,SIGNAL(timeout()),this,SLOT(slotOfHiddenTool()));

	m_pProgressBar = new SeanPptProgressWdg(this);
	m_pProgressBar->hide();
	m_pProgressBar->move((width()-m_pProgressBar->width())/2,(height()-m_pProgressBar->height())/2);

	connect(m_toolBar,SIGNAL(signalOfEnterTestWdg()),this,SLOT(slotOfEnterTestWdg()));
	connect(m_toolBar,SIGNAL(signalOfLeaveTestWdg()),this,SLOT(slotOfLeaveTestWdg()));
    connect(m_toolBar->ui.lastBtn,SIGNAL(clicked()),this,SLOT(OnBtnPrev()));
	connect(m_toolBar->ui.nextBtn,SIGNAL(clicked()),this,SLOT(OnBtnNext()));
	m_index = 1;
	m_total =1;
	m_strDocName = "";
	m_strDocDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)+"\\VideoMeeting";
	m_bShareCursor = true;
	
	connect(&m_Timer, SIGNAL(timeout()), this, SLOT(CaptureMouseTimer()));
	QPixmap pixmap(":/setting/setting/01444.png") ;  
	m_pRemoteCursor = new QLabel(this);
	m_pRemoteCursor->setText("");
	m_pRemoteCursor->setPixmap(pixmap);
	m_pRemoteCursor->setObjectName("cursorLabel");
	m_pRemoteCursor->hide();
	QFile qssFile(":/qss/setting.qss");
	QString qss;
	qssFile.open(QFile::ReadOnly);
	if(qssFile.isOpen())
	{  
		qss = QString(qssFile.readAll());
		this->setStyleSheet(qss);
		qssFile.close();
	}
	m_pRemoteCursor->hide();
	m_pConvertDocThread = new ConvertDocThread(this);
	m_shareState = 3;

	connect(this,SIGNAL(EventConvertDoc(QString)),
		m_pConvertDocThread,SLOT(StartConvert(QString)),Qt::QueuedConnection);

	connect(m_pConvertDocThread,SIGNAL(EventConvertFinish(QString,int)),
		this,SLOT(slotConvertFinish(QString,int)));


}

WdgDocShare::~WdgDocShare()
{

}
/*
bool WdgDocShare::event ( QEvent * event )
{
	if(event->type() == QEvent::MouseMove)
	{
		qDebug()<<"mouse move";
	}
	return false;
}
*/
void WdgDocShare::OnShowRemoteCursor(float x,float y)
{
	m_pRemoteCursor->show();
	float _x1 =(width()-m_pImgLoader->GetImageWidth())/2+(float)(m_pImgLoader->GetImageWidth())*x;
	float _y1 =(height()-m_pImgLoader->GetImageHeight())/2+(float)(m_pImgLoader->GetImageHeight())*y;
	m_pRemoteCursor->move(_x1-m_pRemoteCursor->width()/2,_y1-m_pRemoteCursor->height()/2);
}

void WdgDocShare::OnCloseRemoteCursor()
{
	m_pRemoteCursor->hide();
}

void WdgDocShare::OnBtnPrev()
{

	if(m_index>1)
		m_index--;
	GotoPage(m_index);
}

void WdgDocShare::OnBtnNext()
{
	if(m_index<m_total)
		m_index++;
	else
		return;

	GotoPage(m_index);
}

void WdgDocShare::GotoPage(int index)
{
	//m_pToolBar->m_pLblIndex->setText(QString("%1/%2").arg(index).arg(m_total));
	m_toolBar->ui.pageLabel->setText(QString("%1/%2").arg(index).arg(m_total));
	
	m_pImgLoader->Display(m_strDocDir+"\\"+m_strDocName+"\\"+QString("%1.JPG").arg(index),directoryUrl,index,m_total);
	//通知房间内成员打开这个页面
	if(g_pMeetingFrame)
	{
		g_pMeetingFrame->OpenDoc(m_strDocName.toUtf8().data(),index,m_total);
		//停止广播视频，节省流量
		if(g_pMeetingFrame->HasStartVideo()){
			RoomWdg::GetInstance()->OnCloseVideo();
		}
		
	}
}

void WdgDocShare::resizeEvent ( QResizeEvent * event )
{
	qDebug()<<"width:"<<this->width()<<"height:"<<this->height();
	if(m_pImgLoader!=NULL)
	{
		m_pImgLoader->setGeometry(0,0,width(),height());
		//m_pToolBar->move((width()-m_pToolBar->width())/2,(height()-m_pToolBar->height())-4);
		m_toolBar->move((width()-m_toolBar->width())/2,(height()-m_toolBar->height())-4);
		m_pProgressBar->move((width()-m_pProgressBar->width())/2,(height()-m_pProgressBar->height())/2);
	}
	
}

void WdgDocShare::CaptureMouseTimer()
{
	QPoint p1 = QCursor::pos();
	QPoint p2 = this->mapFromGlobal(p1);
	
	
	//广播给房间内其它成员
	if(g_pMeetingFrame&&(m_lastPos!=p2))
	{
		int _x1 =p2.x()-(width()-m_pImgLoader->GetImageWidth())/2;
		int _y1 =p2.y()-(height()-m_pImgLoader->GetImageHeight())/2;
		float _x = (float)(_x1)/(float)(m_pImgLoader->GetImageWidth());
		float _y = (float)(_y1)/(float)(m_pImgLoader->GetImageHeight());
		qDebug()<<"x:"<<_x<<" _y:"<<_y;

		g_pMeetingFrame->ShareCursor(_x,_y);
		
	}
	m_lastPos = p2;
}


//显示进度窗口
void WdgDocShare::ShowProgressBar()
{
	
		if(m_pProgressBar)
			m_pProgressBar->show();
	
}
//隐藏进度窗口
void WdgDocShare::HideProgressBar()
{
	if(m_pProgressBar)
		m_pProgressBar->hide();
}
//更新进度文字
void WdgDocShare::UpdateProgress(QString strProgress)
{
	if(m_pProgressBar)
	{
		m_pProgressBar->updateProgressWithFloat(strProgress);
	}
}


void WdgDocShare::DisplayRemoteDoc(QString strUrl,QString DirectoryUrl,int index,int total)
{
	directoryUrl = DirectoryUrl;
	m_shareState = 2;
	m_pRemoteCursor->hide();
	setCursor(Qt::ArrowCursor);
	m_toolBar->JustShowLabel();
	m_toolBar->ui.pageLabel->setText(QString("%1/%2").arg(index).arg(total));
	m_pImgLoader->Display(strUrl,DirectoryUrl,index,total);
}
void WdgDocShare::CloseShareDoc()
{
	hide();
	m_shareState = 3;
	m_pImgLoader->Clear();
}

void WdgDocShare::DisplayLocalDoc(QString strUrl,int pageTotal)
{

	if(m_pImgLoader!=NULL)
	{
		QCursor cursor ;  
		QPixmap pixmap(":/setting/setting/01444.png") ;  
		cursor = QCursor(pixmap,-1,-1);  
		setCursor(cursor) ;
		m_Timer.start(50);
		m_shareState = 1;
		m_toolBar->ShowAll();
		m_toolBar->show();
		m_pImgLoader->setGeometry(0,0,width(),height());
		m_strDocName = strUrl;
		m_index = 1;
		m_total = pageTotal;
		//显示第一页
		GotoPage(1);
		m_toolBar->move((width()-m_toolBar->width())/2,(height()-m_toolBar->height())-4);
	}

}

void WdgDocShare::ShowCloseDocbtn(bool bShow){
	m_toolBar->ShowCloseDocBtn(bShow);
}

void WdgDocShare::ShareCursor()
{
	if(m_bShareCursor==false)
	{
		QCursor cursor ;  
		QPixmap pixmap(":/setting/setting/01444.png") ;  
		cursor = QCursor(pixmap,-1,-1);  
		setCursor(cursor) ;
		bool b = this->hasMouseTracking();
		bool b2 = b;
		m_Timer.start(50);
	}
	else
	{
		setCursor(Qt::ArrowCursor);
		m_Timer.stop();
		if(g_pMeetingFrame)
			g_pMeetingFrame->StopShareCursor();
	}
	m_bShareCursor=!m_bShareCursor;
}

void WdgDocShare::slotConvertFinish(QString fileName,int total)
{
	UpdateProgress(QString("上传文档成功"));
	HideProgressBar();
	DisplayLocalDoc(fileName,total);
}

void WdgDocShare::ShowPPT(QString fileName)
{
 	if(m_shareState !=3)
 	{
 		QMessageBox::warning(NULL,"提示","目前正在共享文档，请等待别人退出后再尝试 ");
 		return;
 	}

	QString path =	QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)+"//VideoMeeting//";
	QString docName = OcImageLoader::MD5(fileName);
	m_pRemoteCursor->hide();
	if(QDir(path+docName).exists()==true)
	{   int count = QDir(path+docName).entryList(QDir::Files).size()-1;
		DisplayLocalDoc(docName,count);
	}
	else
	{
		//显示进度窗口
		ShowProgressBar();
		UpdateProgress(QString("开始转换文档"));
		//启动转换文档线程
		m_pConvertDocThread->moveToThread(&m_thread);
		
		m_thread.start();
		emit EventConvertDoc(fileName);
		//m_pConvertDocThread->StartConvert(fileName);
	}
}
void WdgDocShare::slotOfEnterTestWdg(){
	qDebug()<<"slotOfEnterTestWdg";
	setCursor(Qt::ArrowCursor);

}

void WdgDocShare::slotOfLeaveTestWdg(){
	//qDebug()<<"slotOfLeaveTestWdg";
	if (m_bShareCursor &&m_shareState == 1)
	{
		QCursor cursor ;  
		QPixmap pixmap(":/setting/setting/01444.png") ;  
		cursor = QCursor(pixmap,-1,-1);  
		setCursor(cursor) ;
	}else{
		setCursor(Qt::ArrowCursor);
	}
}
void WdgDocShare::slotOfGetShowToolBar()
{
	//if(m_shareState == 1)
		 m_toolBar->show();
}
void WdgDocShare::slotOfGetToHiddenToolBar(){
	
	timer->start(500);
	
}
void WdgDocShare::slotOfHiddenTool(){

	m_toolBar->hide();
	timer->stop();
}
