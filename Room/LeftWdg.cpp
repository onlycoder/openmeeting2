#include "LeftWdg.h"
#include <QMenu>
#include <QDebug>
#include <QSound>
#include "../stdafx.h"
#include <QTextCodec>
#include "../Video/VideoMgr.h"
#include "../Setting/SeanGifWdg.h"
#include <QRgb>
#include <QKeyEvent>
#include <QFont>
#include <QColor>
#include <QImage>
#include <QTextCursor>
#include <QUrl>
#include <QTextDocumentFragment>
#include <QPoint>
#include "RoomWdg.h"
#include <QPropertyAnimation.h>


LeftWdg * LeftWdg::m_instance = NULL;;
LeftWdg * LeftWdg::GetInstance()
{
	return m_instance;
}
//捕捉回车键
bool LeftWdg::eventFilter(QObject *obj, QEvent *e){
	Q_ASSERT(obj == inPutTextEdit);

	
		if (e->type() == QEvent::KeyPress)
		{
			
			QKeyEvent *event = static_cast<QKeyEvent*>(e);
			if (event->key() == Qt::Key_Return/* && (event->modifiers() & Qt::ControlModifier)*/)
			{
				btnSendMsgClicked(); //发送消息的槽
				return true;
			}
		}
	return false;
}
LeftWdg::LeftWdg(QWidget *parent)
	: QWidget(parent)
{
	rowCount = 0;
	m_instance = this;
	setupUi(this);
	lvUser->setSelectionMode(QAbstractItemView::ExtendedSelection);
	lvUser->setContextMenuPolicy(Qt::CustomContextMenu );

 	userModel = new CListModel( this);
	lvUser->setModel(userModel);

	CListItemDelegate* pDelegate = new CListItemDelegate(this);
	pDelegate->SetParentUI(this);
	lvUser->viewport()->installEventFilter(pDelegate);
	lvUser->setItemDelegate(pDelegate);
	cmbSpeakMode->setCurrentIndex(0);
	connect(lvUser,SIGNAL(customContextMenuRequested( const QPoint &)), 
		this, SLOT(OnUserListMenuPopup(const QPoint &)));
	connect(cmbSpeakMode,SIGNAL(currentIndexChanged(const QString &)),this,
		SLOT(OnSpeakModeChange(const QString &)));



	connect(btnSendMsg,SIGNAL(clicked()),this,SLOT(btnSendMsgClicked()));
	connect(this,SIGNAL(EventUpdateUserListUI()),this,SLOT(OnUpdateUserListUI()));


	m_pMenu = new QMenu();
	menuStartSpeaker = m_pMenu->addAction((tr("点名发言")));
	menuStopSpeaker = m_pMenu->addAction((tr("关闭发言")));
	menuSetBigVideo = m_pMenu->addAction((tr("设为主会场")));
	connect(menuStartSpeaker,SIGNAL(triggered()),this,SLOT(OnStartSpeaker()));
	connect(menuStopSpeaker,SIGNAL(triggered()),this,SLOT(OnStopSpeaker()));
	connect(menuSetBigVideo,SIGNAL(triggered()),this,SLOT(OnSetBigVideo()));
	               
	//connect(btnFont,SIGNAL(clicked()),this,SLOT(btnFontClicked()));
	 //connect(gifBtn,SIGNAL(clicked()),this,SLOT(slotOfGifBtnClicked()));
	 connect(inPutTextEdit,SIGNAL(textChanged()),this,SLOT(slotOfQtetEditChanged()));
	 connect(SeanGifWdg::GetInstance(),SIGNAL(EventLeave()),this,SLOT(slotOfLeaveGifwdg()));
	//SeanFontSetting *fontWdg = new SeanFontSetting(chatToolBar);

	 QStringList  fontList;
	 fontList<<"微软雅黑"<<"仿宋"<<"黑体"<<"华文隶书"<<"华文仿宋"<<"华文楷体"<<"隶书"<<"楷书";
	 fontBox->addItems(fontList);


	 QStringList sizeList;
	 sizeList<<"9"<<"10"<<"11"<<"12"<<"13"<<"14";
	 sizeBox->addItems(sizeList);

	 connect(fontBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(slotOfFontChanged(QString)));
	 connect(sizeBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(slotOfSizeChanged(QString)));
	 connect(colorBtn,SIGNAL(clicked()),this,SLOT(slotOfColorChangetd()));
	 connect(gifBtn,SIGNAL(clicked()),this,SLOT(slotOfGifBtnClicked()));


	inPutTextEdit->installEventFilter(this);
	inPutTextEdit->setFocus();
	inPutTextEdit->acceptRichText();

	//将输出栏设置为只读
	outPutTextEdit->setReadOnly(true);

	cmbSpeakMode->setVisible(false);
	fontStr = "微软雅黑";
	sizeStr = "9";
	colorValue = 4278190080;
	gifIsShow = false;
	setMouseTracking(true);
}

