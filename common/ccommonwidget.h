#ifndef CCOMMONWIDGET_H
#define CCOMMONWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>
#include <windows.h>

#define DEF_NOCLIENT_SIZE		5

//窗口属性枚举
enum ENUM_CommonWndFlag
{
    enum_GGWndFrameHint			 = 0x00000000,          // 默认
	enum_GGWndTitlebarHint		 = 0x00000001,          // 支持标题栏
    enum_GGWndResizeHint         = 0x00000002,          // 支持拖拽放大缩小
    enum_GGWndMinimizeButtonHint = 0x00000004,          // 支持最小化按钮
    enum_GGWndMaximizeButtonHint = 0x00000008,          // 支持最大化按钮
    enum_GGWndCloseButtonHint    = 0x00000010,          // 有关闭按钮
	enum_GGWndDragMove           = 0x00000020,          // 支持全局拖动
	enum_GGWndNoActive           = 0x00000040,          // 不激活的
	enum_GGWndSysBackground      = 0x00000080,          // 系统背景
	enum_GGWndMenuHint           = 0x00000100,          // 系统菜单

	enum_GGApplication = enum_GGWndMenuHint|enum_GGWndTitlebarHint | enum_GGWndResizeHint | enum_GGWndMinimizeButtonHint| enum_GGWndMaximizeButtonHint | enum_GGWndCloseButtonHint,

};    

Q_DECLARE_FLAGS(CommonWndFlags,ENUM_CommonWndFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(CommonWndFlags)

typedef struct _MARGINS
{
    int cxLeftWidth;      // width of left border that retains its size
    int cxRightWidth;     // width of right border that retains its size
    int cyTopHeight;      // height of top border that retains its size
    int cyBottomHeight;   // height of bottom border that retains its size
} MARGINS, *PMARGINS;

class CCommonWidget : public QWidget
{
	Q_OBJECT

public:
	CCommonWidget(QWidget *parent, CommonWndFlags WndFlag, Qt::WindowFlags f = 0);
	virtual ~CCommonWidget();

	//创建默认标题栏，系统按钮
	void CreateDefault(long lWndFlag);
	//在指定区创建按钮
	void CreateSystemButton(QLayout* layout, CommonWndFlags WndFlag);
	//在指定区创建标题栏
	void CreateSystemTitle(QLayout* layout);

	//设置标题
	void SetTitle(const WCHAR* pwzTitle);
	//设置系统任务栏标题
	void SetTaskBarTitle(const WCHAR* pwzTitle);
	//设置圆角
	void SetRoundCorner(int iSize);
	//窗口居中
	void CenterWindow(QWidget* parent = NULL);
	//设置最前
	void Foreground(bool abShowFirst = true);

private:
	void MakeRoundCorner();

signals:
    void OnClose();

protected slots:
    void OnMaxClick();
    void OnRestoreClick();
	void OnMinClick();
	virtual void OnSysMenuClick();

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
	QToolButton*                                    m_pSysMenuButton;

protected:
	int												m_iRoundCornerSize;
	QMargins										m_oMaxMargins;
};

#endif // CCOMMONWIDGET_H
