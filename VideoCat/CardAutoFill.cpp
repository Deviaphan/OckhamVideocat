// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CardAutoFill.h"
#include "Kinopoisk/Kinopoisk.h"
#include "Kinopoisk/LoadKinopoiskPage.h"
#include "SynchronizeFiles.h"
#include "CardTabCtrl.h"
#include "GlobalSettings.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(CCardAutoFill, CMFCPropertyPage )


bool SearchById( KinopoiskId id, KinopoiskInfo & ki )
{
	Kinopoisk & kinopoisk = GetKinopoisk();
	ki = kinopoisk.SearchInfo( id );

	return ki.filmID != NO_FILM_ID;
}

bool SearchByUrl( KinopoiskId id, KinopoiskInfo & ki )
{
	LoadKinopoiskInfoDirect( id, ki );
	return ki.filmID != NO_FILM_ID;
}


CCardAutoFill::CCardAutoFill()
	: CMFCPropertyPage(IDD_TAB_AUTOFILL)
	, _filmId( 0 )
	, _title( _T( "" ) )
	, _fillType( 0 )
{

}

CCardAutoFill::~CCardAutoFill()
{
}

void CCardAutoFill::DoDataExchange(CDataExchange* pDX)
{
	CMFCPropertyPage::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_FILM_ID, _filmId );
	DDX_Text( pDX, IDC_FILM_TITLE, _title );
	DDX_Radio( pDX, IDC_RADIO_FILL_SKIP, _fillType );
	DDX_Control( pDX, IDC_COMBO_PLUGIN, _pluginList );
	DDX_Control( pDX, IDC_FILM_LIST, _filmList );
}


BEGIN_MESSAGE_MAP(CCardAutoFill, CMFCPropertyPage )
	ON_BN_CLICKED( IDC_APPLY_FILM, &CCardAutoFill::OnBnClickedApplyFilm )
	ON_BN_CLICKED( IDC_BTN_CLEAR, &CCardAutoFill::OnBnClickedBtnClear )
	ON_BN_CLICKED( IDC_FILL_BY_ID_DB, &CCardAutoFill::OnBnClickedFillByIdDb )
	ON_BN_CLICKED( IDC_FILL_BY_ID_SITE, &CCardAutoFill::OnBnClickedFillByIdSite )
	ON_BN_CLICKED( IDC_FILL_BY_NAME, &CCardAutoFill::OnBnClickedFillByName )
	ON_BN_CLICKED( IDC_RADIO_FILL_AUTO, &CCardAutoFill::OnBnClickedRadioFillAuto )
	ON_BN_CLICKED( IDC_RADIO_FILL_SKIP, &CCardAutoFill::OnBnClickedRadioFillSkip )
	ON_BN_CLICKED( IDC_RADIO_FILL_USER, &CCardAutoFill::OnBnClickedRadioFillUser )
END_MESSAGE_MAP()


BOOL CCardAutoFill::OnInitDialog()
{
	__super::OnInitDialog();

	_pluginList.AddString( L"Кинопоиск" );
	_pluginList.SetCurSel( 0 );

	return TRUE;
}

void CCardAutoFill::ReinitTab()
{
	__super::ReinitTab();

	if( entry->pluginID == PluginNone )
		_fillType = 0;
	else if( entry->pluginID == PluginManual )
		_fillType = 1;
	else
		_fillType = 2;

	EnableControls( _fillType == 2 );

	_filmId = entry->filmId;

	_title.Empty();

	if( entry->year > 0 )
	{
		_title.Format( L"%i - ", (int)entry->year );
	}

	if( !entry->title.empty() )
	{
		_title += entry->title.c_str();

		if( !entry->titleAlt.empty() )
		{
			_title += L" (";
			_title += entry->titleAlt.c_str();
			_title += L")";
		}
	}
	else
	{
		_title = GetNameFromFile( entry->filePath.c_str() );
	}

	UpdateData( FALSE );
}

void CCardAutoFill::EnableControls( BOOL on )
{
	const DWORD id[] = { /*IDC_COMBO_PLUGIN,*/ IDC_FILM_ID, IDC_FILL_BY_ID_DB, IDC_FILL_BY_ID_SITE, IDC_FILM_TITLE, IDC_FILL_BY_NAME, IDC_APPLY_FILM };

	for(unsigned long i : id)
	{
		CWnd * wnd = GetDlgItem( i );
		if( wnd )
			wnd->EnableWindow( on );
	}
}