void LeftWdg::slotOfFontChanged(QString getFont)
{
	fontStr = getFont;
	QFont qFont = QFont(getFont);
    sFont = qFont;
	inPutTextEdit->setCurrentFont(qFont);
}
void LeftWdg::slotOfSizeChanged(QString getSize)
{
	QFont qFont(fontStr,getSize.toInt());
	inPutTextEdit->setCurrentFont(qFont);
	sizeStr = getSize;
}

void LeftWdg::slotOfColorChangetd(){

    
	QColor nColor = QColorDialog::getColor();
	sColor = nColor;
	inPutTextEdit->setTextColor(sColor);

}
void LeftWdg::slotOfGifBtnClicked(){

	if (gifIsShow)
	{
		toHiddenGifwdg();
	}else
	{
		toShowGifwdg();
	}
}
void LeftWdg::slotOfLeaveGifwdg(){
	toHiddenGifwdg();
	gifIsShow = false;
}

void LeftWdg::toShowGifwdg(){

	QPoint GlobalPoint(chatToolBar->mapToGlobal(QPoint(0, 0)));//获取控件在窗体中的坐标
	int x = GlobalPoint.x();
	int y = GlobalPoint.y(); 


	QPropertyAnimation *animation = new QPropertyAnimation(SeanGifWdg::GetInstance(), "geometry");
	animation->setDuration(400);
	animation->setStartValue(QRect(x + chatToolBar->geometry().width(),y, SeanGifWdg::GetInstance()->geometry().width()/4,SeanGifWdg::GetInstance()->geometry().height()));
	animation->setEndValue(QRect(x + chatToolBar->geometry().width(),y - SeanGifWdg::GetInstance()->geometry().height(),SeanGifWdg::GetInstance()->geometry().width(),SeanGifWdg::GetInstance()->geometry().height()));
	animation->start();
	SeanGifWdg::GetInstance()->show();
	gifIsShow = true;
}
void LeftWdg::toHiddenGifwdg(){

	SeanGifWdg::GetInstance()->hide();
	gifIsShow = false;
}
void LeftWdg::slotOfGifNumberGet(unsigned long long num)
{
	//QString imStr = QString(":/gif/gif/00%1[24x24x8BPP].gif").arg(389+num);
	QString imStr = QString(":/setting/setting/0%1[24x24x8BPP].png").arg(3502+num);
	QTextDocumentFragment fragment; 
	fragment = QTextDocumentFragment::fromHtml(QString("<img src='%1'>").arg(imStr)); 
	inPutTextEdit->textCursor().insertFragment(fragment); 
	//inPutTextEdit->append(gStr);
	inPutTextEdit->setVisible(true); 

}
//限制输入字数
void LeftWdg::slotOfQtetEditChanged(){

	QString textContent = inPutTextEdit->toPlainText();
	int length = textContent.count(); //字符数
	if(length > MAX_INTPUT)
	{
		int position = inPutTextEdit->textCursor().position();
		QTextCursor textCursor = inPutTextEdit->textCursor();
		textContent.remove(position - (length - MAX_INTPUT), length - MAX_INTPUT);
		inPutTextEdit->setText(textContent);
		textCursor.setPosition(position - (length - MAX_INTPUT));
		inPutTextEdit->setTextCursor(textCursor);
	}
	
}
LeftWdg::~LeftWdg()
{

}

void LeftWdg::OnAddUser()
{
	if(g_pMeetingFrame!=NULL)
	{
		userModel->insertRows(0,1);
	}
	lvUser->update();
	
}
void LeftWdg::OnDelUser()
{
	if(g_pMeetingFrame!=NULL)
	{
		userModel->removeRows(0,1);
	}
	lvUser->update();
}


