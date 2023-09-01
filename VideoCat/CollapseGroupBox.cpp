// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CollapseGroupBox.h"

IMPLEMENT_DYNAMIC(CollapseGroupBox,CButton);

CollapseGroupBox::CollapseGroupBox()
{
	m_bMemDCCreated = false;
    m_strTitle.Empty();
	m_clrBorder = ::GetSysColor(COLOR_3DSHADOW);
	m_clrClientBackground = ::GetSysColor(COLOR_BTNFACE);
	m_clrTitleText = ::GetSysColor(COLOR_CAPTIONTEXT);
	m_clrTitleBackground = m_clrTitleBackgroundFree = ::GetSysColor(COLOR_INACTIVECAPTION);
	m_clrTitleBackgroundHighLight = ::GetSysColor(COLOR_ACTIVECAPTION);
	m_clrTitleArrow = ::GetSysColor(COLOR_CAPTIONTEXT);
	m_dwAlignment = SS_RIGHT;
	m_frameMinimized = false;
	intoElements.clear();
}

CollapseGroupBox::~CollapseGroupBox(){}

BEGIN_MESSAGE_MAP(CollapseGroupBox, CButton)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CollapseGroupBox::setBGColor(COLORREF bgColor)
{
	m_clrClientBackground = bgColor;
}

void CollapseGroupBox::PreSubclassWindow()
{
	m_lf.lfHeight=11;   
	m_lf.lfWidth=0;   
	m_lf.lfEscapement=0;   
	m_lf.lfOrientation=0;   
	m_lf.lfWeight=FW_NORMAL;   
	m_lf.lfItalic=0;   
	m_lf.lfUnderline=0;   
	m_lf.lfStrikeOut=0;   
	m_lf.lfCharSet=ANSI_CHARSET;   
	m_lf.lfOutPrecision=OUT_DEFAULT_PRECIS;   
	m_lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;   
	m_lf.lfQuality=PROOF_QUALITY;   
	m_lf.lfPitchAndFamily=VARIABLE_PITCH | FF_ROMAN;   
    wcscpy_s(m_lf.lfFaceName, L"MS Sans Serif");

	SetFontBold(TRUE);
	intoElements.clear();
	ReconstructFont();
	ModifyStyle(0, BS_GROUPBOX|BS_OWNERDRAW|SWP_NOREPOSITION|WS_CLIPSIBLINGS);
	CButton::PreSubclassWindow();
}


BOOL CollapseGroupBox::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

BOOL CollapseGroupBox::PreTranslateMessage(MSG* pMsg) 
{
	return CButton::PreTranslateMessage(pMsg);
}

void CollapseGroupBox::OnSize(UINT nType, int cx, int cy) 
{
	CButton::OnSize(nType, cx, cy);

	if ( m_BackgroundDC.GetSafeHdc() )
		m_BackgroundDC.DeleteDC();	
	RefreshCtrl();
}

void CollapseGroupBox::OnPaint() 
{
	GetGroupBoxElements();
	CPaintDC dc(this);
	if (!m_bMemDCCreated)
	{
		RefreshCtrl();
		m_bMemDCCreated = true;
	}
    CRect rect;
    GetClientRect(&rect);
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &m_BackgroundDC, 0, 0, SRCCOPY) ;
}

