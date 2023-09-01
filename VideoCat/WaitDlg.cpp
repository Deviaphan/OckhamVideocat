// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "WaitDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CWaitDlg, CDialog)

CWaitDlg::CWaitDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_WAIT, pParent)
{

}

CWaitDlg::~CWaitDlg()
{
}

void CWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
}


BEGIN_MESSAGE_MAP(CWaitDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

BOOL CWaitDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWnd * logo = GetDlgItem( IDC_LOGO );
	if( logo )
	{
		CRect rect;
		logo->GetWindowRect( rect );
		ScreenToClient( rect );

		_catHead.Initialize( rect );
	}

	return TRUE;
}

void CWaitDlg::OnPaint()
{
	CPaintDC dc( this );

	_catHead.Draw( dc.GetSafeHdc() );
}


void CWaitDlg::OnMouseMove( UINT nFlags, CPoint point )
{
	_catHead.MouseMove( point.x, point.y );

	CDialog::OnMouseMove( nFlags, point );

	InvalidateRect( _catHead.GetRect(), FALSE );
}
