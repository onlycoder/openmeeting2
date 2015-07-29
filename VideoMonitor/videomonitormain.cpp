#include "videomonitormain.h"
#include <QDebug>
#include <QTimer>
VideoMonitorMain * VideoMonitorMain::m_instance = NULL;;
VideoMonitorMain * VideoMonitorMain::GetInstance()
{
	if(m_instance == NULL)
	{
		m_instance = new VideoMonitorMain();
	}
	return m_instance;
}


VideoMonitorMain::VideoMonitorMain(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_countPerPage = 9;
	m_totalPage = 1;
	m_curPage = 1;
	ui.cmbVideoNumber->addItem(QString("1"));
	ui.cmbVideoNumber->addItem(QString("4"));
	ui.cmbVideoNumber->addItem(QString("9"));
	ui.cmbVideoNumber->addItem(QString("16"));
	ui.cmbVideoNumber->addItem(QString("25"));
	ui.cmbVideoNumber->addItem(QString("36"));
	ui.cmbVideoNumber->addItem(QString("49"));
	ui.cmbVideoNumber->addItem(QString("64"));
	ui.cmbVideoNumber->setCurrentIndex(2);
	ui.lblPage->setText(QString("%1/%2").arg(m_curPage).arg(m_totalPage));
	m_videoMgr = new VideoMonitorMgr(ui.wdgVideoContainer);
	connect(ui.cmbVideoNumber,SIGNAL(currentIndexChanged(int)),this,
		SLOT(OnVideoModeChange(int)),Qt::QueuedConnection);
	connect(ui.btnStart,SIGNAL(clicked()),this,SLOT(OnBtnStartClick()));
	connect(ui.btnStop,SIGNAL(clicked()),this,SLOT(OnBtnStopClick()));
	connect(ui.btnPrev,SIGNAL(clicked()),this,SLOT(OnBtnPrevClick()));
	connect(ui.btnNext,SIGNAL(clicked()),this,SLOT(OnBtnNextClick()));
	connect(ui.chkAutoPage,SIGNAL(clicked()),this,SLOT(OnChkAutoPage()));
	
	m_Timer = new QTimer(this);
	m_Timer->setInterval(25000);
	connect(m_Timer, SIGNAL(timeout()), this,SLOT(OnAutoPageTimer()));
}

VideoMonitorMain::~VideoMonitorMain()
{

}


void VideoMonitorMain::OnVideoModeChange(int index){
	qDebug()<<"video number change:"<<index;
	VideoMonitorMgr::GetInstance()->ChangeVideoMode(index+1);
	m_countPerPage = (index+1)*(index+1);
	m_curPage = 1;
	if(m_videoList.size() ==0){
		m_totalPage = 1;
	}
	else
	{
		if(m_videoList.size()%m_countPerPage>0)
			m_totalPage = m_videoList.size()/m_countPerPage+1;
		else
			m_totalPage = m_videoList.size()/m_countPerPage;
	}

	ui.lblPage->setText(QString("%1/%2").arg(m_curPage).arg(m_totalPage));
	OpenCurrentVideo();
}

//启动监控
void VideoMonitorMain::OnBtnStartClick()
{
	if(g_pMeetingFrame!=NULL){
		g_pMeetingFrame->StartVideoMonitor();
	}
}
//停止监控
void VideoMonitorMain::OnBtnStopClick()
{
	StopRecvAllVideo();
	if(g_pMeetingFrame!=NULL){
		g_pMeetingFrame->StopVideoMonitor();
	}
	m_videoList.clear();
}

void VideoMonitorMain::OnBtnPrevClick()
{
	if(m_curPage > 1)
	{
		m_curPage--;
		ui.lblPage->setText(QString("%1/%2").arg(m_curPage).arg(m_totalPage));
		OpenCurrentVideo();
	}
	
}

void VideoMonitorMain::OnBtnNextClick()
{
	if(m_curPage<m_totalPage)
	{
		m_curPage++;
		ui.lblPage->setText(QString("%1/%2").arg(m_curPage).arg(m_totalPage));
		OpenCurrentVideo();
	}
}


