// MeterProduct.cpp : implementation file
//

#include "stdafx.h"

#include <l_str.h>
#include <xdatabase.h>
#include <lfile.h>

#include "ht_meterProduct.h"
#include "MeterProduct.h"

#include "xwork_trans.h"

extern char g_runpath[MAX_PATH];
xdatabase	g_db;
xsql		g_sql;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	
	// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA
	
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
//{{AFX_MSG_MAP(CAboutDlg)
// No message handlers
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMeterProduct dialog
IMPLEMENT_DYNAMIC(CMeterProduct, CResizablePage)

CMeterProduct::CMeterProduct(CWnd* pParent /*=NULL*/)
	: CResizablePage(CMeterProduct::IDD)
	, m_state(0)
	, m_maxid(0)
{
	//{{AFX_DATA_INIT(CMeterProduct)
	m_MeterID = _T("1");
	m_Port = _T("1");
	m_ResultMsg = _T("");
	m_TestOpenClose = FALSE;
	m_bAutoInc = FALSE;
	m_MeterBps = 0;
	m_MeterBps_old = 0;
	m_vendorid = _T("");
	//}}AFX_DATA_INIT

	m_psp.dwFlags &= (~PSP_HASHELP);
	m_psp.dwFlags |= PSP_USEHICON;
	m_psp.hIcon = AfxGetApp()->LoadIcon(IDI_TEST);

	m_lastMsg[0] = 0;
}


void CMeterProduct::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMeterProduct)
	DDX_Control(pDX, IDC_P_METERID, m_MeterID_Edit);
	DDX_Text(pDX, IDC_P_METERID, m_MeterID);
	DDX_Text(pDX, IDC_P_PORT, m_Port);
	DDX_Text(pDX, IDC_P_RESULT, m_ResultMsg);
	DDX_Check(pDX, IDC_P_OPENCLOSE, m_TestOpenClose);
	DDX_Check(pDX, IDC_P_AUTOINC, m_bAutoInc);
	DDX_CBIndex(pDX, IDC_P_Mbps, m_MeterBps);
	DDX_Text(pDX, IDC_P_METERID_HEAD, m_vendorid);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMeterProduct, CResizablePage)
	//{{AFX_MSG_MAP(CMeterProduct)
	ON_BN_CLICKED(IDC_P_START, OnStart)
	ON_MESSAGE(WM_NotifyMsg, OnNotifyMsg)
	ON_BN_CLICKED(IDC_P_OPEN_VALVE, OnPOpenValve)
	ON_BN_CLICKED(IDC_P_CLOSE_VALVE, OnPCloseValve)
	ON_BN_CLICKED(IDC_P_GETID, OnPGetID)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMeterProduct message handlers
BOOL CMeterProduct::OnInitDialog()
{
	CDialog::OnInitDialog();

	//此处设置的RGB值可以改变控件的背景色。   
	m_brush.CreateSolidBrush(RGB(255,255,255));   
    
	//此处设置控件的文字初始颜色。   
	m_color=RGB(0,0,0);
  
	AddAnchor(IDC_P_RESULT, TOP_LEFT, BOTTOM_RIGHT);

	//*
	CFont f;
	f.CreatePointFont(260, "宋体");
	GetDlgItem(IDC_PS_RESULT)->SetFont(&f);
	f.Detach();
	//*/
	/*
	m_PSResult.SetTextFont(120, "宋体");
	m_PSResult.SetForeColor(RGB(255, 160, 200));
	//*/
	m_color = RGB(50, 180, 80);

	char t[32], s[32];
	g_ini.get("生产", "厂商号", t, 5, "1");
	sprintf(s, "%04s", t);
	m_vendorid = s;

	g_ini.get("生产", "ID", t, 12, "0");
	sprintf(s, "%010s", t);
	m_MeterID  = s;

	g_ini.get("生产", "bps", t, sizeof(t)-1, "0");	m_MeterBps = atoi(t);
	g_ini.get("生产", "autoIncrease", t, sizeof(t)-1, "1");		m_bAutoInc = (t[0] != '0');
	g_ini.get("生产", "autoOpenClose", t, sizeof(t)-1, "0");	m_TestOpenClose = (t[0] != '0');

	/// 建立记录结果的数据库连接和文件
	g_ini.get("params", "dbconnect", m_dbconnectString, sizeof(m_dbconnectString), "driver={Microsoft Access Driver (*.mdb)};dbq=mts.mdb");
	int r = g_db.open(m_dbconnectString);
	if (r == XSQL_OK){
		g_sql.setdbconnection(&g_db);
		r = g_sql.geta("SELECT MAX(id) FROM mt_meter", XPARM_COL | XPARM_LONG, &m_maxid, XPARM_END);
		if (r != XSQL_OK && r != XSQL_WARNING){
			g_sql.geterror(m_lastMsg, sizeof(m_lastMsg));
			g_sql.endexec();
			g_db.close();
			MessageBox(m_lastMsg, "数据库文件不对", MB_OK);
		}
	}else
		MessageBox("数据库不能打开，表号将不能记录到数据库中。", "打开数据库出错", MB_ICONWARNING | MB_OK);

	char workdate[24];
	getsimple_now(workdate);  workdate[8] = 0;
	int l = sprintf(m_dataFileName, "+%s\\datapath\\%s", g_runpath, workdate);
	m_dataFileName[l-2] = 0;
	xsys_md(m_dataFileName+1, false);

	sprintf(m_dataFileName+l-2, "\\MT%s.CSV", workdate);
	///

	UpdateData(FALSE);

	return TRUE;
}

