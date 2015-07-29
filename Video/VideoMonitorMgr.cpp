#include "VideoMonitorMgr.h"

VideoMonitorMgr* VideoMonitorMgr::m_instance = NULL;
VideoMonitorMgr* VideoMonitorMgr::GetInstance()
{
	return m_instance;
}

VideoMonitorMgr::VideoMonitorMgr(QWidget *videoContainer)
{
	m_bShareDocing = false;
	m_videoMode = 3;
	m_instance = this;
	m_pVideoContainer = videoContainer;
	gridLayout = new QGridLayout();
	m_pVideoContainer->setLayout(gridLayout);
	gridLayout->setMargin(2);
	gridLayout->setSpacing(0);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);
	
	int total = m_videoMode*m_videoMode;
	int model = m_videoMode;
	for(int i = 0; i<64;i++)
	{
		VideoWin * video0 = new VideoWin(m_pVideoContainer);
		m_pVideoList[i] = video0;
		video0->setVisible(false);
		m_pVideoList[i]->SetFree(true);
	}
	
	for(int i = 0; i<total;i++)
	{
		VideoWin * video0 = m_pVideoList[i];
		video0->setVisible(true);
		gridLayout->addWidget(video0,(i/model),(i%model),1,1);
		video0->SetPos((i/model),(i%model),1,1);
	}
	
}


VideoMonitorMgr::~VideoMonitorMgr(void)
{
	
}


void VideoMonitorMgr:: ChangeVideoMode(int mode)
{
	if(m_videoMode == mode)
		return;

	for(int i = 0; i<64;i++)
	{
		m_pVideoList[i]->setVisible(false);
	}

	m_videoMode = mode;
	int total = m_videoMode*m_videoMode;
	for(int i = 0; i<total;i++)
	{
		VideoWin * video0 = m_pVideoList[i];
		video0->setVisible(true);
		gridLayout->addWidget(video0,(i/mode),(i%mode),1,1);
		video0->SetPos((i/mode),(i%mode),1,1);
	}
}

//得到空闲的视频位置
IVideoWin* VideoMonitorMgr::GetFreeVideo()
{
	
	for(int i = 0;i<(m_videoMode*m_videoMode);i++)
	{
			if(m_pVideoList[i]->isFree())
			{
				m_pVideoList[i]->SetFree(false);
				return m_pVideoList[i];
			}
	}
	
	return NULL;
}

void VideoMonitorMgr::GivebackVideo(uint64_t ssrc)
{
	
		for(int i = 0; i<(m_videoMode*m_videoMode);i++)
		{
			if(m_pVideoList[i]->GetUserID() == ssrc)
			{
				m_pVideoList[i]->setUpdatesEnabled(true);
				m_pVideoList[i]->SetFree(true);

			}
		}
	
}