#include "VideoMgr.h"
#include <QGridLayout>

VideoMgr* VideoMgr::m_instance = NULL;
VideoMgr* VideoMgr::GetInstance()
{
	return m_instance;
}

VideoMgr::VideoMgr(QWidget *videoContainer)
{
	m_bShareDocing = false;
	m_instance = this;
	m_pVideoContainer = videoContainer;
	gridLayout = new QGridLayout();
	m_pVideoContainer->setLayout(gridLayout);
	gridLayout->setMargin(2);
	gridLayout->setSpacing(0);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);
	VideoWin * video0 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video0,1,1,2,2);
	video0->SetPos(1,1,2,2);
	m_pVideoList[12] = video0;
	m_currentBigVideo = 12;
	VideoWin * video1 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video1,0,0,1,1);
	video1->SetPos(0,0,1,1);
	m_pVideoList[0] = video1;
	VideoWin * video2 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video2,0,1,1,1);
	video2->SetPos(0,1,1,1);
	m_pVideoList[1] = video2;
	VideoWin * video3 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video3,0,2,1,1);
	video3->SetPos(0,2,1,1);
	m_pVideoList[2] = video3;
	VideoWin * video4 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video4,0,3,1,1);
	video4->SetPos(0,3,1,1);
	m_pVideoList[3] = video4;
	VideoWin * video5 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video5,1,3,1,1);
	video5->SetPos(1,3,1,1);
	m_pVideoList[4] = video5;
	VideoWin * video6 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video6,2,3,1,1);
	video6->SetPos(2,3,1,1);
	m_pVideoList[5] = video6;
	VideoWin * video7 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video7,3,3,1,1);
	video7->SetPos(3,3,1,1);
	m_pVideoList[6] = video7;
	VideoWin * video8 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video8,3,2,1,1);
	video8->SetPos(3,2,1,1);
	m_pVideoList[7] = video8;
	VideoWin * video9 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video9,3,1,1,1);
	video9->SetPos(3,1,1,1);
	m_pVideoList[8] = video9;
	VideoWin * video10 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video10,3,0,1,1);
	video10->SetPos(3,0,1,1);
	m_pVideoList[9] = video10;
	VideoWin * video11 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video11,2,0,1,1);
	video11->SetPos(2,0,1,1);
	m_pVideoList[10] = video11;
	VideoWin * video12 = new VideoWin(m_pVideoContainer);
	gridLayout->addWidget(video12,1,0,1,1);
	video12->SetPos(1,0,1,1);
	m_pVideoList[11] = video12;
	
	for(int i = 0; i<13;i++)
	{
		connect(m_pVideoList[i],SIGNAL(EventDBClick(qint64,bool)),this,SLOT(OnVideoWinDbClick(qint64,bool)));
		m_pVideoList[i]->SetFree(true);
	}
}

VideoMgr::~VideoMgr()
{

}

QGridLayout* VideoMgr::GetLayout()
{
	return gridLayout;
}

void VideoMgr::SetBigVideo(uint64_t userID)
{
	int count = 13;
	int findIndex = -1;
	for(int i = 0; i<count;i++)
	{
		if(m_pVideoList[i]->GetUserID() == userID)
		{
			findIndex = i;
			break;
		}
	}
	if(findIndex == -1 || findIndex == m_currentBigVideo)
		return;
	gridLayout->removeWidget(m_pVideoList[findIndex]);
	gridLayout->removeWidget(m_pVideoList[m_currentBigVideo]);
	gridLayout->addWidget(m_pVideoList[findIndex],1,1,2,2);
	int x,y,xSpace,ySpace;
	x = m_pVideoList[findIndex]->m_x;
	y = m_pVideoList[findIndex]->m_y;
	xSpace = m_pVideoList[findIndex]->m_xSpace;
	ySpace = m_pVideoList[findIndex]->m_ySpace;
	m_pVideoList[findIndex]->SetPos(1,1,2,2);
	gridLayout->addWidget(m_pVideoList[m_currentBigVideo],x,y,xSpace,ySpace);
	m_pVideoList[m_currentBigVideo]->SetPos(x,y,xSpace,ySpace);
	m_currentBigVideo = findIndex;
	this->UpdateUI();
}

IVideoWin* VideoMgr::GetFreeVideo()
{
	if(g_pMeetingFrame->GetRoomMode()==2)
	{
		//m_pVideoList[m_currentBigVideo]->setUpdatesEnabled(false);
		return m_pVideoList[m_currentBigVideo];
	}
	else
	{
		for(int i = 0; i<13;i++)
		{
			if(m_pVideoList[i]->isFree())
			{
				//m_pVideoList[i]->setUpdatesEnabled(false);
				m_pVideoList[i]->SetFree(false);
				return m_pVideoList[i];
			}
		}
	}
	
	return NULL;
}

