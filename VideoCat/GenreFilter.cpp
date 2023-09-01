// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "VideoCat.h"
#include "GenreFilter.h"
#include "resource.h"

const DWORD GENRE_OFFSET = 1000;

IMPLEMENT_DYNAMIC(CGenreFilter, CDialog)

CGenreFilter::CGenreFilter(CWnd* pParent /*=nullptr*/)
	: CDialog( IDD_GENRE, pParent)
	, film3D( FALSE )
	, strictGenre( FALSE )
	, outputGenres( 0 )
	, extraFiltering( FALSE )
	, filterType( FilterType::ByGenre )
	, tagManager( nullptr )
{

}

CGenreFilter::~CGenreFilter()
{
}

void CGenreFilter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );

	DDX_Check( pDX, IDC_ONLY_3D_FILMS, film3D );
	DDX_Radio( pDX, IDC_RADIO_GENRE, filterType );
	DDX_Control( pDX, IDC_GENRES, _checkList );
	DDX_Check( pDX, IDC_STRICT_GENRE, strictGenre );
}


BEGIN_MESSAGE_MAP(CGenreFilter, CDialog)
	ON_BN_CLICKED( ID_BY_GENRES, &CGenreFilter::OnBnClickedByGenres )
	ON_BN_CLICKED( ID_BY_GENRE_NAME, &CGenreFilter::OnBnClickedByGenreName )
	ON_BN_CLICKED( IDC_RADIO_GENRE, &CGenreFilter::OnBnClickedRadioGenre )
	ON_BN_CLICKED( IDC_RADIO_TAG, &CGenreFilter::OnBnClickedRadioTag )
END_MESSAGE_MAP()


BOOL CGenreFilter::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowPos( nullptr, position.x, position.y, 0, 0, SWP_NOSIZE );

	OnBnClickedRadioGenre();

	return TRUE;
}

void CGenreFilter::ReadGenre()
{
	UpdateData();

	const int numItems = _checkList.GetCount();

	if( filterType == FilterType::ByGenre )
	{
		outputGenres = 0;

		for( int i = 0; i < numItems; ++i )
		{
			if( _checkList.GetCheck( i ) )
			{
				Genre genre = (Genre)(_checkList.GetItemData( i ) - GENRE_OFFSET);
				Set( outputGenres, genre );
			}
		}
	}
	else // ByTag
	{
		selectedTags.clear();

		for( int i = 0; i < numItems; ++i )
		{
			if( _checkList.GetCheck( i ) )
			{
				TagId id = (TagId)_checkList.GetItemData( i );
				selectedTags.insert( id );
			}
		}
	}
}

void CGenreFilter::OnBnClickedByGenres()
{
	ReadGenre();
	extraFiltering = FALSE;

	OnOK();
}


void CGenreFilter::OnBnClickedByGenreName()
{
	ReadGenre();
	extraFiltering = TRUE;

	OnOK();
}


void CGenreFilter::OnBnClickedRadioGenre()
{
	_checkList.ResetContent();
	_checkList.SetCheckStyle( BS_AUTOCHECKBOX );

	for( int i =(int)Genre::FirstGenre; i < (int)Genre::NumOfGenres; ++i )
	{
		Genre genre = (Genre)i;
		auto id = inputGenres.find( genre );
		if( id != inputGenres.end() )
		{
			CString name;
			name.Format( L"%s   [ %u ]", Descript( genre ).GetString(), id->second );
			int index = _checkList.AddString( name );
			_checkList.SetItemData( index, (DWORD_PTR)(i + GENRE_OFFSET) );
		}
	}
}


void CGenreFilter::OnBnClickedRadioTag()
{
	_checkList.ResetContent();
	_checkList.SetCheckStyle( BS_AUTOCHECKBOX );

	for( auto & id : inputTags )
	{
		const TagManager::TagName & tagName = tagManager->GetTag( id.first );
		if( !tagName.empty() )
		{
			CString name;
			name.Format( L"%s   [ %u ]", tagName.c_str(), id.second );
			const int index = _checkList.AddString( name );
			_checkList.SetItemData( index, (DWORD_PTR)(id.first) );
		}
	}
}
