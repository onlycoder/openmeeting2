#ifndef LEFTWDG_H
#define LEFTWDG_H

#define  MAX_INTPUT 512
#include <QWidget>
#include <QPoint>
#include "ui_LeftWdg.h"
#include "../UserList/CListModel.h"
#include "../UserList/CListItemDelegate.h"
#include "stdafx.h"
#include <QDateTime.h>
#include "../ChatList/ChatListModel.h"
#include "../ChatList/ChatListModelDelegate.h"
#include <map>
#include <QMutex>
#include <QMutexLocker>
#include <QColor>
#include <QColorDialog>
#include <QFont>
#include <QEvent>
#include <QMouseEvent>

class LeftWdg : public QWidget, public Ui::LeftWdg
{
	Q_OBJECT


public:
	static LeftWdg * m_instance;
	static LeftWdg * GetInstance();
	void UpdateUI();
	void SetAudioValue(unsigned int ulSSRC,short value);
	short GetAudioValue(unsigned int ulSSRC);
	QString getCurrentDateTime();
signals:
	void EventUpdateUserListUI();
protected slots:
			void OnUserListMenuPopup( const QPoint &pos);
			void OnSpeakModeChange(const QString &);
			void OnAddUser();
			void OnDelUser();
			void OnStartSpeaker();   //点名发言
			void OnStopSpeaker();    //关闭发言
			void OnSetBigVideo();    //设置成主会场
			//收到消息 slot
			void getMessageFromOthers(unsigned long long ulUserID,QString* strMsg, 
				int font_size,unsigned long fontFlat,unsigned long color,QString *familly);
			void OnUpdateUserListUI();

public:
	LeftWdg(QWidget *parent = 0);
	~LeftWdg();
private:
	CListModel* userModel;
	QMenu *m_pMenu;
	ChatListModel *chatModel;
	QDateTime nowDateTime;
	int rowCount;
public slots:
	void btnSendMsgClicked();
	//add by sean
	void slotOfFontChanged(QString getFont);
	void slotOfSizeChanged(QString getSize);
	void slotOfColorChangetd();
	void slotOfGifBtnClicked();

	void slotOfGifNumberGet(unsigned long long num);
	void slotOfQtetEditChanged();
	void slotOfLeaveGifwdg();


private:
	QMutex m_Mutex;
	std::map<unsigned int ,short> m_mapAudioEnergy;

	QAction * menuStartSpeaker;
	QAction * menuStopSpeaker;
	QAction * menuSetBigVideo;
	//add by sean
	bool fontBool;
	QFont sFont;
	QColor sColor;
	bool gifIsShow;
	bool isSend;
	QString sendMessage;
	void editHtmlWithArgument(unsigned long long ulUserID,QString* strMsg, 
		int font_size,unsigned long fontFlat,unsigned long color,QString *familly);
	void toShowGifwdg();
	void toHiddenGifwdg();
public:
	//add by sean
	QString fontStr;
	QString sizeStr;
	unsigned long colorValue;

protected:
	bool eventFilter(QObject *obj, QEvent *e);

};

#endif // LEFTWDG_H
