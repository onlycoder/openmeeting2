#include "SeanPptProgressWdg.h"
#include <QFile>
#include "Room/RoomWdg.h"

SeanPptProgressWdg::SeanPptProgressWdg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QFile qssFile(":/qss/setting.qss");
	QString qss;
	qssFile.open(QFile::ReadOnly);
	if(qssFile.isOpen())
	{  
		qss = QString(qssFile.readAll());
		this->setStyleSheet(qss);
		qssFile.close();
	}

	index = 1;
	QPixmap pixmap(QString(":/setting/setting/00900"));
	ui.imageLabel->setPixmap(pixmap);
	ui.imageLabel->setScaledContents(true);

	timer = new QTimer(this);
	timer->setInterval(100);
	connect(timer, SIGNAL(timeout()), this,SLOT(updatePixmap()));
 	timer->start();
}

SeanPptProgressWdg::~SeanPptProgressWdg()
{

}
void SeanPptProgressWdg::closeWidget(){
	/*
	if (!RoomWdg::GetInstance()->replyGet->isFinished())
	{
		RoomWdg::GetInstance()->replyGet->close();
	}
	*/
	ui.progressLabel->setText(QString("正在上传 (%1%2)").arg("0").arg("%"));
	update();
	close();
}
void SeanPptProgressWdg::updatePixmap(){

	//若当前图标下标超过8表示到达末尾，则重新计数
	index++;
	if(index > 17)
	{
		index = 1;
	}
	QPixmap pixmap(QString(":/setting/setting/00%1").arg(900 + index));
	ui.imageLabel->setPixmap(pixmap);
}
void SeanPptProgressWdg::updateProgressWithFloat(QString uploadStr){

	ui.progressLabel->setText(uploadStr);
	update();
}