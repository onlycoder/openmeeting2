#include "MeetingLoadingWidget.h"
#include <QDebug>
#include <QPainter>
#include <QFile>

MeetingLoadingWidget *MeetingLoadingWidget::load_instance;
MeetingLoadingWidget *MeetingLoadingWidget::GetLoadInstance(){

	if (load_instance== NULL)
	{
		load_instance = new MeetingLoadingWidget();
	}
	return load_instance;
}


MeetingLoadingWidget::MeetingLoadingWidget(QWidget *parent)
	: QWidget(parent)
{


	
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	
    resize(parent->width(),parent->height());
	setStyleSheet("background-color:white");
	 
	this->setObjectName("MeetingLoadingWidget");

	backLabel = new QLabel(this);
	backLabel->setPixmap(QPixmap(":/login/login/Normal_bkg1.png"));
	backLabel->setGeometry(0,0,this->width(),this->height());
	backLabel->setObjectName("backLabel");

	index = 1;
	cancelBtn = new QPushButton(this);
	cancelBtn->setObjectName("cancelBtn");

	QPalette pal;
	pal.setColor(QPalette::ButtonText,QColor(255,255,255));
	cancelBtn->setPalette(pal);

	loadingLabel = new QLabel(this);
	tipLabel = new QLabel(this);

	QPixmap pixmap(QString(":/login/login/VideoOptimization") + QString::number(index, 10));
	loadingLabel->setFixedSize(pixmap.size());
	loadingLabel->setPixmap(pixmap);
	loadingLabel->setScaledContents(true);
	loadingLabel->setObjectName("loadingLabel");

	loadingLabel->setGeometry(this->width()/2 - pixmap.size().width()/2,this->height()/3,pixmap.size().width(),pixmap.size().height());

	connect(cancelBtn,SIGNAL(clicked()),this,SLOT(cancelBtnClicked()));

	timer = new QTimer(this);
	timer->setInterval(100);
	connect(timer, SIGNAL(timeout()), this,SLOT(updatePixmap()));

	QFile qssFile(":/qss/login.qss");
	QString qss;
	qssFile.open(QFile::ReadOnly);
	if(qssFile.isOpen())
	{  
		qss = QString(qssFile.readAll());
		this->setStyleSheet(qss);
		qssFile.close();
	}

}

MeetingLoadingWidget::~MeetingLoadingWidget()
{

}


void MeetingLoadingWidget::startWithBtnTitleAndTipString(bool is_start,QString btnStr,QString tipStr)
{
	int tipWidth = fontMetrics ().width(tipStr);
	int tWidth = tipWidth>30?tipWidth:30;
	
	QPixmap pixmap(QString(":/login/login/VideoOptimization") + QString::number(index, 10));

	if (tipWidth == 0)
	{
		tipLabel->setHidden(true);
	}else{

		tipLabel->setText(tipStr);
		tipLabel->setGeometry(this->width()/2 - tWidth/2,this->height()/2,tWidth,30);
	}

	int btnWidth = fontMetrics().width(btnStr);
	int bWidth = btnWidth>200?btnWidth:200;

	if (btnWidth == 0)
	{
		cancelBtn->setHidden(true);
	} 
	else
	{
		cancelBtn->setText(btnStr);
		cancelBtn->setGeometry(this->width()/2 - bWidth/2,this->height()/2 + 50,bWidth,30);
	}
	if(is_start)
	{
		timer->start();
	}
	else
	{
		timer->stop();
		index = 1;
		loadingLabel->setPixmap(QString(":/login/login/VideoOptimization") + QString::number(index, 10));
	}
}

void MeetingLoadingWidget::updatePixmap()
{
	//若当前图标下标超过8表示到达末尾，则重新计数
	index++;
	if(index > 8)
	{
		index = 1;
	}
	QPixmap pixmap(":/login/login/VideoOptimization"+ QString::number(index, 10));
	loadingLabel->setPixmap(pixmap);
}

/*loading widget cancel*/
void MeetingLoadingWidget::cancelBtnClicked(){

	emit toHiddenLoadWidget();
}