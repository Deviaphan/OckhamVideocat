// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "VideoCat.h"

#include "MainFrm.h"
#include "VideoCatView.h"
#include "VideoTreeView.h"
#include "ResString.h"
#include "GlobalSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetTitle( ResString(IDS_MAINFRAME_TITLE) );

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	const int treeWidth = theApp.GetProfileInt( _T( "Frame" ), _T( "TreeWidth" ), 280 );

	m_wndSplitter.CreateStatic(this, 1, 2);
	m_wndSplitter.CreateView( 0, 0, RUNTIME_CLASS( CVideoTreeView ), CSize( treeWidth, 250 ), pContext );
	m_wndSplitter.CreateView( 0, 1, RUNTIME_CLASS( CVideoCatView ), CSize(250, 250), pContext );

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( cs.hMenu != nullptr )
	{
		::DestroyMenu( cs.hMenu );      // удаляем меню, если загружено
		cs.hMenu = nullptr;              // для этого окна меню нет
	}

	int top = theApp.GetProfileInt( _T( "Frame" ), _T( "Top" ), -1 );
	int left = theApp.GetProfileInt( _T( "Frame" ), _T( "Left" ), -1 );
	int bottom = theApp.GetProfileInt( _T( "Frame" ), _T( "Bottom" ), -1 );
	int right = theApp.GetProfileInt( _T( "Frame" ), _T( "Right" ), -1 );

	if( top != -1 && bottom != -1 && right != -1 && left != -1 )
	{
		cs.cx = right - left;
		cs.cy = bottom - top;

		RECT workArea;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &workArea, 0 );
		left += workArea.left;
		top += workArea.top;

		int max_x = GetSystemMetrics( SM_CXSCREEN ) - GetSystemMetrics( SM_CXICON );
		int max_y = GetSystemMetrics( SM_CYSCREEN ) - GetSystemMetrics( SM_CYICON );
		cs.x = min( left, max_x );
		cs.y = min( top, max_y );
	}

	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

void CMainFrame::OnDestroy()
{
	try
	{
		WINDOWPLACEMENT wp;
		memset( &wp, 0, sizeof( WINDOWPLACEMENT ) );
		wp.length = sizeof( WINDOWPLACEMENT );
		GetWindowPlacement( &wp );

		const int top = theApp.GetProfileInt( _T( "Frame" ), _T( "Top" ), -1 );
		const int left = theApp.GetProfileInt( _T( "Frame" ), _T( "Left" ), -1 );
		const int bottom = theApp.GetProfileInt( _T( "Frame" ), _T( "Bottom" ), -1 );
		const int right = theApp.GetProfileInt( _T( "Frame" ), _T( "Right" ), -1 );

		if( top != wp.rcNormalPosition.top )
			theApp.WriteProfileInt( _T( "Frame" ), _T( "Top" ), wp.rcNormalPosition.top );
		if( left != wp.rcNormalPosition.left )
			theApp.WriteProfileInt( _T( "Frame" ), _T( "Left" ), wp.rcNormalPosition.left );
		if( bottom != wp.rcNormalPosition.bottom )
			theApp.WriteProfileInt( _T( "Frame" ), _T( "Bottom" ), wp.rcNormalPosition.bottom );
		if( right != wp.rcNormalPosition.right )
			theApp.WriteProfileInt( _T( "Frame" ), _T( "Right" ), wp.rcNormalPosition.right );

		CWnd * treePane = m_wndSplitter.GetPane( 0, 0 );
		if( treePane )
		{
			CRect treeRect;
			treePane->GetWindowRect( treeRect );
			const int treeWidth = theApp.GetProfileInt( _T( "Frame" ), _T( "TreeWidth" ), -1 );
			if( treeWidth != treeRect.Width() )
				theApp.WriteProfileInt( _T( "Frame" ), _T( "TreeWidth" ), treeRect.Width() );
		}
	}
	catch( ... )
	{
	}

	CFrameWnd::OnDestroy();
}

// диагностика CMainFrame

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG



void CMainFrame::OnSize( UINT nType, int cx, int cy )
{
	CFrameWnd::OnSize( nType, cx, cy );

	UpdateViewSizes();
}

void CMainFrame::UpdateViewSizes()
{
	if( !m_wndSplitter.IsWindowVisible() )
		return;

	CWnd * treePane = m_wndSplitter.GetPane( 0, 0 );
	if( !treePane )
		return;
	CRect treeRect;
	treePane->GetWindowRect( treeRect );

	CWnd * viewPane = m_wndSplitter.GetPane( 0, 1 );
	if( !viewPane )
		return;
	CRect viewRect;
	viewPane->GetWindowRect( viewRect );

	GetGlobal().bgFullSize.SetRect( 0, 0, treeRect.Width() + viewRect.Width(), treeRect.Height() ); // высота у дерева и списка одинаковые
	GetGlobal().bgTree.SetRect( 0, 0, treeRect.Width(), treeRect.Height() );
	GetGlobal().bgView.SetRect( 0, 0, viewRect.Width(), viewRect.Height() );
}
