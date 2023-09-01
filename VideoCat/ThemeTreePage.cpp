// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ThemeTreePage.h"
#include "ViewStyle.h"

#include "resource.h"



IMPLEMENT_DYNAMIC(ThemeTreePage, CDialog)

ThemeTreePage::ThemeTreePage(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_THEME_TREE, pParent)
	, _textSize( 0 )
{

}

ThemeTreePage::~ThemeTreePage()
{
}

void ThemeTreePage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_TREE_GROUP, _treeGroup );
	DDX_Control( pDX, IDC_FONT, _fontName );
	DDX_Text( pDX, IDC_TEXT_SIZE, _textSize );
	DDX_Control( pDX, IDC_COLOR, _color );
	DDX_Control( pDX, IDC_SELECTED_COLOR, _colorSelected );
}


BEGIN_MESSAGE_MAP(ThemeTreePage, CDialog)
	ON_BN_CLICKED( IDC_DELETE_FONT, &ThemeTreePage::OnBnClickedDeleteFont )
END_MESSAGE_MAP()

void ThemeTreePage::OnBnClickedDeleteFont()
{
	_fontName.SetCurSel( -1 );
	UpdateData( FALSE );
}

void ThemeTreePage::WriteData( const ViewStyle & style )
{
	_fontName.SelectFont( style.tileItem.fontName );

	_textSize = style.tree.fontSize;
	_color.SetColor( style.tree.textColor );
	_colorSelected.SetColor( style.tree.textColorSelected );

	UpdateData( FALSE );
}

void ThemeTreePage::ReadData( ViewStyle & style )
{
	UpdateData();

	if( _fontName.GetSelFont() )
	{
		style.tree.fontName = _fontName.GetSelFont()->m_strName;
	}
	else
	{
		style.tree.fontName.Empty();
	}

	style.tree.fontSize = _textSize;
	style.tree.textColor = _color.GetColor();
	style.tree.textColorSelected = _colorSelected.GetColor();
}
