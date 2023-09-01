// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CopyFolderDataDlg.h"
#include "ResString.h"
#include "resource.h"



IMPLEMENT_DYNAMIC(CopyFolderDataDlg, CDialog)

CopyFolderDataDlg::CopyFolderDataDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_COPY_FOLDER_DATA, pParent)
{

}

CopyFolderDataDlg::~CopyFolderDataDlg()
{
}

void CopyFolderDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMMENT, _commentBox );
	DDX_Check( pDX, IDC_COPY_DESCROPTION, data.description );
	DDX_Check( pDX, IDC_COPY_POSTER, data.poster );
	DDX_Check( pDX, IDC_COPY_TAGS, data.tags );
	DDX_Check( pDX, IDC_COPY_COMMENT, data.comments );
}


BEGIN_MESSAGE_MAP(CopyFolderDataDlg, CDialog)
END_MESSAGE_MAP()



BOOL CopyFolderDataDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	_commentBox.SetText( ResString( IDS_COMMENT_COPY_FOLDER ) );

	return TRUE;
}

void CopyCards( CollectionDB & cdb, Entry & root, CopyData & data )
{
	for( const EntryHandle & h : root.childs )
	{
		Entry * entry = cdb.FindEntry( h );
		if( !entry )
			continue;

		if( entry->IsFolder() )
			continue;

		if( data.description )
		{
			entry->title = root.title;
			entry->titleAlt = root.titleAlt;
			entry->description = root.description;
			entry->filmId = root.filmId;
			entry->genres = root.genres;
			entry->rating = root.rating;
			entry->tagline = root.tagline;
			entry->year = root.year;
		}

		if( data.tags )
		{
			entry->tags = root.tags;
		}

		if( data.comments )
		{
			entry->comment = root.comment;
		}
	}
}

void CopyPosters( CollectionDB & cdb, Entry & root )
{
	Gdiplus::Bitmap * rootPoster = nullptr;

	if( root.posterId != DefaultPosterID )
		cdb.GetPosterManager().LoadPoster( root.posterId, rootPoster );

	for( const EntryHandle & h : root.childs )
	{
		Entry * entry = cdb.FindEntry( h );
		if( !entry )
			continue;

		if( entry->IsFolder() )
			continue;

		cdb.GetPosterManager().RemovePoster( entry->posterId );
		entry->posterId = DefaultPosterID;

		if( rootPoster )
		{
			entry->posterId = cdb.GetPosterManager().SaveImage( rootPoster );
		}
	}
}