bool CCardAutoFill::SaveData()
{
	UpdateData();

	if( entry->pluginID == PluginNone || entry->pluginID == (PluginID)PluginManual )
		entry->filmId = NO_FILM_ID;
	else
		entry->filmId = _filmId;

	return true;
}

void CCardAutoFill::OnBnClickedBtnClear()
{
	entry->title = GetNameFromFile( entry->filePath.c_str() );
	entry->titleAlt.clear();
	entry->year = 0;
	Clear( entry->flags, EntryTypes::IsFilm3D );
	entry->filmId = NO_FILM_ID;
	entry->pluginID = PluginNone;
	entry->tagline.clear();
	entry->description.clear();
	entry->seriesData = 0;
	entry->genres = 0;
	entry->rating = 0;
	entry->tags.clear();

	ReinitTab();

	CCardTabCtrl * tabCtrl = dynamic_cast<CCardTabCtrl*>(GetParent());
	if( tabCtrl )
		tabCtrl->ReloadCard();
}

void Copy( const KinopoiskInfo & ki, Entry & entry )
{
	entry.filmId = ki.filmID;
	entry.title = ki.nameRU;
	entry.titleAlt = ki.nameEN;
	entry.year = ki.year;
	entry.tagline = ki.tagline;
	entry.description = ki.description;
	if( ki.rating != 0 )
		entry.rating = ki.rating;

	entry.genres = ki.genres;
}

void CCardAutoFill::OnBnClickedFillByIdDb()
{
	UpdateData();
	
	KinopoiskInfo ki;

	if( !SearchById( _filmId, ki ) )
		return;

	Copy( ki, *entry );

	CCardTabCtrl * tabCtrl = dynamic_cast<CCardTabCtrl*>(GetParent());
	if( tabCtrl )
	{
		tabCtrl->ReloadCard();
		tabCtrl->SetActiveTab( 1 );
	}
}


void CCardAutoFill::OnBnClickedFillByIdSite()
{
	UpdateData();

	KinopoiskInfo ki;

	if( !SearchByUrl( _filmId, ki ) )
		return;

	Copy( ki, *entry );

	CCardTabCtrl * tabCtrl = dynamic_cast<CCardTabCtrl*>(GetParent());
	if( tabCtrl )
	{
		tabCtrl->ReloadCard();
		tabCtrl->SetActiveTab( 1 );
	}
}


void CCardAutoFill::OnBnClickedFillByName()
{
	CWaitCursor waiting;
	UpdateData();

	_filmList.ResetContent();

	Kinopoisk & kinopoisk = GetKinopoisk();

	kinopoisk.SearchInfo( _title, _foundedFilms );

	std::sort( _foundedFilms.begin(), _foundedFilms.end(),
		[]( const auto & lhs, const auto & rhs )->bool
		{
		if( lhs.year > rhs.year )
			return true;
		if( lhs.year < rhs.year )
			return false;

		if( lhs.nameRU < rhs.nameRU )
			return true;
		if( lhs.nameRU > rhs.nameRU )
			return false;
		if( lhs.nameEN < rhs.nameEN )
			return true;

			return false;
		}
	);

	for( const KinopoiskInfo & ki : _foundedFilms )
	{
		CString s;
		s.Format( L"%i - %s (%s)", (int)ki.year, ki.nameRU.c_str(), ki.nameEN.c_str() );

		_filmList.AddString( s );
	}
}


void CCardAutoFill::OnBnClickedRadioFillSkip()
{
	entry->pluginID = PluginNone;
	EnableControls( FALSE );
}


void CCardAutoFill::OnBnClickedRadioFillUser()
{
	entry->pluginID = (PluginID)PluginManual;
	EnableControls( FALSE );
}


void CCardAutoFill::OnBnClickedRadioFillAuto()
{
	entry->pluginID = (PluginID)PluginKinopoisk;

	EnableControls( TRUE );
}


void CCardAutoFill::OnBnClickedApplyFilm()
{
	UpdateData();

	if( _foundedFilms.empty() )
		return;

	const int curSel = _filmList.GetCurSel();
	if( curSel < 0 )
		return;

	Copy( _foundedFilms.at( curSel ), *entry );

	entry->pluginID = (PluginID)PluginKinopoisk;

	CCardTabCtrl * tabCtrl = dynamic_cast<CCardTabCtrl*>(GetParent());
	if( tabCtrl )
	{
		tabCtrl->ReloadCard();
		tabCtrl->SetActiveTab( 1 );
	}
}