// control drawing
void CollapseGroupBox::RefreshCtrl() 
{
	if (!GetSafeHwnd())	return;
	CClientDC curDC(this);  
   	CRect rectClient;
	GetClientRect(rectClient);

	if (!m_BackgroundDC.GetSafeHdc() )
	{
		CBitmap memBitmap;
		m_BackgroundDC.CreateCompatibleDC(&curDC) ;
		memBitmap.CreateCompatibleBitmap(&curDC, rectClient.Width(),rectClient.Height()) ;
		m_BackgroundDC.SelectObject(&memBitmap) ;
	}

	rectClient.DeflateRect(1,1);
	CSize sizeText;
	CRect rectText;
	CFont *pOldFont = m_BackgroundDC.SelectObject(&m_font);
	if ( m_strTitle.IsEmpty() )
	{ 
        GetWindowText(m_strTitle);
		if ( ! m_strTitle.IsEmpty() )	m_strTitle = _T(" ") + m_strTitle + _T(" ");
	}
	
	if (m_strTitle.IsEmpty())	
	{
		sizeText.cx = 0;
		sizeText.cy = 0;
	}
	else sizeText = m_BackgroundDC.GetTextExtent(m_strTitle);
	
	//-> header
	rectTitle.top = rectClient.top;
	rectTitle.left = rectClient.left;
	rectTitle.right = rectClient.right;
	rectTitle.bottom = rectClient.top + sizeText.cy + 4;
	CPen penFrame, penTmp;
	CBrush brushBKTitle(m_clrTitleBackground);
	CBrush brushBKContent(::GetSysColor(COLOR_BTNFACE));
	penFrame.CreatePen(PS_SOLID, 1, m_clrClientBackground);
	penTmp.CreatePen(PS_SOLID, 1, m_clrTitleBackground);	

	CPen* pOldPen = m_BackgroundDC.SelectObject(&penFrame);
	CBrush* pOldBrush = m_BackgroundDC.SelectObject(&brushBKContent);

	CRect tmpClient;
	GetClientRect(tmpClient);
	m_BackgroundDC.Rectangle(tmpClient);
	brushBKContent.DeleteObject();
	brushBKContent.CreateSolidBrush(m_clrClientBackground);
	m_BackgroundDC.SelectObject(&brushBKContent);
	m_BackgroundDC.Rectangle(tmpClient.left,tmpClient.top + 17,tmpClient.right,tmpClient.top);
	penFrame.DeleteObject();
	penFrame.CreatePen(PS_SOLID, 1, m_clrBorder);
	m_BackgroundDC.SelectObject(&penFrame);
	m_BackgroundDC.SelectObject(&brushBKTitle);

	m_BackgroundDC.RoundRect(rectTitle, CPoint(10, 10)); 
	if (!m_frameMinimized)
	{
		m_BackgroundDC.SelectObject(&penTmp);
		m_BackgroundDC.Rectangle(rectTitle.left,rectTitle.top+5,rectTitle.right,rectTitle.bottom); 
		m_BackgroundDC.SelectObject(&penFrame);
	}
	m_BackgroundDC.SelectObject(pOldBrush); 
	//<-
	
	//-> header button
	if (m_dwAlignment != SS_LEFT)
	{
		rectButton.top = rectTitle.top + 5;
		rectButton.left = rectTitle.left + 6;
		rectButton.bottom = rectButton.top + 9;
		rectButton.right = rectButton.left + 4;
	}
	else
	{
		rectButton.top = rectTitle.top + 5;
		rectButton.left = rectTitle.right - 13;
		rectButton.bottom = rectButton.top + 9;
		rectButton.right = rectButton.left + 4;
	}

	CPen penArrow;
	penArrow.CreatePen(PS_SOLID, 1, m_clrTitleArrow);
	m_BackgroundDC.SelectObject(&penArrow);
	if (!m_frameMinimized)
	{
		m_BackgroundDC.MoveTo(rectButton.left,rectButton.top);
		m_BackgroundDC.LineTo(rectButton.left+2,rectButton.top+2);
		m_BackgroundDC.LineTo(rectButton.left+5,rectButton.top-1);
		m_BackgroundDC.MoveTo(rectButton.left,rectButton.top+1);
		m_BackgroundDC.LineTo(rectButton.left+2,rectButton.top+3);
		m_BackgroundDC.LineTo(rectButton.left+5,rectButton.top);

		m_BackgroundDC.MoveTo(rectButton.left,rectButton.top+3);
		m_BackgroundDC.LineTo(rectButton.left+2,rectButton.top+5);
		m_BackgroundDC.LineTo(rectButton.left+5,rectButton.top+2);
		m_BackgroundDC.MoveTo(rectButton.left,rectButton.top+4);
		m_BackgroundDC.LineTo(rectButton.left+2,rectButton.top+6);
		m_BackgroundDC.LineTo(rectButton.left+5,rectButton.top+3);
	}
	else
	{
		if (m_dwAlignment != SS_LEFT)
		{
			m_BackgroundDC.MoveTo(rectButton.left,rectButton.top+1);
			m_BackgroundDC.LineTo(rectButton.left+2,rectButton.top+3);
			m_BackgroundDC.LineTo(rectButton.left-1,rectButton.top+6);
			m_BackgroundDC.MoveTo(rectButton.left+1,rectButton.top+1);
			m_BackgroundDC.LineTo(rectButton.left+3,rectButton.top+3);
			m_BackgroundDC.LineTo(rectButton.left,rectButton.top+6);

			m_BackgroundDC.MoveTo(rectButton.left+3,rectButton.top+1);
			m_BackgroundDC.LineTo(rectButton.left+5,rectButton.top+3);
			m_BackgroundDC.LineTo(rectButton.left+2,rectButton.top+6);
			m_BackgroundDC.MoveTo(rectButton.left+4,rectButton.top+1);
			m_BackgroundDC.LineTo(rectButton.left+6,rectButton.top+3);
			m_BackgroundDC.LineTo(rectButton.left+3,rectButton.top+6);
		}
		else
		{
			m_BackgroundDC.MoveTo(rectButton.right,rectButton.top+1);
			m_BackgroundDC.LineTo(rectButton.right-2,rectButton.top+3);
			m_BackgroundDC.LineTo(rectButton.right+1,rectButton.top+6);
			m_BackgroundDC.MoveTo(rectButton.right-1,rectButton.top+1);
			m_BackgroundDC.LineTo(rectButton.right-3,rectButton.top+3);
			m_BackgroundDC.LineTo(rectButton.right,rectButton.top+6);

			m_BackgroundDC.MoveTo(rectButton.right-3,rectButton.top+1);
			m_BackgroundDC.LineTo(rectButton.right-5,rectButton.top+3);
			m_BackgroundDC.LineTo(rectButton.right-2,rectButton.top+6);
			m_BackgroundDC.MoveTo(rectButton.right-4,rectButton.top+1);
			m_BackgroundDC.LineTo(rectButton.right-6,rectButton.top+3);
			m_BackgroundDC.LineTo(rectButton.right-3,rectButton.top+6);
		}
	}
	//<-
	
	//-> items background part
	if (!m_frameMinimized)
	{
		rectContent.left = rectClient.left;
		rectContent.top = rectClient.top + sizeText.cy + 4;
		rectContent.right = rectClient.right;
		rectContent.bottom = rectContent.top + rectClient.Height() - sizeText.cy - 4; 
		m_BackgroundDC.SelectObject(&penFrame);
		m_BackgroundDC.MoveTo(rectContent.left,rectTitle.top + 5);
		m_BackgroundDC.LineTo(rectContent.left,rectContent.bottom);
		m_BackgroundDC.LineTo(rectContent.right,rectContent.bottom);
		m_BackgroundDC.LineTo(rectContent.right,rectTitle.top + 5);
		
		m_BackgroundDC.SelectObject(pOldPen);
		m_BackgroundDC.SelectObject(pOldBrush); 
	}	
	//<-

	//-> caption string 
	switch(m_dwAlignment)
	{
		case SS_LEFT:
			rectText.top = rectTitle.top + 2;
			rectText.left = rectTitle.left + 2;
			rectText.bottom = rectText.top + sizeText.cy;
			rectText.right = rectText.left + sizeText.cx;
			break;
		case SS_CENTER:
			rectText.top = rectTitle.top + 2;
			rectText.left = rectTitle.left + (rectTitle.Width() - sizeText.cx) / 2;
			rectText.bottom = rectText.top + sizeText.cy;
			rectText.right = rectText.left + sizeText.cx;
			break;
		case SS_RIGHT:
			rectText.top = rectTitle.top + 2;
			rectText.right = rectClient.right - 2;
			rectText.bottom = rectText.top + sizeText.cy;
			rectText.left = rectText.right - sizeText.cx;
			break;
	}		    
	COLORREF clrOldText = m_BackgroundDC.SetTextColor(m_clrTitleText);
	UINT nMode = m_BackgroundDC.SetBkMode(TRANSPARENT);
	m_BackgroundDC.DrawText(m_strTitle, &rectText, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP ); 
	//<-

	m_BackgroundDC.SetBkMode(nMode);
    m_BackgroundDC.SetTextColor(clrOldText);
    m_BackgroundDC.SelectObject(pOldFont);
	m_BackgroundDC.SelectObject(pOldPen);
	m_BackgroundDC.SelectObject(pOldBrush); 
}

