// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "EditToolbarDlg.h"
#include <algorithm>
#include "GlobalSettings.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(EditToolbarDlg, CDialog)

EditToolbarDlg::EditToolbarDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_EDIT_TOOLBAR, pParent)
{

}

EditToolbarDlg::~EditToolbarDlg()
{
}

void EditToolbarDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_TOOLBAR_LIST, _toolbarList );
	DDX_Control( pDX, IDC_COMMAND_LIST, _commandList );
}


BEGIN_MESSAGE_MAP(EditToolbarDlg, CDialog)
	ON_LBN_DBLCLK( IDC_TOOLBAR_LIST, &EditToolbarDlg::OnLbnDblclkToolbarList )
	ON_LBN_DBLCLK( IDC_COMMAND_LIST, &EditToolbarDlg::OnLbnDblclkCommandList )
	ON_BN_CLICKED( IDC_MOVE_UP, &EditToolbarDlg::OnBnClickedMoveUp )
	ON_BN_CLICKED( IDC_MOVE_DOWN, &EditToolbarDlg::OnBnClickedMoveDown )
END_MESSAGE_MAP()


BOOL EditToolbarDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CommandExecute::Instance().GetAllCommands( _allCommands );

	std::sort( _allCommands.begin(), _allCommands.end(), []( const auto & lhs, const auto & rhs )->bool { return lhs.title < rhs.title; } );

	for( const auto & command : _allCommands )
	{
		_commandList.AddString( command.title );
	}

	_selectedCommands = GetGlobal().toolbarCommands;

	UpdateToolbarList();

	return TRUE;
}

void EditToolbarDlg::OnOK()
{
	GetGlobal().SetTollbarCommands( _selectedCommands );

	CDialog::OnOK();
}



void EditToolbarDlg::UpdateToolbarList()
{
	_toolbarList.ResetContent();

	for( CommandID id : _selectedCommands )
	{
		_toolbarList.AddString( CommandExecute::Instance().GetTitle( id ) );
	}
}


void EditToolbarDlg::OnLbnDblclkToolbarList()
{
	const int index = _toolbarList.GetCurSel();
	if( index < 0 )
		return;

	_selectedCommands.erase( _selectedCommands.begin() + index );

	UpdateToolbarList();
}


void EditToolbarDlg::OnLbnDblclkCommandList()
{
	const int index = _commandList.GetCurSel();
	if( index < 0 )
		return;

	for( CommandID id : _selectedCommands )
	{
		if( id == _allCommands[index].id )
			return;
	}

	_selectedCommands.push_back( _allCommands[index].id );

	UpdateToolbarList();
}


void EditToolbarDlg::OnBnClickedMoveUp()
{
	const int index = _toolbarList.GetCurSel();
	if( index <= 0 )
		return;

	std::swap( _selectedCommands[index], _selectedCommands[index - 1] );

	UpdateToolbarList();

	_toolbarList.SetCurSel( index - 1 );
}


void EditToolbarDlg::OnBnClickedMoveDown()
{
	const int index = _toolbarList.GetCurSel();
	if( index < 0 )
		return;

	if( index == (_selectedCommands.size() - 1) )
		return;

	std::swap( _selectedCommands[index], _selectedCommands[index + 1] );

	UpdateToolbarList();

	_toolbarList.SetCurSel( index + 1 );
}