void CMeterProduct::OnOK()
{
	g_ini.set("生产", "厂商号", LPCTSTR(m_vendorid));
	g_ini.set("生产", "ID", LPCTSTR(m_MeterID));
	g_ini.set("生产", "bps", m_MeterBps);
	g_ini.set("生产", "autoIncrease", int(m_bAutoInc));
	g_ini.set("生产", "autoOpenClose", int(m_TestOpenClose));

	CDialog::OnOK();
}

void CMeterProduct::set_TransConfig(void)
{
	g_worker.set_notify_handle(m_hWnd, WM_NotifyMsg);
	g_worker.set_config(LPCTSTR(m_Port), "");
}

void CMeterProduct::OnStart()
{
	UpdateData();
	m_ResultMsg = "";
	UpdateData(FALSE);

	set_TransConfig();

	char b[64];
	if (m_MeterBps != m_MeterBps_old){
		m_state = 6;
		GetDlgItemText(IDC_P_Mbps, b, sizeof(b)-1);
		g_worker.notify("设置通讯速率", b);
		m_MeterBps_old = m_MeterBps;
	}else{
		m_state = 0;
		sprintf(b, "0,%s,%s", LPCTSTR(m_vendorid), LPCTSTR(m_MeterID));
		g_worker.notify("设置节点ID", b);
		// g_worker.notify("单表抄表", b);

		m_MeterID_Edit.SetSel(0, -1);
		m_MeterID_Edit.SetFocus();
	}
}

void CMeterProduct::s_record_result(const char * meterNumber)
{
	int r;

	++m_maxid;
	if (g_db.isopen()){
		r = g_sql.directexec(
			"INSERT INTO mt_meter(id,meterNumber,createTime) "
			"VALUES(?,?,now)",
			XPARM_IN  | XPARM_LONG, &m_maxid,
						XPARM_STR, meterNumber, 16,
			XPARM_END
		);
	}
	char result_rec[64], snow[24];
	getnowtime(snow);
	sprintf(result_rec, "%d\t%s\t%s\r\n", m_maxid, snow, meterNumber);
	r = write_file(m_dataFileName, result_rec, m_lastMsg, sizeof(m_lastMsg));
}

