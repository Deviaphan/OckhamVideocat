// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "EraseDlg.h"
#include "poster.h"
#include "SynchronizeFiles.h"
#include "resource.h"

IMPLEMENT_DYNAMIC( CEraseDlg, CDialog )

CEraseDlg::CEraseDlg( CWnd* pParent /*=nullptr*/ )
	: CDialog( IDD_ERASE, pParent )
{

}

CEraseDlg::~CEraseDlg()
{
}

void CEraseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Check( pDX, IDC_ERASE_FOLDER, data.notEraseFolder );
	DDX_Check( pDX, IDC_DELETE_KINOPOISK, data.eraseKinopoisk );
	DDX_Check( pDX, IDC_DELETE_COUNTERS, data.eraseCounters );
	DDX_Check( pDX, IDC_DELETE_TAGS, data.eraseTags );
	DDX_Check( pDX, IDC_DELETE_COMMENTS, data.eraseComments );
	DDX_Check( pDX, IDC_DELETE_TECHINFO, data.eraseTechInfo );
	DDX_Check( pDX, IDC_DELETE_POSTERS, data.erasePosters );
	DDX_Check( pDX, IDC_DELETE_THUMBS, data.eraseThumbs );
}


BEGIN_MESSAGE_MAP(CEraseDlg, CDialog)
	ON_BN_CLICKED( IDC_BTN_ERASE, &CEraseDlg::OnBnClickedBtnErase )
END_MESSAGE_MAP()


void CEraseDlg::OnOK()
{
	// disable ENTER
}


void CEraseDlg::OnBnClickedBtnErase()
{
	UpdateData();

	int num = 0;
	num += data.eraseKinopoisk;
	num += data.eraseCounters;
	num += data.eraseTags;
	num += data.eraseComments;
	num += data.eraseTechInfo;
	num += data.erasePosters;
	num += data.eraseThumbs;

	if( num )
		CDialog::OnOK();
	else
		CDialog::OnCancel();
}


void EraseCards( CollectionDB & cdb, Entry & root, EraseData & data )
{
	if( data.notEraseFolder )
	{
		data.notEraseFolder = FALSE;
	}
	else
	{
		if( data.eraseKinopoisk )
		{
			root.pluginID = PluginNone;
			root.filmId = NO_FILM_ID;

			root.title = GetNameFromFile( root.filePath.c_str() ).GetString();

			root.titleAlt.clear();

			root.description.clear();
			root.tagline.clear();
			root.seriesData = 0;
			root.genres = 0; // очищаем только жанры, прочую информацию сохраняем
			root.rating = 0;
			root.year = 0;
		}

		if( data.eraseCounters )
		{
			root.numViews = 0;
		}

		if( data.eraseTags )
		{
			root.tags.clear();
		}

		if( data.eraseComments )
		{
			root.comment.clear();
		}

		if( data.eraseTechInfo )
		{
			root.techInfo.clear();
		}
	}

	for( const EntryHandle & h : root.childs )
	{
		Entry * entry = cdb.FindEntry( h );
		if( entry )
			EraseCards( cdb, *entry, data );
	}
}

void ErasePosters( CollectionDB & cdb, Entry & root )
{
	if( root.posterId != DefaultPosterID )
	{
		cdb.GetPosterManager().RemovePoster( root.posterId );
		root.posterId = DefaultPosterID;
	}

	for( const EntryHandle & h : root.childs )
	{
		Entry * entry = cdb.FindEntry( h );
		if( entry )
			ErasePosters( cdb, *entry );
	}
}

void EraseThumbs( CollectionDB & cdb, Entry & root )
{
	const CString thumbName = cdb.thumbsDirectory + root.thisEntry.ToBase64() + L".jpg";
	if( PathFileExists( thumbName.GetString() ) )
	{
		// удаляем превьюшки
		::DeleteFile( thumbName );
	}

	for( const EntryHandle & h : root.childs )
	{
		Entry * entry = cdb.FindEntry( h );
		if( entry )
			EraseThumbs( cdb, *entry );
	}
}
