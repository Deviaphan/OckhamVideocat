#pragma once
#include <afxbutton.h>
class CommentButton: public CMFCButton
{
	DECLARE_DYNAMIC( CommentButton )
public:
	CommentButton();
	~CommentButton() override;

public:
	void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct ) override;

protected:
	DECLARE_MESSAGE_MAP()

public:
	CString comment;
};

