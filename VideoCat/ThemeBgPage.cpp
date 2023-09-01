// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ThemeBgPage.h"
#include "ViewStyle.h"

#include "resource.h"


IMPLEMENT_DYNAMIC(ThemeBgPage, CDialog)

ThemeBgPage::ThemeBgPage(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_THEME_BG, pParent)
	, _borderOpacity( 0 )
	, _glassBlur( 0 )
	, _glassOpacity( 0 )
{

}

ThemeBgPage::~ThemeBgPage()
{
}

void ThemeBgPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_IMAGE_PATH, _imageFile );
	DDX_Control( pDX, IDC_TOP_COLOR, _topColor );
	DDX_Control( pDX, IDC_BOTTOM_COLOR, _bottomColor );
	DDX_Control( pDX, IDC_BORDER_COLOR, _borderColor );
	DDX_Text( pDX, IDC_BORDER_OPACITY, _borderOpacity );
	DDV_MinMaxInt( pDX, _borderOpacity, 0, 255 );
	DDX_Control( pDX, IDC_GLASS_COLOR, _glassColor );
	DDX_Text( pDX, IDC_GLASS_BLUR, _glassBlur );
	DDV_MinMaxInt( pDX, _glassBlur, 0, 10 );
	DDX_Text( pDX, IDC_GLASS_OPACITY, _glassOpacity );
	DDV_MinMaxInt( pDX, _glassOpacity, 0, 255 );
	DDX_Control( pDX, IDC_BG_GROUP, _bgGroup );
	DDX_Control( pDX, IDC_SPIN2, _glassSpiner );
	DDX_Control( pDX, IDC_SPIN1, _borderSpiner );
	DDX_Control( pDX, IDC_SPIN3, _blurSpiner );
}


BEGIN_MESSAGE_MAP(ThemeBgPage, CDialog)
	ON_BN_CLICKED( IDC_DELETE_BG, &ThemeBgPage::OnBnClickedDeleteBg )
END_MESSAGE_MAP()


BOOL ThemeBgPage::OnInitDialog()
{
	CDialog::OnInitDialog();

	_imageFile.EnableFileBrowseButton( L"jpg", L"JPEG|*.jpg|PNG|*.png||" );

	_glassSpiner.SetRange( 0, 255 );
	_borderSpiner.SetRange( 0, 255 );
	_blurSpiner.SetRange( 0, 10 );

	return TRUE;
}


void ThemeBgPage::OnOK()
{
	//CDialog::OnOK();
}


void ThemeBgPage::OnCancel()
{
	//CDialog::OnCancel();
}

void ThemeBgPage::WriteData( const ViewStyle & style )
{
	if( !style.bgImageFile.IsEmpty() )
	{
		_imageFile.SetWindowText( style.bgImageFile );
	}

	_topColor.SetColor( style.bgTopColor );
	_bottomColor.SetColor( style.bgBottomColor );
	_borderColor.SetColor( style.tileItemSelected.borderColor );
	_borderOpacity = style.tileItemSelected.borderOpacity;
	_glassColor.SetColor( style.glassColor );
	_glassOpacity = style.glassOpacity;
	_glassBlur = style.glassBlur;

	UpdateData( FALSE );
}

void ThemeBgPage::ReadData( ViewStyle & style )
{
	UpdateData();

	_imageFile.GetWindowText( style.bgImageFile );

	style.bgTopColor = _topColor.GetColor();
	style.bgBottomColor = _bottomColor.GetColor();

	style.tileItemSelected.borderColor = _borderColor.GetColor();
	style.tileItemSelected.borderOpacity = _borderOpacity;
	style.listItemSelected.borderColor = _borderColor.GetColor();
	style.listItemSelected.borderOpacity = _borderOpacity;
	{
		style.tileItem.borderColor = _borderColor.GetColor();
		style.tileItem.borderOpacity = _borderOpacity;
		style.listItem.borderColor = _borderColor.GetColor();
		style.listItem.borderOpacity = _borderOpacity;
	}

	style.glassColor = _glassColor.GetColor();
	style.glassOpacity = _glassOpacity;
	style.glassBlur = _glassBlur;
}

void ThemeBgPage::OnBnClickedDeleteBg()
{
	_imageFile.SetWindowText( L"" );
	UpdateData( FALSE );
}