void CollapseGroupBox::ReconstructFont()
{
	m_font.DeleteObject();
	/*BOOL bCreated =*/ m_font.CreateFontIndirect(&m_lf);
}

void CollapseGroupBox::UpdateSurface()
{
	RefreshCtrl();
	CRect rc;
	GetWindowRect(&rc);
	RedrawWindow();
	GetParent()->ScreenToClient(&rc);
	GetParent()->InvalidateRect(&rc,TRUE);
	GetParent()->UpdateWindow();
}

CollapseGroupBox& CollapseGroupBox::SetFont(LOGFONT lf)
{
	CopyMemory(&m_lf, &lf, sizeof(m_lf));
	ReconstructFont();
	UpdateSurface();
	return *this;
}

CollapseGroupBox& CollapseGroupBox::SetFontBold(BOOL bBold)
{
	m_lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
	ReconstructFont();
	UpdateSurface();
	return *this;
}

CollapseGroupBox& CollapseGroupBox::SetFontName(const CString& strFont, BYTE byCharSet)
{
	m_lf.lfCharSet = byCharSet;
	wcscpy_s(m_lf.lfFaceName,strFont);
	ReconstructFont();
	UpdateSurface();
	return *this;
}

CollapseGroupBox& CollapseGroupBox::SetFontUnderline(BOOL bSet)
{
	m_lf.lfUnderline = bSet;
	ReconstructFont();
	UpdateSurface();
	return *this;
}

