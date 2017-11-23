#if !defined(AFX_STATICEX_H__7F5F3175_4C06_4CB8_87AD_14E4B498A353__INCLUDED_)
#define AFX_STATICEX_H__7F5F3175_4C06_4CB8_87AD_14E4B498A353__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StaticEx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStaticEx window

class CStaticEx : public CStatic
{
// Construction
public:
	CStaticEx();

// Attributes
public:

// Operations
public:
	void SetForeColor(COLORREF color);      //�����ı���ɫ
	void SetBkColor(COLORREF color);        //���ñ�����ɫ
	void SetTextFont(int FontHight,LPCTSTR FontName);   //��������
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CStaticEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStaticEx)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
    COLORREF    m_ForeColor;        //�ı���ɫ
    COLORREF    m_BackColor;        //����ɫ
    CBrush      m_BkBrush;          //����ˢ
    CFont*      p_Font;             //����
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATICEX_H__7F5F3175_4C06_4CB8_87AD_14E4B498A353__INCLUDED_)
