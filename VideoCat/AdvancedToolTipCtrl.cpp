// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "AdvancedToolTipCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


AdvancedToolTipCtrl::AdvancedToolTipCtrl()
	: _currentControl(nullptr)
	, _currentItem(-1)
{
}

AdvancedToolTipCtrl::~AdvancedToolTipCtrl()
{
}


BEGIN_MESSAGE_MAP(AdvancedToolTipCtrl, CMFCToolTipCtrl)
	ON_NOTIFY_REFLECT(TTN_SHOW, &AdvancedToolTipCtrl::OnShow)
END_MESSAGE_MAP()


void AdvancedToolTipCtrl::OnShow(NMHDR* pNMHDR, LRESULT* pResult)
{
	_currentControl = CWnd::FromHandle( (HWND)pNMHDR->idFrom );
	_currentItem = -1;

	for( int i = 0; i < (int)_tooltips.size(); ++i )
	{
		const TooltipItem & tt = _tooltips[i];
		if( tt.wnd == _currentControl )
		{
			_currentItem = i;
			break;
		}
	}

	if( _currentItem != -1 )
	{
		const TooltipItem & tt = _tooltips[_currentItem];

		SetDescription( tt.description );
	}
	else
	{
		SetDescription( L"" );
	}

	CMFCToolTipCtrl::OnShow (pNMHDR, pResult);
}

CSize AdvancedToolTipCtrl::GetIconSize()
{
	if( _currentItem == -1 )
		return CSize(0, 0);

	const TooltipItem & tt = _tooltips[_currentItem];
	if( tt.hicon == nullptr )
		return CSize( 0, 0 );

	return CSize( 32, 32 );
}

BOOL AdvancedToolTipCtrl::OnDrawIcon (CDC* pDC, const CRect & rectImage)
{
	if( _currentItem == -1 )
		return TRUE;

	const TooltipItem & tt = _tooltips[_currentItem];
	if( tt.hicon == nullptr )
		return TRUE;

	pDC->DrawIcon( rectImage.left, rectImage.top, tt.hicon );

	return TRUE;
}

void AdvancedToolTipCtrl::AddTooltip( CWnd * wnd, const CString & title, const CString & description, HICON icon )
{
	TooltipItem tt;
	tt.wnd = wnd;
	tt.hicon = icon;
	tt.title = title;
	tt.description = description;

	_tooltips.push_back( tt );

	AddTool( wnd, title );
}

void AdvancedToolTipCtrl::EditTooltip( CWnd * wnd, const CString & title, const CString & description, HICON icon )
{
	int itemIndex = -1;

	for( int i = 0; i < (int)_tooltips.size(); ++i )
	{
		const TooltipItem & tt = _tooltips[i];
		if( tt.wnd == wnd )
		{
			itemIndex = i;
			break;
		}
	}

	if( itemIndex == -1 )
		return;

	TooltipItem & tt = _tooltips[itemIndex];
	tt.title = title;
	tt.description = description;
	tt.hicon = icon;

	UpdateTipText( title, wnd );
}
