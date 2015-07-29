#include "ccommonwidget.h"
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include "qpainter.h"
#include "QStyleOption.h"
#include "OSInfo.h"
#define  TITLE_HEIGHT 88
CCommonWidget::CCommonWidget(QWidget *parent, CommonWndFlags WndFlag, Qt::WindowFlags f)
	: QWidget(parent, f | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
{
	m_lWndFlag = WndFlag;
	m_pTitleLabel = NULL;
	m_pCloseButton = NULL;
	m_pMaxButton = NULL;
	m_pRestoreButton = NULL;
	m_pMinButton = NULL;
	m_iRoundCornerSize = 6;
	if(m_lWndFlag & enum_GGWndResizeHint)
	{
		::SetWindowLongA(winId(),GWL_STYLE, WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		::SetWindowLongA(winId(),GWL_EXSTYLE, WS_EX_APPWINDOW);
	}
	else
	{
		::SetWindowLongA(winId(),GWL_STYLE, WS_POPUP | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		if(m_lWndFlag & (enum_GGWndMinimizeButtonHint | enum_GGWndMaximizeButtonHint))
		{
			::SetWindowLongA(winId(),GWL_EXSTYLE, WS_EX_APPWINDOW);
		}
	}
	//::SetWindowPos(winId(),NULL,0,0,0,0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

  	BOOL bSetTheme = TRUE;
	OSVERSIONINFOW osinfo = {0};
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
	if(::GetVersionExW(&osinfo))
	{
		if(osinfo.dwMajorVersion <= 5)
			bSetTheme = FALSE;
	}

	HRESULT hr = S_OK;
	//去Win7边框
	if(bSetTheme)
	{
		HINSTANCE hInst = LoadLibraryW(L"UxTheme.dll");
		if (hInst)
		{
			typedef HRESULT (WINAPI *PFUN_SetWindowTheme)(HWND, LPCWSTR, LPCWSTR);
			PFUN_SetWindowTheme pFun = (PFUN_SetWindowTheme)GetProcAddress(hInst, "SetWindowTheme");
			if (pFun)
				hr = pFun(winId(), L"", L"");        //去掉xp主题
			FreeLibrary(hInst);
		}
	
		hInst = LoadLibraryW(L"dwmapi.dll");
        if(hInst)
        {
            typedef HRESULT (WINAPI * TmpFun)(HWND,DWORD,LPCVOID,DWORD);
            TmpFun DwmSetWindowAttributeEX = (TmpFun)::GetProcAddress(hInst, "DwmSetWindowAttribute");
            if (DwmSetWindowAttributeEX)
            {
                DWORD dwAttr = 1;
				hr = DwmSetWindowAttributeEX(winId(),2,&dwAttr, 4);    //去掉vista特效
            }

			MARGINS margins = {-1};
			typedef HRESULT (WINAPI * TmpFun1)(HWND,MARGINS*);
			TmpFun1 DwmExtendFrameIntoClientArea = (TmpFun1)::GetProcAddress(hInst, "DwmExtendFrameIntoClientArea");
			if (DwmExtendFrameIntoClientArea)
			{
				DwmExtendFrameIntoClientArea(winId(),&margins);
			}
            FreeLibrary(hInst);
        }
	}
	//::SetWindowPos(winId(),NULL,0,0,0,0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	if(m_lWndFlag & enum_GGWndSysBackground)
		setAttribute(Qt::WA_StyledBackground);

	if(m_lWndFlag & enum_GGWndNoActive)
		setAttribute(Qt::WA_ShowWithoutActivating);
}

CCommonWidget::~CCommonWidget()
{

}

//创建默认标题栏，系统按钮
void CCommonWidget::CreateDefault(long lWndFlag)
{
}

//在指定区创建按钮
void CCommonWidget::CreateSystemButton(QLayout* layout, CommonWndFlags WndFlag)
{
	if(WndFlag & enum_GGWndMenuHint)
	{
		m_pSysMenuButton = new QToolButton(this);
		m_pSysMenuButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		m_pSysMenuButton->setObjectName(QString::fromUtf8("SysMenuButton"));
		m_pSysMenuButton->setToolTip(QString::fromUtf16(L"系统菜单"));
		layout->addWidget(m_pSysMenuButton);
		connect(m_pSysMenuButton,SIGNAL(clicked()), this, SLOT(OnSysMenuClick()));
	}
	if(WndFlag & enum_GGWndMinimizeButtonHint)
	{
		m_pMinButton = new QToolButton(this);
		m_pMinButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		m_pMinButton->setObjectName(QString::fromUtf8("SystemMinButton"));
		m_pMinButton->setToolTip(QString::fromUtf16(L"最小化"));
		layout->addWidget(m_pMinButton);
		connect(m_pMinButton,SIGNAL(clicked()), this, SLOT(OnMinClick()));
	}

	if(WndFlag & enum_GGWndMaximizeButtonHint)
	{
		m_pMaxButton = new QToolButton(this);
		m_pMaxButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		m_pMaxButton->setObjectName(QString::fromUtf8("SystemMaxButton"));
		m_pMaxButton->setToolTip(QString::fromUtf16(L"最大化"));
		layout->addWidget(m_pMaxButton);
		m_pRestoreButton = new QToolButton(this);
		m_pRestoreButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		m_pRestoreButton->setObjectName(QString::fromUtf8("SystemRestoreButton"));
		m_pRestoreButton->setToolTip(QString::fromUtf16(L"还原"));
		m_pRestoreButton->hide();
		layout->addWidget(m_pRestoreButton);
		connect(m_pMaxButton,SIGNAL(clicked()), this, SLOT(OnMaxClick()));
		connect(m_pRestoreButton,SIGNAL(clicked()), this, SLOT(OnRestoreClick()));
	}

	if(WndFlag & enum_GGWndCloseButtonHint)
	{
		m_pCloseButton = new QToolButton(this);
		m_pCloseButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		m_pCloseButton->setObjectName(QString::fromUtf8("SystemCloseButton"));
		m_pCloseButton->setToolTip(QString::fromUtf16(L"关闭"));
		layout->addWidget(m_pCloseButton);
		connect(m_pCloseButton,SIGNAL(clicked()), this, SIGNAL(OnClose()));
	}
}

void CCommonWidget::CreateSystemTitle(QLayout* layout)
{
	m_pTitleLayout = layout;
	m_pTitleLabel = new QLabel(this);
	if(m_pTitleLabel)
	{
		m_pTitleLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
		m_pTitleLabel->setObjectName(QString::fromUtf8("SystemTitleLabel"));
		layout->addWidget(m_pTitleLabel);
	}
}

//设置标题
void CCommonWidget::SetTitle(const WCHAR* pwzTitle)
{
	if(m_pTitleLabel)
	{
		m_pTitleLabel->setText(QString::fromUtf16(pwzTitle));
	}
}

//设置系统任务栏标题
void CCommonWidget::SetTaskBarTitle(const WCHAR* pwzTitle)
{
	setWindowTitle(QString::fromUtf16(pwzTitle));
}

//设置圆角
void CCommonWidget::SetRoundCorner(int iSize)
{
	if(iSize < 0)
		return;

	m_iRoundCornerSize = iSize;
	if(iSize > 0)
	{
		MakeRoundCorner();
	}
	else
	{
		::SetWindowRgn(winId(),NULL,TRUE);
	}
}

//窗口居中
void CCommonWidget::CenterWindow(QWidget* parent)
{
	int x = 0;
	int y = 0;
	if(NULL == parent)
	{
		const QRect rect = QApplication::desktop()->availableGeometry();
		x = rect.left() + (rect.width() - width()) / 2;
		y = rect.top() + (rect.height() - height()) / 2;
	}
	else
	{
		QPoint point(0,0);
		point = parent->mapToGlobal(point);
		x = point.x() + (parent->width() - width()) / 2;
		y = point.y() + (parent->height() - height()) / 2;
	}
	move(x,y);
}

//设置最前
void CCommonWidget::Foreground(bool abShowFirst/* = true*/)
{
	if(abShowFirst)
	{
		show();
	}
	SwitchToThisWindow(winId(), TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCommonWidget::OnMaxClick()
{
	showMaximized();
}

void CCommonWidget::OnRestoreClick()
{
	showNormal();
}

void CCommonWidget::OnMinClick()
{
	showMinimized();
}

void CCommonWidget::OnSysMenuClick()
{

}
bool CCommonWidget::winEvent (MSG* msg, long *result)
{
	switch(msg->message)
	{
		case WM_INPUTLANGCHANGEREQUEST:
			{
				HKL hkl = (HKL)msg->lParam;  
				//激活对应输入法
				ActivateKeyboardLayout(hkl, KLF_REPLACELANG);

				//不允许默认处理，否则某些输入法会导致程序卡死
				*result = 0;
			}
			return true;

		case WM_GETMINMAXINFO:
			{
				MINMAXINFO *mmi = (MINMAXINFO *)msg->lParam;
				const QRect rect = QApplication::desktop()->availableGeometry();
				RECT clientRect,wndRect;
				POINT point = {0,0};
				::GetClientRect(winId(),&clientRect);
				::ClientToScreen(winId(),&point);
				::OffsetRect(&clientRect,point.x,point.y);
				::GetWindowRect(winId(),&wndRect);
				if(clientRect.right - clientRect.left > 0)
				{
					int leftOffset = clientRect.left - wndRect.left;
					int topOffset = clientRect.top - wndRect.top;
					int rightOffset = wndRect.right - clientRect.right;
					int bottomOffset = wndRect.bottom - clientRect.bottom;
					m_oMaxMargins.setLeft(leftOffset);
					m_oMaxMargins.setTop(topOffset);
					m_oMaxMargins.setRight(rightOffset);
					m_oMaxMargins.setBottom(bottomOffset);
					mmi->ptMaxPosition.x = rect.left() - leftOffset;
					mmi->ptMaxPosition.y = rect.top()  - topOffset;
					mmi->ptMaxSize.x = rect.width() + leftOffset + rightOffset;
					mmi->ptMaxSize.y = rect.height() + topOffset + bottomOffset;
					mmi->ptMaxTrackSize.x = mmi->ptMaxSize.x;
					mmi->ptMaxTrackSize.y = mmi->ptMaxSize.y;
				}
				else
				{
					mmi->ptMaxPosition.x = rect.left() - m_oMaxMargins.left();
					mmi->ptMaxPosition.y = rect.top()  - m_oMaxMargins.top();
					mmi->ptMaxSize.x = rect.width() + m_oMaxMargins.left() + m_oMaxMargins.right();
					mmi->ptMaxSize.y = rect.height() + m_oMaxMargins.top() + m_oMaxMargins.bottom();
					mmi->ptMaxTrackSize.x = mmi->ptMaxSize.x;
					mmi->ptMaxTrackSize.y = mmi->ptMaxSize.y;
				}
			}
			break;

		case WM_NCHITTEST:
			{
				QPoint absPos(LOWORD(msg->lParam), HIWORD(msg->lParam));
				QPoint pos = mapFromGlobal(absPos);
				int x = pos.x();
				int y = pos.y();
				int w = width();
				int h = height();
				if(x >= 0 && x <= w && y >= 0 && y <= h)
				{
					if((m_lWndFlag & enum_GGWndResizeHint) && !isMaximized() && !isMinimized())
					{
						if(x < DEF_NOCLIENT_SIZE)
						{
							if(y < DEF_NOCLIENT_SIZE)
							{
								*result = HTTOPLEFT;
							}
							else if(y > h - DEF_NOCLIENT_SIZE)
							{
								*result = HTBOTTOMLEFT;
							}
							else
							{
								*result = HTLEFT;
							}
						}
						else if(x > w - DEF_NOCLIENT_SIZE)
						{
							if(y < DEF_NOCLIENT_SIZE)
							{
								*result = HTTOPRIGHT;
							}
							else if(y > h - DEF_NOCLIENT_SIZE)
							{
								*result = HTBOTTOMRIGHT;
							}
							else
							{
								*result = HTRIGHT;
							}
						}
						else if(y < DEF_NOCLIENT_SIZE)
						{
							*result = HTTOP;
						}
						else if(y > h - DEF_NOCLIENT_SIZE)
						{
							*result = HTBOTTOM;
						}
						else if(y < TITLE_HEIGHT)
						{
							if(m_pTitleLabel)
							{
								const QRect rcTitle = m_pTitleLayout->geometry();
								if(x > rcTitle.x() && x < rcTitle.right() && y > rcTitle.y() && y < rcTitle.bottom())
								{
									*result = HTCAPTION;
								}
								else
								{
									*result = HTCLIENT;
								}
							}
							else
							{
								*result = HTCLIENT;
							}
						}
						else
						{
							*result = HTCLIENT;
						}
					}
					else
					{
						if(y < TITLE_HEIGHT)
						{
							if(m_pTitleLabel)
							{
								const QRect rcTitle = m_pTitleLayout->geometry();
								if(x > rcTitle.x() && x < rcTitle.right() && y > rcTitle.y() && y < rcTitle.bottom())
								{
									*result = HTCAPTION;
								}
								else
								{
									*result = HTCLIENT;
								}
							}
							else
							{
								*result = HTCLIENT;
							}
						}
						else
						{
							*result = HTCLIENT;
						}
					}
				}
				else
				{
					return false;
				}
			}
			return true;

		case WM_NCLBUTTONDBLCLK:
			{
				BOOL bCaption = FALSE;
				if((m_lWndFlag & (enum_GGWndResizeHint | enum_GGWndMaximizeButtonHint)))
				{
					QPoint absPos(LOWORD(msg->lParam), HIWORD(msg->lParam));
					QPoint pos = mapFromGlobal(absPos);
					int x = pos.x();
					int y = pos.y();
					int w = width();
					int h = height();
					if(y < TITLE_HEIGHT && x >= DEF_NOCLIENT_SIZE && x <= w - DEF_NOCLIENT_SIZE)
					{
						if(m_pTitleLabel)
						{
							const QRect rcTitle = m_pTitleLayout->geometry();
							if(x > rcTitle.x() && x < rcTitle.right() && y > rcTitle.y() && y < rcTitle.bottom())
							{
								bCaption = TRUE;
							}
						}
					}
				}

				if(bCaption)
				{
					if(isMaximized())
					{
						showNormal();
					}
					else if(isMinimized())
					{
						showNormal();
					}
					else
					{
						showMaximized();
					}
				}
			}
			return true;

		case WM_NCLBUTTONDOWN:
			if(isMaximized())
			{
				if(!COSInfo::IsAboveWinXP())
					return true;
			}
			break;

		case WM_NCCALCSIZE:
			if(msg->wParam)
			{
				NCCALCSIZE_PARAMS* lpncsp = (NCCALCSIZE_PARAMS*)msg->lParam;
				if(lpncsp->rgrc[0].right - lpncsp->rgrc[0].left > 10)
				{
					lpncsp->rgrc[0].left += 1;
					lpncsp->rgrc[0].top += 1;
					lpncsp->rgrc[0].right -= 1;
					lpncsp->rgrc[0].bottom -= 1;
				}
				//*result = WVR_ALIGNTOP | WVR_ALIGNLEFT | WVR_ALIGNBOTTOM | WVR_ALIGNRIGHT;
			}
			*result = 0;
			return true;

		case WM_ACTIVATE:
			{
				MARGINS margins = {-1};
				HINSTANCE hInst = LoadLibraryW(L"dwmapi.dll");
				if (hInst)
				{
					typedef HRESULT (WINAPI * TmpFun)(HWND,MARGINS*);
					TmpFun DwmExtendFrameIntoClientArea = (TmpFun)::GetProcAddress(hInst, "DwmExtendFrameIntoClientArea");
					if (DwmExtendFrameIntoClientArea)
					{
						DwmExtendFrameIntoClientArea(winId(),&margins);
					}
					FreeLibrary(hInst);
				}
				::SetWindowLongA(winId(),GWL_EXSTYLE, WS_EX_LEFT);
			}
			break;

		case WM_NCACTIVATE:
		case 0x00AE:
		case 0x00AF:
			*result = 0;
			return false;
	}

	return false;
}

void CCommonWidget::resizeEvent (QResizeEvent* event)
{
	if(isMaximized())
	{
		if(m_pMaxButton)
			m_pMaxButton->hide();
		if(m_pRestoreButton)
			m_pRestoreButton->show();
		MakeRoundCorner();
	}
	else if(isMinimized())
	{

	}
	else
	{
		if(m_pRestoreButton)
			m_pRestoreButton->hide();
		if(m_pMaxButton)
			m_pMaxButton->show();
		MakeRoundCorner();
	}
	
}

void CCommonWidget::paintEvent(QPaintEvent* event)
{
	if(m_lWndFlag & enum_GGWndSysBackground)
	{
		QWidget::paintEvent(event);
	}
	else
	{
		QPainter painter(this);
		QStyleOption opt;
		opt.initFrom(this);
		style()->drawControl(QStyle::CE_FocusFrame, &opt, &painter, this);

		//界面自定义绘制
		OnPaint(&painter);
	}
}

void CCommonWidget::OnPaint(QPainter* painter)
{
}

void CCommonWidget::MakeRoundCorner()
{
	if(m_iRoundCornerSize> 0)
	{
		RECT rect,wndRect;
		POINT point = {0,0};
		::GetClientRect(winId(),&rect);
		::ClientToScreen(winId(),&point);
		::GetWindowRect(winId(),&wndRect);

		HRGN hRgn = ::CreateRoundRectRgn(point.x - wndRect.left,point.y - wndRect.top,rect.right + point.x - wndRect.left + 1,rect.bottom + point.y - wndRect.top + 1,m_iRoundCornerSize,m_iRoundCornerSize);
		if(hRgn)
		{
			if(::SetWindowRgn(winId(),hRgn,TRUE) == 0)
			{
				::DeleteObject(hRgn);
			}
		}
	}
}