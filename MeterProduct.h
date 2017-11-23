#pragma once
// CentorTest.h : header file
//

#include <treeprop/ResizablePage.h>
#include <treeprop/TreePropSheetUtil.hpp>

#include "StaticEx.h"

#include <xlist.h>

struct MP_failed{
	char meterNumber_ok[20];
	char meterNumber_failed[20];
};

struct MP_meter{
	char meterNumber[20];
	char createTime[32];
	char company[32];
};

/////////////////////////////////////////////////////////////////////////////
// CMeterProduct dialog

class CMeterProduct
	: public CResizablePage
	, public TreePropSheet::CWhiteBackgroundProvider
{
	DECLARE_DYNAMIC(CMeterProduct)

// Construction
public:
	CMeterProduct(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMeterProduct)
	enum { IDD = IDD_PRODUCT };
	CEdit	m_MeterID_Edit;
	CString	m_MeterID;
	CString	m_Port;
	CString	m_ResultMsg;
	BOOL	m_TestOpenClose;
	BOOL	m_bAutoInc;
	int 	m_MeterBps;	/// 波特率
	CString	m_MeterIDLen_Text;
	CString	m_vendorid;	/// 表具厂商ID(前4位数)
	CComboBox m_Company;	/// 客户名称选择
	CString m_strCompany;	/// 客户名称

	// CStaticEx	m_PSResult;
	//}}AFX_DATA

	int		m_MeterBps_old;
	char	m_sCompany[64];

	CBrush   m_brush;   
	COLORREF   m_color;   

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeterProduct)
	public:
	virtual void OnOK();
	virtual void OnCancel();

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_VIRTUAL

	char m_lastMsg[1024];
	int  m_state;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMeterProduct)
	afx_msg void OnStart();
	afx_msg LRESULT OnNotifyMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPOpenValve();
	afx_msg void OnPCloseValve();
	afx_msg void OnPGetID();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();

protected:
	void set_TransConfig(void);
	bool s_has_duplicate(const char * meterNumber);
	void s_record_result(const char * meterNumber);	/// 记录正确放码
	void s_record_failed(const char * meterNumber); /// 记录错误放码
	MP_failed * find_failed(const char * meterNumber);

	void set_meterid_parms(char * parms);

	void down(void);

protected:
	int		m_maxid;
	char	m_dbconnectString[MAX_PATH];
	char	m_dataFileName[MAX_PATH];
	bool	m_lock_error;

	int		m_MeterIDLen;
	char	m_lastOk[64];	/// 最近成功的号码
	int 	m_oks;
	int		m_dups;
	bool	m_isdup;

	unsigned char m_meter_type;	/// 表的通讯类型 0xX0，只用高位半字 2016-05-24
	unsigned char m_meter_type_new;	/// 写表ID后，新取得的
	bool	m_saved;

	MP_meter m_oldMeter;	/// 重号meter

	CString m_fullMeterID;

	xlist<MP_failed>	m_fails;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
