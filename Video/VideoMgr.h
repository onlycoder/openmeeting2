#ifndef VIDEOMGR_H
#define VIDEOMGR_H

#include <QWidget>
#include <QGridLayout>
#include "VideoWin.h"
class VideoMgr:public QObject
{
	Q_OBJECT
public:
	VideoMgr(QWidget *videoContainer = 0);
	~VideoMgr();
public:
	static VideoMgr* m_instance;
	static VideoMgr* GetInstance();
public:
	IVideoWin* GetFreeVideo();
	void GivebackVideo(uint64_t ssrc);
	void OnRoomModeChange(int mode);
	void UpdateUI();
	void SetBigVideo(uint64_t userID);
	void HideAll();
	void ShowAll();
	void FreeAll();
	QGridLayout* GetLayout();
	public slots:
		void OnVideoWinDbClick(qint64,bool);
private:
	VideoWin* m_pVideoList[13];
	QWidget * m_pVideoContainer;
	QGridLayout* gridLayout;
	int m_currentBigVideo;         //当前主会场
	bool m_bShareDocing;
private:
	void ReLayoutAllVideo();	
};

#endif // VIDEOMGR_H