void LeftWdg::OnSpeakModeChange(const QString &strItem)
{
	if(strItem==QString(tr("按键说话")))
	{
		lblSpeakerStatus->setPixmap(QPixmap(":/skin/room/afsh_1.png"));
		g_pMeetingFrame->SetSpeakMode(2);
		if(g_pMeetingFrame->HasStartAudio() == true)
			g_pMeetingFrame->StopPublishAudio(0);
	}
	else if(strItem == QString(tr("主持人点名")))
	{
		lblSpeakerStatus->setPixmap(QPixmap(":/skin/room/zcrkzsh_1.png"));
		g_pMeetingFrame->SetSpeakMode(3);
		if(g_pMeetingFrame->HasStartAudio() == true)
			g_pMeetingFrame->StopPublishAudio(0);
	}
	else if(strItem == QString(tr("自由说话")))
	{
		lblSpeakerStatus->setPixmap(QPixmap(":/skin/room/imgSpeakStatus.png"));
		g_pMeetingFrame->SetSpeakMode(1);
		if(g_pMeetingFrame->HasStartAudio() == false)
			g_pMeetingFrame->StartPublishAudio(0,AppSetting::GetInstance()->GetMicIndex()-1);
	}
}
void LeftWdg::OnUserListMenuPopup(const QPoint &pos)
{
	if(g_pMeetingFrame->GetUserRole(0)==1)
	{
		QModelIndex loModelIndex =lvUser->indexAt(pos);
		Client_Online_User_Info userInfo;
		if(g_pMeetingFrame->GetUser(loModelIndex.row(),&userInfo))
		{
			if(userInfo.sessionID == g_pMeetingFrame->GetMySessionID())
			{
				if(g_pMeetingFrame->HasStartAudio())
				{
					menuStartSpeaker->setEnabled(false);
					menuStopSpeaker->setEnabled(true);
				}
				else
				{
					menuStartSpeaker->setEnabled(true);
					menuStopSpeaker->setEnabled(false);
				}

				if(g_pMeetingFrame->HasStartVideo()&&g_pMeetingFrame->GetRoomMode() == 1)
					menuSetBigVideo->setEnabled(true);
				else
					menuSetBigVideo->setEnabled(false);
			}
			else
			{
				if(userInfo.ulAudioSSRC>0)
				{
					menuStartSpeaker->setEnabled(false);
					menuStopSpeaker->setEnabled(true);
				}
				else
				{
					menuStartSpeaker->setEnabled(true);
					menuStopSpeaker->setEnabled(false);
				}

				if(userInfo.ulVideoSSRC>0)
					menuSetBigVideo->setEnabled(true);
				else
					menuSetBigVideo->setEnabled(false);
			}

			if(loModelIndex.isValid())
			{
				m_pMenu->exec(lvUser->mapToGlobal( pos ));
			}
		}
	}
}

void LeftWdg::OnStartSpeaker()
{
	
	QModelIndex loModelIndex =lvUser->indexAt(lvUser->mapFromGlobal(m_pMenu->pos()));
	Client_Online_User_Info userInfo;
	if(g_pMeetingFrame->GetUser(loModelIndex.row(),&userInfo))
	{
		if(userInfo.sessionID == g_pMeetingFrame->GetMySessionID())
		{
			g_pMeetingFrame->StartPublishAudio(0,AppSetting::GetInstance()->GetMicIndex()-1);
		}
		g_pMeetingFrame->SetAsSpeaker(userInfo.sessionID,true);
	}
	else
	{
		qDebug()<<"OnStartSpeaker not find user item";
	}
}

void LeftWdg::OnStopSpeaker()
{
	QModelIndex loModelIndex =lvUser->indexAt(lvUser->mapFromGlobal(m_pMenu->pos()));
	Client_Online_User_Info userInfo;
	if(g_pMeetingFrame->GetUser(loModelIndex.row(),&userInfo))
	{
		if(userInfo.sessionID == g_pMeetingFrame->GetMySessionID())
		{
			g_pMeetingFrame->StopPublishAudio(0);
		}

		g_pMeetingFrame->SetAsSpeaker(userInfo.sessionID,false);
	}
	else
	{
		qDebug()<<"OnStopSpeaker not find user item";
	}
}

void LeftWdg::OnSetBigVideo()
{
	QModelIndex loModelIndex =lvUser->indexAt(lvUser->mapFromGlobal(m_pMenu->pos()));
	Client_Online_User_Info userInfo;
	if(g_pMeetingFrame->GetUser(loModelIndex.row(),&userInfo))
	{
		if(userInfo.sessionID == g_pMeetingFrame->GetMySessionID())
		{
			VideoMgr::GetInstance()->SetBigVideo(0);
			g_pMeetingFrame->SetAsBigVideo(g_pMeetingFrame->GetMySessionID());
		}
		else
		{
			VideoMgr::GetInstance()->SetBigVideo(userInfo.sessionID);
			g_pMeetingFrame->SetAsBigVideo(userInfo.sessionID);
		}
		
	}
	else
	{
		qDebug()<<"OnStopSpeaker not find user item";
	}
}

