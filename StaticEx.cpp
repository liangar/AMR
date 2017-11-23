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
    m_ForeColor = RGB(0,0,0);            //������ɫ����ɫ��
    m_BackColor = RGB(255,255,255);      //����ɫ����ɫ��
    m_BkBrush.CreateSolidBrush(m_BackColor);    //����ˢ
    p_Font = NULL;                       //����ָ��
}

CStaticEx::~CStaticEx()
{
    if (p_Font)    delete p_Font;        //ɾ������
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
	
    pDC->SetTextColor( m_ForeColor );         // ����������ɫ
    pDC->SetBkColor( m_BackColor );           // ���ñ���ɫ

    return (HBRUSH)m_BkBrush.GetSafeHandle(); // ���ر���ˢ
}

//���ÿؼ�������ɫ

void CStaticEx::SetForeColor(COLORREF color)
{
    m_ForeColor = color;         //����������ɫ
}

//���ÿؼ�������ɫ

void CStaticEx::SetBkColor(COLORREF color)
{
    m_BackColor = color;
    m_BkBrush.Detach();           //����ԭ����ˢ
    m_BkBrush.CreateSolidBrush( m_BackColor ); //��������ˢ
}

//���ÿؼ�����

void CStaticEx::SetTextFont(int FontHight,LPCTSTR FontName)
{
    if (p_Font)
		delete p_Font;    // ɾ��������
    p_Font = new CFont;   // �����������
    p_Font->CreatePointFont( FontHight, FontName ); // ��������
    SetFont(p_Font);                // ���ÿؼ�����
}
