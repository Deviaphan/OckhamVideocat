// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "NewFileExtension.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CNewFileExtension, CDialog)

CNewFileExtension::CNewFileExtension(CWnd* pParent /*=nullptr*/)
	: CDialog(CNewFileExtension::IDD, pParent)
	, newExtension( _T( "" ) )
{

}

CNewFileExtension::~CNewFileExtension()
{
}

void CNewFileExtension::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EXTENSION_NAME, newExtension );
}


BEGIN_MESSAGE_MAP(CNewFileExtension, CDialog)
END_MESSAGE_MAP()
