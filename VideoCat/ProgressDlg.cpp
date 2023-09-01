// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ProgressDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const DWORD CProgressDlg::IDD = IDD_PROGRESS;

IMPLEMENT_DYNAMIC(CProgressDlg, CDialog)

CProgressDlg::CProgressDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CProgressDlg::IDD, pParent)
	, _progressStep(L"")
	, _progressTitle(L"")
	, _curStep(0)
	, _total(0)
{

}

CProgressDlg::~CProgressDlg()
{
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PROGRESS_TITLE, _progressTitle);
	DDX_Text(pDX, IDC_PROGRESS_STEPS, _progressStep);
	DDX_Control(pDX, IDC_PROGRESS, _progressBar);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	ON_MESSAGE( UPDATE_PROGRESS, &CProgressDlg::OnUpdateProgress )
END_MESSAGE_MAP()

void CProgressDlg::SetTitle( const CString & title )
{
	_progressTitle = title;
	UpdateData(FALSE);
}

void CProgressDlg::SetProgressCount( int total )
{
	_curStep = 0;
	_total = total;
	_progressBar.SetRange(0, (short)_total+1);

	UpdateProgress();
}

void CProgressDlg::SetCurrentStep( int step )
{
	_curStep = step;

	UpdateProgress();
}

void CProgressDlg::NextStep()
{
	++_curStep;

	UpdateProgress();
}

void CProgressDlg::UpdateProgress()
{
	_progressBar.SetPos( _curStep );
	_progressStep.Format( L"%i / %i", _curStep, _total );
	UpdateData( FALSE );
}


ProgressController::ProgressController()
	: _bar(nullptr)
{	;	}

ProgressController::~ProgressController()
{
	Destroy();
}

void ProgressController::Destroy()
{
	if( _bar )
	{
		_bar->DestroyWindow();
		delete _bar;
	}
	_bar = nullptr;
}

void ProgressController::Create(const CString & title )
{
	_bar = new CProgressDlg;
	_bar->Create( IDD_PROGRESS, AfxGetApp()->GetMainWnd() );
	_bar->ShowWindow(SW_SHOW);
	
	SetTitle(title);
}

void ProgressController::SetTitle( const CString & title )
{
	_bar->SetTitle(title);
}


void ProgressController::SetProgressCount( int total )
{
	_bar->SetProgressCount( total );
}

void ProgressController::SetCurrentStep( int step )
{
	_bar->SetCurrentStep( step );
}


void ProgressController::NextStep()
{
	//_bar->NextStep();

	HWND hwnd = _bar->m_hWnd;
	::PostMessage( hwnd, UPDATE_PROGRESS, 0, 0 );

	MSG msg;
	while( ::PeekMessage( &msg, hwnd, 0, 0, PM_NOREMOVE ) )
	{
		if( !AfxGetThread()->PumpMessage() )
			break;
	}
}


LRESULT CProgressDlg::OnUpdateProgress( WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
	NextStep();

	return 1;
}
