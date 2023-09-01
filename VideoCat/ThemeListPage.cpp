// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ThemeListPage.h"
#include "ViewStyle.h"

#include "resource.h"


IMPLEMENT_DYNAMIC(ThemeListPage, CDialog)

ThemeListPage::ThemeListPage(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_THEME_LIST, pParent)
	, _sizeLine1( 0 )
	, _sizeLine2( 0 )
	, _sizeGenre( 0 )
	, _sizeTag( 0 )
{

}

ThemeListPage::~ThemeListPage()
{
}

void ThemeListPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_SIZE_LINE_1, _sizeLine1 );
	DDX_Text( pDX, IDC_SIZE_LINE_2, _sizeLine2 );
	DDX_Text( pDX, IDC_SIZE_TAG, _sizeTag );
	DDX_Text( pDX, IDC_SIZE_GENRE, _sizeGenre );
	DDX_Control( pDX, IDC_LIST_GROUP, _listGroup );
	DDX_Control( pDX, IDC_BUTTON_COLOR, _btnSelectedColor );
	DDX_Control( pDX, IDC_BUTTON_HOVER_COLOR, _btnSelectedHoverColor );
	DDX_Control( pDX, IDC_FONT, _fontName );
	DDX_Control( pDX, IDC_COLOR_LINE_1, _colorLine1 );
	DDX_Control( pDX, IDC_SELECTED_COLOR_LINE_1, _colorSelectedLine1 );
	DDX_Control( pDX, IDC_COLOR_LINE_2, _colorLine2 );
	DDX_Control( pDX, IDC_SELECTED_COLOR_LINE_2, _colorSelectedLine2 );
	DDX_Control( pDX, IDC_COLOR_TAG, _colorTag );
	DDX_Control( pDX, IDC_SELECTED_COLOR_TAG, _colorSelectedTag );
	DDX_Control( pDX, IDC_COLOR_GENRE, _colorGenre );
	DDX_Control( pDX, IDC_SELECTED_COLOR_GENRE, _colorSelectedGenre );
}


BEGIN_MESSAGE_MAP(ThemeListPage, CDialog)
	ON_BN_CLICKED( IDC_DELETE_FONT, &ThemeListPage::OnBnClickedDeleteFont )
END_MESSAGE_MAP()

void ThemeListPage::WriteData( const ViewStyle & style )
{
	_fontName.SelectFont( style.listItem.fontName );

	_btnSelectedColor.SetColor( style.listItemSelected.btnColor );
	_btnSelectedHoverColor.SetColor( style.listItemSelected.btnColorHover );

	_colorLine1.SetColor( style.listItem.colorTitle1 );
	_colorLine2.SetColor( style.listItem.colorTitle2 );
	_colorSelectedLine1.SetColor( style.listItemSelected.colorTitle1 );
	_colorSelectedLine2.SetColor( style.listItemSelected.colorTitle2 );
	_colorTag.SetColor( style.listItem.colorTag );
	_colorSelectedTag.SetColor( style.listItemSelected.colorTag );
	_colorGenre.SetColor( style.listItem.colorGenre );
	_colorSelectedGenre.SetColor( style.listItemSelected.colorGenre );
	_sizeLine1 = style.listItem.sizeTitle1;
	_sizeLine2 = style.listItem.sizeTitle2;
	_sizeTag = style.listItem.sizeTag;
	_sizeGenre = style.listItem.sizeGenre;

	UpdateData( FALSE );
}

void ThemeListPage::ReadData( ViewStyle & style )
{
	UpdateData();

	if( _fontName.GetSelFont() )
	{
		style.listItem.fontName = _fontName.GetSelFont()->m_strName;
		style.listItemSelected.fontName = _fontName.GetSelFont()->m_strName;
	}
	else
	{
		style.listItem.fontName.Empty();
		style.listItemSelected.fontName.Empty();
	}

	style.listItem.btnColor = _btnSelectedColor.GetColor();
	style.listItem.btnColorHover = _btnSelectedHoverColor.GetColor();
	style.listItemSelected.btnColor = _btnSelectedColor.GetColor();
	style.listItemSelected.btnColorHover = _btnSelectedHoverColor.GetColor();

	style.listItem.colorTitle1 = _colorLine1.GetColor();
	style.listItem.colorTitle2 = _colorLine2.GetColor();
	style.listItem.sizeTitle1 = _sizeLine1;
	style.listItem.sizeTitle2 = _sizeLine2;

	style.listItemSelected.colorTitle1 = _colorSelectedLine1.GetColor();
	style.listItemSelected.colorTitle2 = _colorSelectedLine2.GetColor();
	style.listItemSelected.sizeTitle1 = _sizeLine1;
	style.listItemSelected.sizeTitle2 = _sizeLine2;

	style.listItem.colorTag = _colorTag.GetColor();
	style.listItemSelected.colorTag = _colorSelectedTag.GetColor();
	style.listItem.colorGenre = _colorGenre.GetColor();
	style.listItemSelected.colorGenre = _colorSelectedGenre.GetColor();

	style.listItem.sizeTag = _sizeTag;
	style.listItemSelected.sizeTag = _sizeTag;

	style.listItem.sizeGenre = _sizeGenre;
	style.listItemSelected.sizeGenre = _sizeGenre;
}


void ThemeListPage::OnBnClickedDeleteFont()
{
	_fontName.SetCurSel( -1 );
	UpdateData( FALSE );
}
