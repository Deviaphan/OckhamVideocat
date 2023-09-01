// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "FileFormatSettings.h"
#include "NewFileExtension.h"
#include <algorithm>


IMPLEMENT_DYNAMIC( CFileFormatSettings, CPropertyPage)

CFileFormatSettings::CFileFormatSettings( )
	: _commandLine( _T( "" ) )
	, _playerType( 0 )
	, _lastSel(-1)
	, _special3d( FALSE )
{

}

CFileFormatSettings::~CFileFormatSettings()
{
}

void CFileFormatSettings::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_LIST_FORMATS, _extList );
	DDX_Control( pDX, IDC_PLAYER_PATH, _browsePlayer );
	DDX_Text( pDX, IDC_PLAYER_COMMANDS, _commandLine );
	DDX_Check( pDX, IDC_3D_INDIVIDUAL, _special3d );
	DDX_Radio( pDX, IDC_RADIO_DEFAULT_PLAYER, _playerType );
}


BEGIN_MESSAGE_MAP(CFileFormatSettings, CPropertyPage)
	ON_BN_CLICKED( IDC_BTN_ADD_FORMAT, &CFileFormatSettings::OnBnClickedBtnAddFormat )
	ON_BN_CLICKED( IDC_BTN_REMOVE_FORMAT, &CFileFormatSettings::OnBnClickedBtnRemoveFormat )
	ON_LBN_SELCHANGE( IDC_LIST_FORMATS, &CFileFormatSettings::OnLbnSelchangeListFormats )
END_MESSAGE_MAP()


void CFileFormatSettings::OnBnClickedBtnAddFormat()
{
	CNewFileExtension newExt;
	if( IDOK != newExt.DoModal() )
		return;

	if( newExt.newExtension.IsEmpty() )
		return;

	extensions.emplace_back(  newExt.newExtension  );
	UpdateList();
}


void CFileFormatSettings::OnBnClickedBtnRemoveFormat()
{
	if( _lastSel < 0 )
		return;

	if( extensions.at( _lastSel ).extension == Film3dExtension )
		return;

	if( IDYES != AfxMessageBox( L"Удалить выбранный тип файлов?", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST ) )
		return;

	extensions.erase( extensions.begin() + _lastSel );
	UpdateList();
}

void CFileFormatSettings::UpdateList()
{
	_extList.ResetContent();

	std::sort( extensions.begin(), extensions.end() );

	for( int i = 0; i < (int)extensions.size(); ++i )
	{
		_extList.InsertString( i, extensions.at( i ).extension );
	}

	_lastSel = -1;
}


BOOL CFileFormatSettings::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	UpdateList();

	return TRUE;
}

void CFileFormatSettings::SaveLastState()
{
	UpdateData();

	if( _lastSel >= 0 )
	{
		ExtData & extData = extensions.at(_lastSel );
		
		if( _playerType == 0 )
			extData.player.Empty();
		else
			_browsePlayer.GetWindowText( extData.player );

		extData.commandLine = _commandLine;
		extData.ignoreIn3d = _special3d;
	}
}

void CFileFormatSettings::OnLbnSelchangeListFormats()
{
	SaveLastState();

	const int curSel = _extList.GetCurSel();
	if( curSel < 0 )
		return;
	
	_lastSel = curSel;

	const ExtData & extData = extensions.at( curSel );
	_special3d = extData.ignoreIn3d;
	_browsePlayer.SetWindowText( extData.player );
	_commandLine = extData.commandLine;

	_playerType = !extData.player.IsEmpty();

	UpdateData( FALSE );
}


void CFileFormatSettings::OnOK()
{
	// stub
}

void CFileFormatSettings::OnCancel()
{
	// stub
}

