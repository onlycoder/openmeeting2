#ifndef VIDEOMONITORMAIN_H
#define VIDEOMONITORMAIN_H

#include <QWidget>
#include <QTimer>
#include "ui_videomonitormain.h"
#include "../Video/VideoMonitorMgr.h"
#include <map>
#include <vector>
using namespace std;
typedef struct tag_VideoItem
{
	uint64_t userId;
	uint32_t ssrc;
	char userName[64];
	unsigned int hasOpen;
}VideoItem,*PVideoItem;
class VideoMonitorMain : public QWidget
{
	Q_OBJECT

public:
	VideoMonitorMain(QWidget *parent = 0);
	~VideoMonitorMain();

	static VideoMonitorMain * m_instance;
public:
	static VideoMonitorMain * GetInstance();
	void AddVideo(uint64_t userId,uint32_t ssrc);
	void RemoveVideo(uint64_t userId);

	public slots:
		//视频显示模式改变了
		void OnVideoModeChange(int index);
		void OnBtnStartClick();
		void OnBtnStopClick();
		void OnBtnNextClick();
		void OnBtnPrevClick();
		void OnChkAutoPage();
		void OnAutoPageTimer();
private:
	void StopRecvAllVideo();
	void OpenCurrentVideo();
private:
	Ui::VideoMonitorMain ui;
	VideoMonitorMgr* m_videoMgr;
	QTimer* m_Timer;
	vector<PVideoItem> m_videoList;
	//每页显示多少视频
	unsigned int m_countPerPage;
	//一共多少页
	unsigned int m_totalPage;
	//当前页
	unsigned int m_curPage;
};

#endif // VIDEOMONITORMAIN_H
