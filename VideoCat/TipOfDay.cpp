// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "TipOfDay.h"
#include <vector>
#include "GlobalSettings.h"
#include "WinVerTest.h"
#include "SynchronizeFiles.h"
#include "resource.h"
#include "afxwin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CTipOfDay : public CDialog
{
	DECLARE_DYNAMIC( CTipOfDay )

public:
	CTipOfDay( CWnd* pParent = nullptr );   // standard constructor
	virtual ~CTipOfDay();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_TIP_OF_DAY
	};
#endif

protected:
	BOOL OnInitDialog() override;
	BOOL DestroyWindow() override;
	void DoDataExchange( CDataExchange* pDX ) override;    // DDX/DDV support
	void OnOK() override;
	void OnCancel() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedCheckHints();
	afx_msg void OnPrevHint();
	afx_msg void OnNextHint();
	afx_msg LRESULT OnNcHitTest( CPoint point );
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	afx_msg void OnGetMinMaxInfo( MINMAXINFO* lpMMI );

private:
	CButton _tips;
	CImage _bg;
	int _curHint;

public:
	std::vector<CString> allHints;
};


IMPLEMENT_DYNAMIC(CTipOfDay, CDialog)

CTipOfDay::CTipOfDay(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_TIP_OF_DAY, pParent)
	, _curHint(0)
{

}

CTipOfDay::~CTipOfDay()
{
}

void CTipOfDay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECK_HINTS, _tips );
}


BEGIN_MESSAGE_MAP(CTipOfDay, CDialog)
	ON_BN_CLICKED( IDC_CHECK_HINTS, &CTipOfDay::OnBnClickedCheckHints )
	ON_BN_CLICKED( IDC_BTN_PREV_HINT, &CTipOfDay::OnPrevHint )
	ON_BN_CLICKED( IDC_BTN_NEXT_HINT, &CTipOfDay::OnNextHint )
	ON_WM_NCHITTEST()
	ON_WM_ERASEBKGND()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


BOOL CTipOfDay::OnInitDialog()
{
	_curHint = rand() % (int)allHints.size();

	_bg.Load( allHints.at(_curHint) );

	CDialog::OnInitDialog();

	if( GetGlobal().ShowTipOfDay() )
	{
		_tips.SetCheck( BST_CHECKED );
	}

	return TRUE;
}

void CTipOfDay::OnOK()
{
	OnCancel();
}


void CTipOfDay::OnCancel()
{
	CDialog::OnCancel();

	DestroyWindow();
}


BOOL CTipOfDay::DestroyWindow()
{
	BOOL result = CDialog::DestroyWindow();

	delete this;

	return result;
}

void CTipOfDay::OnBnClickedCheckHints()
{
	BOOL hint = GetGlobal().ShowTipOfDay();
	GetGlobal().SetShowTipOfDay( !hint );
}


void CTipOfDay::OnNextHint()
{
	++_curHint;
	if( _curHint >= (int)allHints.size() )
		_curHint = 0;

	_bg.Destroy();
	_bg.Load( allHints.at( _curHint ) );

	Invalidate();
}


void CTipOfDay::OnPrevHint()
{
	--_curHint;
	if( _curHint < 0 )
		_curHint = (unsigned)allHints.size() - 1;

	_bg.Destroy();
	_bg.Load( allHints.at( _curHint ) );

	Invalidate();	
}

LRESULT CTipOfDay::OnNcHitTest( CPoint point )
{
	UINT ret = CDialog::OnNcHitTest( point );

	// если обработчик по умолчанию говорит нам что мышка
	// над клиентской областью окна,  заменяем возвращаемое
	// значение на HTCAPTION - мышка над заголовком окна, 
	// а за заголовок перемещать окно можно!
	if( ret == HTCLIENT )
		return HTCAPTION;

	return CDialog::OnNcHitTest( point );
}


BOOL CTipOfDay::OnEraseBkgnd( CDC* pDC )
{
	BOOL result = CDialog::OnEraseBkgnd( pDC );

	_bg.BitBlt( pDC->GetSafeHdc(), 0, 0, SRCCOPY );

	return result;
}


void CTipOfDay::OnGetMinMaxInfo( MINMAXINFO* lpMMI )
{
	lpMMI->ptMinTrackSize.x = 650;
	lpMMI->ptMinTrackSize.y = 400;

	lpMMI->ptMaxTrackSize.x = 650;
	lpMMI->ptMaxTrackSize.y = 400;

	CDialog::OnGetMinMaxInfo( lpMMI );
}

void ShowTipOfDay()
{
	if( !GetGlobal().ShowTipOfDay() )
		return;

	CString hintDir = GetGlobal().GetHintDir();
	if( !::PathFileExists( hintDir ) )
		return;

	std::vector<CString> allHints;


	static bool checkWindowsVersion = true;
	static bool isWin7plus = true;
	if( checkWindowsVersion )
	{
		checkWindowsVersion = false;
		isWin7plus = IsWin7Plus();
	}

	const FINDEX_INFO_LEVELS infoLevel = isWin7plus ? FindExInfoBasic : FindExInfoStandard;
	const FINDEX_SEARCH_OPS searchOp = FindExSearchNameMatch;
	const DWORD useLargeFetch = isWin7plus ? FIND_FIRST_EX_LARGE_FETCH : 0;

	WIN32_FIND_DATA fd;
	
	HANDLE hFind = ::FindFirstFileEx( hintDir + L"*", infoLevel, &fd, searchOp, nullptr, useLargeFetch );

	if( hFind == INVALID_HANDLE_VALUE )
		return;


	FindGuard guard( hFind );
	do
	{
		// пропускаем файлы и папки, начинаеющиеся с точки (помимо служебных, было решено пропускать папки, начинающиеся с точки)
		if( fd.cFileName[0] == L'.' )
			continue;

		if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			continue;

		allHints.push_back( hintDir + fd.cFileName );
	}
	while( ::FindNextFile( hFind, &fd ) );


	if( allHints.empty() )
		return;

	// Диалог удалит сам себя при закрытии
	CTipOfDay * tod = new CTipOfDay;
	tod->allHints.swap( allHints );
	tod->Create( IDD_TIP_OF_DAY );
	tod->ShowWindow( SW_SHOWNORMAL );
} //-V773
