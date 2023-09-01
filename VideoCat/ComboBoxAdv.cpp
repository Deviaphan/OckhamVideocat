// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ComboBoxAdv.h"


ComboBoxAdv::ComboBoxAdv()
{
}


ComboBoxAdv::~ComboBoxAdv()
{
}
BEGIN_MESSAGE_MAP( ComboBoxAdv, CComboBox )
	ON_WM_CTLCOLOR()
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

void ComboBoxAdv::SetBackColor( COLORREF col )
{
	m_Brush.DeleteObject();

	m_BackColor = col;
	m_Brush.CreateSolidBrush( m_BackColor );


	UpdateCtrl();
}

HBRUSH ComboBoxAdv::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	//return CComboBox::OnCtlColor( pDC, pWnd, nCtlColor );
	if( !m_Brush.m_hObject )
	{
		m_Brush.CreateSolidBrush( RGB(0,255,0) );
	}

	pDC->SetBkColor( RGB( 255, 0, 0 ) );
	pDC->SetTextColor( RGB( 0, 0, 255 ) );

	return (HBRUSH)m_Brush;
}

void ComboBoxAdv::UpdateCtrl()
{

}



//HBRUSH ComboBoxAdv::OnCtlColor( CDC * pDC, CWnd * pWnd, UINT nCtlColor )
//{
//	if( !m_Brush.m_hObject )
//	{
//		m_Brush.CreateSolidBrush( m_BackColor );
//	}
//
//	pDC->SetBkColor( m_BackColor );
//	pDC->SetTextColor(  m_TextColor );
//
//	return (HBRUSH)m_Brush;
//}




HBRUSH ComboBoxAdv::CtlColor( CDC * pDC, UINT /*nCtlColor*/ )
{
	if( !m_Brush.m_hObject )
	{
		m_Brush.CreateSolidBrush( RGB( 0, 255, 0 ) );
	}

	pDC->SetBkColor( RGB( 255, 0, 0 ) );
	pDC->SetTextColor( RGB( 0, 0, 255 ) );

	return (HBRUSH)m_Brush;
}