CollapseGroupBox& CollapseGroupBox::SetFontItalic(BOOL bSet)
{
	m_lf.lfItalic = bSet;
	ReconstructFont();
	UpdateSurface();
	return *this;	
}

CollapseGroupBox& CollapseGroupBox::SetFontSize(int nSize)
{
	CFont cf;
	LOGFONT lf;
	cf.CreatePointFont(nSize * 10, m_lf.lfFaceName);
	cf.GetLogFont(&lf);
	m_lf.lfHeight = lf.lfHeight;
	m_lf.lfWidth  = lf.lfWidth;
	ReconstructFont();
	UpdateSurface();
	return *this;
}

CollapseGroupBox& CollapseGroupBox::SetBorderColor(COLORREF clrBorder)
{
	m_clrBorder = clrBorder;
	UpdateSurface();
	return *this;
}

CollapseGroupBox& CollapseGroupBox::SetCatptionTextColor(COLORREF clrText ) 
{
	m_clrTitleText = clrText;
	UpdateSurface();
	return *this;
}

CollapseGroupBox& CollapseGroupBox::SetBackgroundColor(COLORREF clrBKTilte)
{
	m_clrTitleBackground = clrBKTilte;
	UpdateSurface();
	return *this;
}

CollapseGroupBox& CollapseGroupBox::SetText(LPCTSTR lpszText)
{
	if(IsWindow(this->GetSafeHwnd())) 
	{
		m_strTitle = lpszText;
		UpdateSurface();
	}
	return *this;
}