void LeftWdg::UpdateUI()
{
	ROOM_INFO roomInfo;
	if(g_pMeetingFrame->GetCurrentRoomInfo(&roomInfo))
	{
		switch(roomInfo.speakMode)
		{
		case 1:
			{
				cmbSpeakMode->setCurrentIndex(0);
				lblSpeakerStatus->setPixmap(QPixmap(":/skin/room/imgSpeakStatus.png"));
			}
			break;
		case 2:
			{
				cmbSpeakMode->setCurrentIndex(1);
				lblSpeakerStatus->setPixmap(QPixmap(":/skin/room/afsh_1.png"));
			}
			break;
		case 3:
			{
				cmbSpeakMode->setCurrentIndex(2);
				lblSpeakerStatus->setPixmap(QPixmap(":/skin/room/zcrkzsh_1.png"));
			}
			break;
		}
	}
	if(g_pMeetingFrame->GetUserRole(0) == 1)
	{
		cmbSpeakMode->setVisible(true);
		
	}
	else
	{
		cmbSpeakMode->setVisible(false);
		
	}
	lblUserNum->setText(QString("房间内成员(%1)").arg(g_pMeetingFrame->GetUserCount()));
	this->update();
}
//消息发送slot
void LeftWdg::btnSendMsgClicked()
{
	QTextCodec::setCodecForTr(QTextCodec::codecForName("GBK"));
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));

	if (inPutTextEdit->toPlainText().isEmpty())
	{
		inPutTextEdit->setFocus();
		return;
	}
	QString qStr = inPutTextEdit->toHtml();  
	//含有中文 qstring转char
	QByteArray ba = qStr.toLocal8Bit();  
	char* chaMessage = ba.data();

	//qDebug()<<"chaMessage=="<<chaMessage;
	if(g_pMeetingFrame!=NULL)
	{
		isSend = true;
		QByteArray fa = fontStr.toLocal8Bit();
		char *chaFont = fa.data();
		 g_pMeetingFrame->SendTextMsg(0,chaMessage,sizeStr.toInt(),0,colorValue,chaFont);

        QString *mStr = new QString(chaMessage);
	    QString *fStr = new QString(chaFont);
	    editHtmlWithArgument(0,mStr,sizeStr.toInt(),0,colorValue,fStr);
        
	}
	inPutTextEdit->clear();
	inPutTextEdit->setFocus();
}

/*收到消息*/
void LeftWdg::getMessageFromOthers(unsigned long long ulUserID,QString* strMsg, 
	int font_size,unsigned long fontFlat,unsigned long color,QString *familly){
	//闪烁窗口，以提醒有新消息到了
		::FlashWindow(RoomWdg::GetInstance()->winId(),true);
		QSound::play("newmsg.wav");
	isSend = false;
	editHtmlWithArgument(ulUserID,strMsg,font_size,fontFlat,color,familly);
	strMsg = NULL;
	familly = NULL;
}
//发送或接收消息处理
void LeftWdg::editHtmlWithArgument(unsigned long long ulUserID,QString* strMsg, 
	int font_size,unsigned long fontFlat,unsigned long color,QString *familly){

		QString tColor;
		if (isSend)
		{
			tColor = "green";
		}else{
		tColor ="blue";
		}
		QString dateTime = getCurrentDateTime();
		QString temp = QString("<font size=\"4\" color=%1>%2    %3:</font> <font size=\"%4\">%5</font>")
 			.arg(tColor).arg(QString::fromUtf8(g_pMeetingFrame->GetUserName(ulUserID))).arg(dateTime).arg(font_size - 6).arg(*strMsg);
		outPutTextEdit->append(temp);
		
}
void LeftWdg::OnUpdateUserListUI()
{
	lvUser->update();    
}
void LeftWdg::SetAudioValue(unsigned int ulSSRC,short value)
{
	QMutexLocker autoLock(&m_Mutex);
	std::map<unsigned int ,short>::iterator item = m_mapAudioEnergy.find(ulSSRC);
	if(item == m_mapAudioEnergy.end())
	{
		m_mapAudioEnergy.insert(std::map<unsigned int ,short>::value_type(ulSSRC,value));
	}
	else
	{
		item->second = value;
	}
	
	emit EventUpdateUserListUI();
}

short LeftWdg::GetAudioValue(unsigned int ulSSRC)
{
	QMutexLocker autoLock(&m_Mutex);
	std::map<unsigned int ,short>::iterator item = m_mapAudioEnergy.find(ulSSRC);
	if(item!=m_mapAudioEnergy.end())
	{
		return item->second;
	}
	return 0;
}
//获取时间
QString LeftWdg::getCurrentDateTime()
{
	QTime time = QTime::currentTime();
	QDate date = QDate::currentDate();

	return QString("%1  %2").arg(date.toString(Qt::ISODate))
		.arg(time.toString(Qt::ISODate));
}