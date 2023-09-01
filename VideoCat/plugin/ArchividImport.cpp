// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ArchividImport.h"
#include <filesystem>
#include "../resource.h"


IMPLEMENT_DYNAMIC(CArchividImport, CDialog)

CArchividImport::CArchividImport(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_ARCHIVID, pParent)
{

}

CArchividImport::~CArchividImport()
{
}

void CArchividImport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_ARCHIVID_DIR, archividDir );
}

BEGIN_MESSAGE_MAP(CArchividImport, CDialog)
END_MESSAGE_MAP()

void CArchividImport::OnOK()
{
	UpdateData();

	std::error_code errorCode;
	if( !std::filesystem::exists( std::filesystem::path( archividDir.GetString() ), errorCode ) )
	{
		CDialog::OnCancel();
		return;
	}

	CDialog::OnOK();
}