CollapseGroupBox& CollapseGroupBox::SetAlignment(DWORD dwType)
{
	switch(dwType)
	{
		case SS_LEFT:	
			m_dwAlignment =  SS_LEFT;
			break;
		case SS_CENTER:
			m_dwAlignment = SS_CENTER;
			break;
		case SS_RIGHT:
			m_dwAlignment = SS_RIGHT;
			break;
	}
	UpdateSurface();
	return *this;
}

CollapseGroupBox& CollapseGroupBox::SetFrameExpand(bool boxToLine)
{
   	CRect rectClient;
	this->GetWindowRect(rectClient);
	GetParent()->ScreenToClient(rectClient);

	CRect tmpRect = rectTitle;
	m_frameMinimized = boxToLine;
	ShowElements(!m_frameMinimized);
	if (!m_frameMinimized)
	{
		tmpRect.bottom += rectContent.Height();	
	}
	tmpRect.DeflateRect(-1,-1);
	tmpRect.MoveToXY(rectClient.left,rectClient.top);
	MoveWindow(tmpRect);
	GetParent()->SendMessage(WM_COMMAND, UM_UPDATEFORM, (LPARAM) m_hWnd);
	UpdateSurface();
	return *this;
}

void CollapseGroupBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	CButton::OnLButtonDown(nFlags, point);
	CRect tmpRect = rectTitle;
	tmpRect.DeflateRect(-1,-1);
	if ((nFlags&MK_LBUTTON) && tmpRect.PtInRect(point)) 
	{
		SetFrameExpand(!m_frameMinimized);
		UpdateSurface();
	}
}

void CollapseGroupBox::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect tmpRect = rectTitle;
	tmpRect.DeflateRect(-1,-1);
	if (GetCapture() == this)
	{
		if(!tmpRect.PtInRect(point))
		{
			m_clrTitleBackground = m_clrTitleBackgroundFree;
			UpdateSurface();
			GetParent()->SendMessage(WM_COMMAND, UM_MOUSELEAVE,(LPARAM) m_hWnd);
			ReleaseCapture();
		}
	}
	else
	{
		if(tmpRect.PtInRect(point))
		{
			m_clrTitleBackground = m_clrTitleBackgroundHighLight;
			UpdateSurface();
			GetParent()->SendMessage(WM_COMMAND, UM_MOUSEENTRY,(LPARAM) m_hWnd);
			SetCapture();
		}
	}
	CButton::OnMouseMove(nFlags,point);
}

BOOL CollapseGroupBox::IsInGroupBox(CWnd *pCtrl) const
{
	BOOL bIsInGroup = FALSE;
	if (pCtrl == NULL) return FALSE;
	CRect rcGroup, rc;
	GetWindowRect(&rcGroup);
	pCtrl->GetWindowRect(&rc);
	if (rcGroup.PtInRect(rc.BottomRight()) && rcGroup.PtInRect(rc.TopLeft())) bIsInGroup = TRUE;
	if (! bIsInGroup && (rcGroup.PtInRect(rc.BottomRight()) || rcGroup.PtInRect(rc.TopLeft()))) bIsInGroup = TRUE;
	return bIsInGroup;
}

void CollapseGroupBox::GetGroupBoxElements()
{
	CRect rcGroup;
	if (intoElements.empty())
	{
		GetWindowRect(&rcGroup);
		CWnd* pCtrl = NULL;
	    pCtrl = GetParent()->GetWindow(GW_CHILD);
		while (pCtrl != NULL)
		{
			if (pCtrl->GetSafeHwnd() != GetSafeHwnd())
			{
				BOOL bIsInGroup = IsInGroupBox(pCtrl);
				if (bIsInGroup)
				{
					intoElements.push_back(pCtrl);
				}
			}
			pCtrl = pCtrl->GetNextWindow();
		}
	}
	return;
}

void CollapseGroupBox::ShowElements(bool bShow)
{
	for (int i = 0; i < (int)intoElements.size(); i++)
	{
		intoElements.at(i)->EnableWindow(bShow);
		intoElements.at(i)->ShowWindow(bShow);
	}
}
