#pragma once
#include <QWidget>
#include <QGridLayout>
#include "VideoWin.h"

class VideoMonitorMgr
{
public:
	VideoMonitorMgr(QWidget *videoContainer = 0);
	~VideoMonitorMgr(void);
private:
	static VideoMonitorMgr* m_instance;
public:
	static VideoMonitorMgr* GetInstance();
	void ChangeVideoMode(int mode);

public:
	IVideoWin* GetFreeVideo();
	void GivebackVideo(uint64_t ssrc);
private:
	VideoWin* m_pVideoList[64];
	QWidget * m_pVideoContainer;
	QGridLayout* gridLayout;
	int m_currentBigVideo;         //当前主会场
	bool m_bShareDocing;
	unsigned int m_videoMode ;
};

