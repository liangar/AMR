// StaticEx.cpp : implementation file
//

#include "stdafx.h"
#include "ht_meterproduct.h"
#include "StaticEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStaticEx

CStaticEx::CStaticEx()
{
    m_ForeColor = RGB(0,0,0);            //文字颜色（黑色）
    m_BackColor = RGB(255,255,255);      //背景色（白色）
    m_BkBrush.CreateSolidBrush(m_BackColor);    //背景刷
    p_Font = NULL;                       //字体指针
}

CStaticEx::~CStaticEx()
{
    if (p_Font)    delete p_Font;        //删除字体
}


BEGIN_MESSAGE_MAP(CStaticEx, CStatic)
	//{{AFX_MSG_MAP(CStaticEx)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStaticEx message handlers

HBRUSH CStaticEx::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CStatic::OnCtlColor(pDC, pWnd, nCtlColor);
	
    pDC->SetTextColor( m_ForeColor );         // 设置文字颜色
    pDC->SetBkColor( m_BackColor );           // 设置背景色

    return (HBRUSH)m_BkBrush.GetSafeHandle(); // 返回背景刷
}

//设置控件文字颜色

void CStaticEx::SetForeColor(COLORREF color)
{
    m_ForeColor = color;         //设置文字颜色
}

//设置控件背景颜色

void CStaticEx::SetBkColor(COLORREF color)
{
    m_BackColor = color;
    m_BkBrush.Detach();           //分离原背景刷
    m_BkBrush.CreateSolidBrush( m_BackColor ); //建立背景刷
}

//设置控件字体

void CStaticEx::SetTextFont(int FontHight,LPCTSTR FontName)
{
    if (p_Font)
		delete p_Font;    // 删除旧字体
    p_Font = new CFont;   // 生成字体对象
    p_Font->CreatePointFont( FontHight, FontName ); // 创建字体
    SetFont(p_Font);                // 设置控件字体
}