LRESULT CMeterProduct::OnNotifyMsg(WPARAM wParam, LPARAM lParam)
{
	const char * msg = (const char *)lParam;
	if (endcmp(m_lastMsg, msg) != 0){
		m_ResultMsg += msg;
		m_ResultMsg += "\r\n";
		SetDlgItemText(IDC_P_RESULT, LPCTSTR(m_ResultMsg));
		// UpdateData(FALSE);
		SendDlgItemMessage(IDC_P_RESULT, WM_VSCROLL, SB_BOTTOM);
		strncpy(m_lastMsg, msg, sizeof(m_lastMsg)-1);  m_lastMsg[sizeof(m_lastMsg)-1] = 0;
	}
	if (m_state != 4){
		if (g_worker.iserror()){
			m_color = RGB(255, 0, 0);
			SetDlgItemText(IDC_PS_RESULT, g_worker.get_state());
		}else if (g_worker.isok() || g_worker.isend()){
			if (ht_isok(g_worker.get_singleState())){
				m_color = RGB(50, 180, 80);
				if (m_state)
					s_record_result(LPCTSTR(m_MeterID));
			}else{
				m_color = RGB(255, 0, 0);
			}
//			char msg[256];
//			sprintf(msg, "%s: %s", LPCTSTR(m_MeterID), g_worker.get_singleMsg());
//			SetDlgItemText(IDC_PS_RESULT, msg);
			SetDlgItemText(IDC_PS_RESULT, g_worker.get_singleMsg());
		}else{
			m_color = RGB(0, 0, 0);
			SetDlgItemText(IDC_PS_RESULT, g_worker.get_state());
		}
	}

	char meterID[64];
	sprintf(meterID, "0,%s,%s", LPCTSTR(m_vendorid), LPCTSTR(m_MeterID));

	if (g_worker.isend()){
		if (g_worker.iserror())
			m_state = 4;

		if (m_state == 6){	// 设置表具通讯bps
			m_state = 0;
			sprintf(meterID, "0,%s,%s", LPCTSTR(m_vendorid), LPCTSTR(m_MeterID));
			g_worker.notify("设置节点ID", m_MeterID);
		}else if (m_state == 0){
			if (m_TestOpenClose){
				m_state = 8;
				g_worker.notify("关阀", meterID);
			}else{
				m_state = 3;
				g_worker.notify("调试用单表抄收", meterID);
			}
		}else if (m_state == 8){
			m_state = 1;
			g_worker.notify("调试用单表抄收", meterID);
		}else if (m_state == 1){
			if (g_worker.iserror() || !ht_isok(g_worker.get_singleState())){
				m_state = 3;
				g_worker.notify("调试用单表抄收", meterID);
			}else{
				m_state = 2;
				g_worker.notify("开阀", meterID);
			}
		}else if (m_state == 2){
			m_state = 3;
			g_worker.notify("调试用单表抄收", meterID);
		}else if (m_state == 3 || m_state == 7){
			if (m_bAutoInc && m_state == 3){
				double id = atof(LPCTSTR(m_MeterID)) + 1;
				sprintf(meterID, "%.0f", id);
				m_MeterID = meterID;
				UpdateData(FALSE);
			}
			g_worker.notify("down", "");
			m_state = 4;
		}else if (m_state == 5){	// 读表ID
			memcpy(meterID, "0,", 2);
			// format: "OK: <meterID 14>,<N>"
			memcpy(meterID+2, msg+4, 4);  meterID[6] = 0;
			SetDlgItemText(IDC_R_METERID_HEAD, meterID+2);
			meterID[6] = ',';
			getaword(meterID+7, (char *)msg+8, ',');
			SetDlgItemText(IDC_R_METERID, meterID+7);

			m_state = 7;
			g_worker.notify("调试用单表抄收", meterID);
		}
	}

	return 0;
}

void CMeterProduct::OnPOpenValve() 
{
	set_TransConfig();
	m_state = 4;

	char meterID[64];
	sprintf(meterID, "0,%s,%s", LPCTSTR(m_vendorid), LPCTSTR(m_MeterID));
	g_worker.notify("开阀", meterID);
}

void CMeterProduct::OnPCloseValve() 
{
	set_TransConfig();
	m_state = 4;

	char meterID[64];
	sprintf(meterID, "0,%s,%s", LPCTSTR(m_vendorid), LPCTSTR(m_MeterID));
	g_worker.notify("关阀", meterID);
}

void CMeterProduct::OnPGetID() 
{
	set_TransConfig();
	m_state = 5;

	g_worker.notify("读设备地址", "99");
}

HBRUSH CMeterProduct::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetDlgCtrlID() == IDC_PS_RESULT){
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(m_color);
	}   
	return (HBRUSH)m_brush.GetSafeHandle();
}
