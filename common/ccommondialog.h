#ifndef CCOMMONDIALOG_H
#define CCOMMONDIALOG_H

#include <QtGui/QDialog>
#include "ccommonwidget.h"

class CCommonDialog : public QDialog
{
	Q_OBJECT

public:
	CCommonDialog(QWidget *parent, CommonWndFlags WndFlag, Qt::WindowFlags f = 0);
	virtual ~CCommonDialog();

	//创建默认标题栏，系统按钮
	void CrateDefault(long lWndFlag);
	//在指定区创建按钮
	void CrateSystemButton(QLayout* layout, CommonWndFlags WndFlag);
	//在指定区创建标题栏
	void CrateSystemTitle(QLayout* layout);

	//设置标题
	void SetTitle(const WCHAR* pwzTitle);
	//设置系统任务栏标题
	void SetTaskBarTitle(const WCHAR* pwzTitle);
	//设置圆角
	void SetRoundCorner(int iSize);
	//窗口居中
	void CenterWindow(QWidget* parent = NULL);

private:
	void MakeRoundCorner();

signals:
    void OnClose();

protected slots:
    void OnMaxClick();
    void OnRestoreClick();
	void OnMinClick();

protected:
	virtual bool winEvent (MSG* msg, long *result);
	virtual void resizeEvent (QResizeEvent* event);
	virtual void paintEvent (QPaintEvent* event);

	virtual void OnPaint(QPainter* painter);

private:
	CommonWndFlags									m_lWndFlag;
	QLabel*											m_pTitleLabel;
	QLayout*										m_pTitleLayout;
	QToolButton*									m_pCloseButton;
	QToolButton*									m_pMaxButton;
	QToolButton*									m_pRestoreButton;
	QToolButton*									m_pMinButton;

private:
	int												m_iRoundCornerSize;
	QMargins										m_oMaxMargins;
};

#endif // CCOMMONDIALOG_H