void VideoMonitorMain::OnAutoPageTimer(){
	if(m_videoList.size()<=m_countPerPage)
		return;
	if(m_curPage == m_totalPage)
	{
		m_curPage = 1;
	}
	else
	{
		m_curPage++;
	}
	ui.lblPage->setText(QString("%1/%2").arg(m_curPage).arg(m_totalPage));
	OpenCurrentVideo();
}
//是否自动翻页
void VideoMonitorMain::OnChkAutoPage()
{
	if(ui.chkAutoPage->isChecked()==true){
		ui.btnPrev->setEnabled(false);
		ui.btnNext->setEnabled(false);
		m_Timer->start();
	}else{
		ui.btnPrev->setEnabled(true);
		ui.btnNext->setEnabled(true);
		m_Timer->stop();
	}
}
void VideoMonitorMain::OpenCurrentVideo()
{
	StopRecvAllVideo();
	for(int i=(m_curPage-1)*m_countPerPage;i<m_curPage*m_countPerPage;i++){
		if(i<m_videoList.size()){
			PVideoItem pVideoItem = m_videoList[i];
			g_pMeetingFrame->StartRecvRemoteVideo2(pVideoItem ->userId,pVideoItem->ssrc,
				VideoMonitorMgr::GetInstance()->GetFreeVideo());
			pVideoItem->hasOpen = 1;
		}
	}
}
void VideoMonitorMain::StopRecvAllVideo()
{
	if(g_pMeetingFrame!=NULL){
		
		vector<PVideoItem>::iterator item = m_videoList.begin();
		while(item!=m_videoList.end())
		{
			PVideoItem pVideoItem =(PVideoItem)(*item);
			if(pVideoItem->hasOpen==1){
				g_pMeetingFrame->StopRecvRemoteVideo2(pVideoItem->ssrc);
				VideoMonitorMgr::GetInstance()->GivebackVideo(pVideoItem->userId);
				pVideoItem->hasOpen = 0;
			}
			item++;
		}
	}
}

void VideoMonitorMain::AddVideo(uint64_t userId,uint32_t ssrc)
{
	
	int i = 0;
	bool bFind = false;
	for(i = 0;i<m_videoList.size();i++){
		PVideoItem pVideoItem = m_videoList[i];
		if(pVideoItem->userId == userId){
			bFind = true;
			pVideoItem->ssrc = ssrc;
			break;
		}
	}
	//没有打到，新建一个
	if(bFind == false)
	{
		PVideoItem pVideoItem = new VideoItem();
		pVideoItem->ssrc = ssrc;
		pVideoItem->userId = userId;
		pVideoItem->hasOpen = 0;
		if(m_videoList.size()<m_curPage*m_countPerPage)
		{
			g_pMeetingFrame->StartRecvRemoteVideo2(userId,ssrc,VideoMonitorMgr::GetInstance()->GetFreeVideo());
			pVideoItem->hasOpen = 1;
		}
		m_videoList.push_back(pVideoItem);
	}else
	{
		if(i>(m_curPage-1)*m_countPerPage&&i<(m_curPage+1)*m_countPerPage)
		{
			PVideoItem pVideoItem = m_videoList[i];
			g_pMeetingFrame->StartRecvRemoteVideo2(pVideoItem->userId,pVideoItem->ssrc,
				VideoMonitorMgr::GetInstance()->GetFreeVideo());
			pVideoItem->hasOpen = 1;
		}
	}
	qDebug()<<"video list size:"<<m_videoList.size();
	if(m_videoList.size() ==0){
		m_totalPage = 1;
	}
	else
	{
		if(m_videoList.size()%m_countPerPage>0)
			m_totalPage = m_videoList.size()/m_countPerPage+1;
		else
			m_totalPage = m_videoList.size()/m_countPerPage;
	}

	ui.lblPage->setText(QString("%1/%2").arg(m_curPage).arg(m_totalPage));

}

void VideoMonitorMain::RemoveVideo(uint64_t userId)
{
	vector<PVideoItem>::iterator item = m_videoList.begin();
	while(item!=m_videoList.end())
	{
		PVideoItem pVideoItem =(PVideoItem)(*item);
		if(pVideoItem->userId == userId){
			m_videoList.erase(item);
			if(pVideoItem->hasOpen==1){
				if(g_pMeetingFrame!=NULL){
					g_pMeetingFrame->StopRecvRemoteVideo2(pVideoItem->ssrc);
					VideoMonitorMgr::GetInstance()->GivebackVideo(pVideoItem->userId);
				}
			}
			delete pVideoItem;
			break;
		}
		item++;
	}
	
}