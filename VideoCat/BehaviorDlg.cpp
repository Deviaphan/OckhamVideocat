// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "BehaviorDlg.h"
#include "resource.h"


IMPLEMENT_DYNAMIC( CBehaviorDlg, CPropertyPage )

CBehaviorDlg::CBehaviorDlg()
	: CPropertyPage( IDD_BEHAVIOR )
	, closeBeforePlay( FALSE )
	, marktype( 0 )
	, alwaysShowRating( FALSE )
	, hideSecondTitle( FALSE )
	, tipOfDay( TRUE )
	, numRandomFilms( 0 )
	, autoplayRandomFilm( FALSE )
{

}

CBehaviorDlg::~CBehaviorDlg()
{}

void CBehaviorDlg::DoDataExchange( CDataExchange* pDX )
{
	CPropertyPage::DoDataExchange( pDX );
	DDX_Check( pDX, IDC_HIDE_SECOND_TITLE, hideSecondTitle );
	DDX_Check( pDX, IDC_CLOSE_BEFORE_PLAY, closeBeforePlay );
	DDX_Radio( pDX, IDC_MARK_NOT_VIEWED, marktype );
	DDX_Check( pDX, IDC_CHK_SHOW_RATINGS, alwaysShowRating );
	DDX_Check( pDX, IDC_SHOW_TIP_OF_DAY, tipOfDay );
	DDX_Text( pDX, IDC_NUM_RANDOM_FILMS, numRandomFilms );
	DDV_MinMaxInt( pDX, numRandomFilms, 1, 100 );
	DDX_Check( pDX, IDC_AUTOPLAY_RANDOM, autoplayRandomFilm );
}


BEGIN_MESSAGE_MAP( CBehaviorDlg, CPropertyPage )
END_MESSAGE_MAP()




BOOL CBehaviorDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;
}