void VideoMgr::OnRoomModeChange(int mode)
{
	if(m_bShareDocing == true)
		return;
	for(int i = 0; i<13;i++)
	{
		m_pVideoList[i]->setUpdatesEnabled(true);
		m_pVideoList[i]->SetFree(true);
		if(mode == 1)
		{
			//m_pVideoList[i]->SetHidden(false);
			ReLayoutAllVideo();
		}
		else
		{
			if(i!=m_currentBigVideo){
				m_pVideoList[i]->SetHidden(true);
				gridLayout->removeWidget(m_pVideoList[i]);
			}
			else{
				m_pVideoList[i]->SetHidden(false);
				gridLayout->removeWidget(m_pVideoList[i]);
				gridLayout->addWidget(m_pVideoList[i],0,0,3,3);
			}

		}
	}
}
//双击一个视频窗口
void VideoMgr::OnVideoWinDbClick(qint64 userId,bool bFullScreen){
	qDebug()<<"OnVideoWinDbClick:"<<userId;
	if(m_bShareDocing == true)
		return;
	
		VideoWin* w = NULL;
		if(bFullScreen ==false){
			for(int i = 0; i<13;i++)
			{
				if(m_pVideoList[i]->GetUserID() == userId)
				{
					w = m_pVideoList[i];
				}else{
					m_pVideoList[i]->SetHidden(true);
				}
				gridLayout->removeWidget(m_pVideoList[i]);
			}
			if(w!=NULL)
				gridLayout->addWidget(w,0,0,3,3);
			
		}else{
			for(int i = 0; i<13;i++)
			{
				w = m_pVideoList[i];
				if(m_pVideoList[i]->GetUserID() == userId)
				{
					gridLayout->removeWidget(w);
				}else{
					m_pVideoList[i]->SetHidden(false);
				}
				gridLayout->addWidget(w,w->m_x,w->m_y,w->m_xSpace,w->m_ySpace);
			}
			
		}
		
	
}

void VideoMgr::FreeAll(){
	for(int i = 0; i<13;i++)
	{
		m_pVideoList[i]->setUpdatesEnabled(true);
		m_pVideoList[i]->SetFree(true);
	}
}

void VideoMgr::GivebackVideo(uint64_t ssrc)
{
	if(m_bShareDocing == true)
		return;
	if(g_pMeetingFrame->GetRoomMode()==2)
	{
		 m_pVideoList[m_currentBigVideo]->setUpdatesEnabled(true);
		 m_pVideoList[m_currentBigVideo]->SetFree(true);
	}
	else
	{
		for(int i = 0; i<13;i++)
		{
			if(m_pVideoList[i]->GetUserID() == ssrc)
			{
				m_pVideoList[i]->setUpdatesEnabled(true);
				m_pVideoList[i]->SetFree(true);

			}
		}
	}
}

void VideoMgr::HideAll()
{
	m_bShareDocing = true;
	for(int i = 0; i<13;i++)
	{
		m_pVideoList[i]->SetHidden(true);
	}
}

void VideoMgr::ShowAll()
{
	m_bShareDocing = false;
	OnRoomModeChange(g_pMeetingFrame->GetRoomMode());
}
void VideoMgr::ReLayoutAllVideo(){
	for(int i = 0; i<13;i++)
	{   
		VideoWin *w = m_pVideoList[i];
		w->SetHidden(false);
		gridLayout->removeWidget(w);
		gridLayout->addWidget(w,w->m_x,w->m_y,w->m_xSpace,w->m_ySpace);
	}
}

void VideoMgr::UpdateUI()
{
	if(m_bShareDocing == true)
		return;


	if(g_pMeetingFrame->GetRoomMode() == 1)
	{
		//重新排列视频
		ReLayoutAllVideo();
	}
	else
	{
		for(int i = 0; i<13;i++)
		{
			if(i!=m_currentBigVideo){
				m_pVideoList[i]->SetHidden(true);
				gridLayout->removeWidget(m_pVideoList[i]);
			}
			else{
				m_pVideoList[i]->SetHidden(false);
				gridLayout->removeWidget(m_pVideoList[i]);
				gridLayout->addWidget(m_pVideoList[i],0,0,3,3);
			}
		}
	}

}