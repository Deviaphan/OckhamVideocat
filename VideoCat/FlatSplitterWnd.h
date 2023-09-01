#pragma once



class CFlatSplitterWnd: public CSplitterWnd
{
public:
	CFlatSplitterWnd();
	virtual ~CFlatSplitterWnd();

	void OnDrawSplitter( CDC * pDC, ESplitType nType, const CRect & rectArg ) override;
	void RecalcLayout() override;    // call after changing sizes

	static void DeferClientPos( AFX_SIZEPARENTPARAMS * lpLayout, CWnd * pWnd, int x, int y, int cx, int cy, BOOL bScrollBar );

	static void LayoutRowCol( CSplitterWnd::CRowColInfo * pInfoArray, int nMax, int nSize, int nSizeSplitter );

protected:
	DECLARE_MESSAGE_MAP()
};
