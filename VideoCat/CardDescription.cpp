// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CardDescription.h"
#include <algorithm>
#include "Entry.h"
#include "CollectionDB.h"
#include "VideoCatDoc.h"
#include "Cyrillic.h"
#include "resource.h"

struct UpdateTags: public Traverse::Base
{
	TagId from;
	TagId to;

	virtual Traverse::Result Run( Entry & entry ) override
	{
		// проверяется наличие старой и новой метки
		auto ifrom = std::find( entry.tags.begin(), entry.tags.end(), from );
		auto ito = std::find( entry.tags.begin(), entry.tags.end(), to );

		if( ifrom != entry.tags.end() )
		{
			// если новой метки нет, то переименовывается старая
			// если новая уже есть, то ужаляем стару, чтобы не было дубликата
			if( ito == entry.tags.end() )
				*ifrom = to;
			else
				entry.tags.erase( ifrom );
		}

		return Traverse::CONTINUE;
	}

};


IMPLEMENT_DYNAMIC(CCardDescription, CMFCPropertyPage )

CCardDescription::CCardDescription()
	: CMFCPropertyPage(IDD_TAB_DESCRIPTION)
	, _tagManager(nullptr)
	, _newTagName(L"")
{

}

CCardDescription::~CCardDescription()
{
}

void CCardDescription::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );

	DDX_Text( pDX, IDC_EDIT_TAGLINE, _tagline );
	DDX_Text( pDX, IDC_EDIT_DESCRITPION, _description );
	DDX_Text( pDX, IDC_EDIT_COMMENT, _comment );
	DDX_Text( pDX, IDC_NEW_TAG_NAME, _newTagName );
	DDX_Control( pDX, IDC_ALL_TAGS, _allTags );
	DDX_Control( pDX, IDC_USED_TAGS, _usedTags );
}

BEGIN_MESSAGE_MAP(CCardDescription, CMFCPropertyPage )
	ON_BN_CLICKED( IDC_BTN_NEW_TAG, &CCardDescription::OnNewTag )
	ON_BN_CLICKED( IDC_BTN_RENAME_TAG, &CCardDescription::OnRenameTag )
	ON_BN_CLICKED( IDC_BTN_DELETE_TAG, &CCardDescription::OnDeleteTag )
	ON_BN_CLICKED( IDC_CLEAR_TAGS, &CCardDescription::OnClearTags )
	ON_LBN_DBLCLK( IDC_USED_TAGS, &CCardDescription::OnRemoveUsedTags )
	ON_LBN_DBLCLK( IDC_ALL_TAGS, &CCardDescription::OnAddFromAllTags )
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

BOOL CCardDescription::OnInitDialog()
{
	_tagManager = &doc->GetCurrentCollection()->GetTagManager();

	CMFCPropertyPage::OnInitDialog();

	return TRUE;
}

void CCardDescription::ReinitTab()
{
	VC_TRY;
	__super::ReinitTab();

	_tagline = entry->tagline.c_str();
	_description = entry->description.c_str();
	_comment = entry->comment.c_str();

	for( TagId id : entry->tags )
	{
		_currentTags.insert( id );
	}

	UpdateFilmTags();
	UpdateAllTags();

	UpdateData( FALSE );

	VC_CATCH( ... )
	{
		;
	}
}

bool CCardDescription::SaveData()
{
	if( !initialized )
		return false;

	UpdateData();

	entry->tagline = _tagline;
	entry->description = _description;
	entry->comment = _comment;

	std::set<TagId> oldTagList( entry->tags.begin(), entry->tags.end() );
	if( oldTagList != _currentTags )
	{
		entry->tags.clear();
		entry->tags.insert( entry->tags.end(), _currentTags.begin(), _currentTags.end() );
	}

	return true;
}

void CCardDescription::OnNewTag()
{
	UpdateData();
	_newTagName.Trim();
	if( _newTagName.IsEmpty() )
		return;

	ToLower( _newTagName );

	TagId id = _tagManager->AddTag( _newTagName.GetString() );
	_currentTags.insert( id );

	doc->InvalidateDatabase();

	UpdateFilmTags();
	UpdateAllTags();
}

void CCardDescription::OnRenameTag()
{
	UpdateData();

	const int index = _allTags.GetCurSel();
	if( index < 0 )
		return;

	TagId id = (TagId)_allTags.GetItemData( index );

	std::pair<TagId, TagId> result = _tagManager->RenameTag( id, _newTagName.GetString() );

	if( result.first != EMPTY_TAG )
	{
		UpdateTags updateTags;
		updateTags.from = result.first;
		updateTags.to = result.second;

		doc->GetCurrentCollection()->ForEach( &updateTags );
	}

	ReinitTab();

	doc->InvalidateDatabase();
}

void CCardDescription::OnDeleteTag()
{
	UpdateData();

	const int index = _allTags.GetCurSel();
	if( index < 0 )
		return;

	TagId id = (TagId)_allTags.GetItemData( index );
	_tagManager->DeleteTag( id );

	doc->InvalidateDatabase();

	_currentTags.erase( id );

	UpdateFilmTags();
	UpdateAllTags();
}

void CCardDescription::UpdateFilmTags()
{
	_usedTags.ResetContent();

	for( TagId id : _currentTags )
	{
		const std::wstring tagName = _tagManager->GetTag( id );
		if( tagName.empty() )
			continue;

		const int index = _usedTags.AddString( tagName.c_str() );
		_usedTags.SetItemData( index, (DWORD_PTR)id );
	}
}

void CCardDescription::UpdateAllTags()
{
	_allTags.ResetContent();

	const TagManager::AllTags & allTags = _tagManager->GetAllTags();

	for( auto & tag : allTags )
	{
		const int index = _allTags.AddString( tag.second.c_str() );
		_allTags.SetItemData( index, (DWORD_PTR)tag.first );
	}
}

void CCardDescription::OnClearTags()
{
	_currentTags.clear();
	_text.Empty();

	UpdateData( FALSE );

	UpdateFilmTags();
}

void CCardDescription::OnRemoveUsedTags()
{
	const int index = _usedTags.GetCurSel();
	if( index < 0 )
		return;

	TagId id = (TagId)_usedTags.GetItemData( index );
	_currentTags.erase( id );

	UpdateFilmTags();
}

void CCardDescription::OnAddFromAllTags()
{
	const int index = _allTags.GetCurSel();
	if( index < 0 )
		return;

	TagId id = (TagId)_allTags.GetItemData( index );

	_currentTags.insert( id );

	UpdateFilmTags();
}

void CCardDescription::OnShowWindow( BOOL bShow, UINT nStatus )
{
	if( bShow == FALSE )
		SaveData();

	__super::OnShowWindow( bShow, nStatus );
}
