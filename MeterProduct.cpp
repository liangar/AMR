// MeterProduct.cpp : implementation file
//

#include "stdafx.h"

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#include <l_str.h>
#include <xdatabase.h>
#include <lfile.h>

#include <xshare_recv_mem.h>

#include "ht_meterProduct.h"
#include "MeterProduct.h"

#include "xwork_trans.h"

#define MP_RGB_GREEN	RGB(25, 100, 50)

extern char g_runpath[MAX_PATH];

extern xshare_recv_mem	g_msg_mem;

xdatabase	g_db;
xsql		g_sql;

static int	g_cmd = 1;	// 1/2/3/7 = 放码/开阀/关阀/读表ID

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static void playAlarm(void)
{
	PlaySound("Alarm01.wav",NULL, SND_ASYNC |SND_NODEFAULT);
}

static void playDing(void)
{
	PlaySound("Ding.wav",NULL, SND_ASYNC |SND_NODEFAULT);
}

static void playOk(void)
{
	PlaySound("Ok.wav",NULL, SND_ASYNC |SND_NODEFAULT);
}

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
	, m_meter_type(0)
{
	//{{AFX_DATA_INIT(CMeterProduct)
	m_MeterID = _T("1");
	m_Port = _T("1");
	m_ResultMsg = _T("");
	m_TestOpenClose = FALSE;
	m_bAutoInc = FALSE;
	m_MeterBps = 0;
	m_MeterIDLen_Text = "10";
	m_MeterBps_old = 0;
	m_vendorid = _T("");
	//}}AFX_DATA_INIT

	m_MeterIDLen = 10;
	m_psp.dwFlags &= (~PSP_HASHELP);
	m_psp.dwFlags |= PSP_USEHICON;
	m_psp.hIcon = AfxGetApp()->LoadIcon(IDI_TEST);

	m_lastMsg[0] = 0;
	m_saved = m_isdup = false;
}


void CMeterProduct::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMeterProduct)
	DDX_Control(pDX, IDC_P_METERID, m_MeterID_Edit);
	DDX_Control(pDX, IDC_COMB_COMPANY, m_Company);
	DDX_Text(pDX, IDC_P_METERID, m_MeterID);
	DDX_Text(pDX, IDC_P_PORT, m_Port);
	DDX_Text(pDX, IDC_P_RESULT, m_ResultMsg);
	DDX_Check(pDX, IDC_P_OPENCLOSE, m_TestOpenClose);
	DDX_Check(pDX, IDC_P_AUTOINC, m_bAutoInc);
	DDX_CBIndex(pDX, IDC_P_Mbps, m_MeterBps);
	DDX_Text(pDX, IDC_P_LEN, m_MeterIDLen_Text);
	DDX_Text(pDX, IDC_P_METERID_HEAD, m_vendorid);
	//}}AFX_DATA_MAP

	m_MeterIDLen = atoi(LPCSTR(m_MeterIDLen_Text));
	GetDlgItemText(IDC_COMB_COMPANY, m_sCompany, sizeof(m_sCompany));
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
	f.CreatePointFont(220, "宋体");
	GetDlgItem(IDC_PS_RESULT)->SetFont(&f);
	f.Detach();
	f.CreatePointFont(150, "宋体");
	GetDlgItem(IDC_oks)->SetFont(&f);
	f.Detach();
	f.CreatePointFont(150, "宋体");
	GetDlgItem(IDC_dups)->SetFont(&f);
	f.Detach();
	//*/
	/*
	m_PSResult.SetTextFont(120, "宋体");
	m_PSResult.SetForeColor(RGB(255, 160, 200));
	//*/
	m_color = MP_RGB_GREEN;

	char t[4096], s[32];
	g_ini.get("params", "portNumber", t, sizeof(t), "1");  m_Port = t;

	g_ini.get("生产", "bps", t, sizeof(t)-1, "0");	m_MeterBps = atoi(t);
	g_ini.get("生产", "IDlen", t, sizeof(t)-1, "0");m_MeterIDLen = atoi(t);  m_MeterIDLen_Text = t;
	
	g_ini.get("生产", "ID", t, 16, "0");
	trimstr0(t);
	ht_lpad(t, m_MeterIDLen);
	m_MeterID  = t;

	if (m_MeterIDLen > 4){
		g_ini.get("生产", "厂商号", t, 8, "1");
		sprintf(s, "%04d", atoi(t));
		m_vendorid = s;
	}else
		m_vendorid = "0000";

	g_ini.get("生产", "autoIncrease", t, sizeof(t)-1, "1");		m_bAutoInc = (t[0] != '0');
	g_ini.get("生产", "autoOpenClose", t, sizeof(t)-1, "0");	m_TestOpenClose = (t[0] != '0');

	g_ini.get("生产", "companys", t, sizeof(t)-1, "骏普");
	{
		const char * p = t;
		for (p = getaword(t, p, ','); t[0]; p = getaword(t, p, ',')){
			m_Company.AddString(t);
		}
	}

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

	g_ini.get("生产", "comp", t, sizeof(t)-1, "骏普");
	m_Company.SetCurSel(m_Company.FindString(0, t));

	m_fails.init(8, 8);
	memset(m_lastOk, 0, sizeof(m_lastOk));
	m_oks = m_dups = 0;
	m_lock_error = false;

	return TRUE;
}

void CMeterProduct::down(void)
{
	g_sql.endexec();
	g_db.endtrans(true);

	m_fails.free_all();
}

void CMeterProduct::OnOK()
{
	down();
	
	g_ini.set("params", "portNumber", LPCTSTR(m_Port));

	g_ini.set("生产", "厂商号", LPCTSTR(m_vendorid));
	g_ini.set("生产", "ID", LPCTSTR(m_MeterID));
	g_ini.set("生产", "bps", m_MeterBps);
	g_ini.set("生产", "IDlen", m_MeterIDLen_Text);

	g_ini.set("生产", "autoIncrease", int(m_bAutoInc));
	g_ini.set("生产", "autoOpenClose", int(m_TestOpenClose));
	g_ini.set("生产", "comp", m_sCompany);

	CDialog::OnOK();
}

void CMeterProduct::OnCancel()
{
	down();
	
	g_ini.set("生产", "ID", LPCTSTR(m_MeterID));

	CDialog::OnCancel();
}

void CMeterProduct::set_TransConfig(void)
{
	g_worker.set_noMeterState(m_MeterIDLen<5);

	g_worker.set_notify_handle(m_hWnd, WM_NotifyMsg);
	g_worker.set_config(LPCTSTR(m_Port), "");
}

void CMeterProduct::OnStart()
{
	g_cmd = 1;

	CString mid, hid;
	char vendorid[16], len_string[16];
	m_MeterID_Edit.GetWindowText(mid);
	GetDlgItemText(IDC_P_LEN, len_string, 16);
	m_MeterIDLen = atoi(len_string);
	m_MeterIDLen_Text = len_string;

	m_saved = m_isdup = false;

	GetDlgItemText(IDC_P_METERID_HEAD, vendorid, 8);
//	GetDlgItem(IDC_P_METERID_HEAD)->GetWindowText(hid);
	if (m_lock_error &&
		(m_MeterID.Compare(LPCTSTR(mid)) || m_vendorid.Compare(vendorid))
		)
	{
		playDing();
		sprintf(m_lastMsg, "放弃对表 %s 的处理吗？\n放弃请点击“是”", LPCTSTR(m_MeterID));
		if (MessageBox(m_lastMsg, "请确认",  MB_YESNO) != IDYES){
			return;
		}
	}

	// 长度不一致告警
	if (m_MeterIDLen != int(strlen(mid))){
		playDing();
		MessageBox("表号长度与位数设置不一致，不能放码！", "表号长度错误", MB_ICONWARNING | MB_OK);
		return;
	}

	char b[64];
	m_MeterID = mid;
	set_meterid_parms(b);

	m_fullMeterID = m_vendorid;
	m_fullMeterID += m_MeterID;

	MP_failed * p_failed = find_failed(m_fullMeterID);
	if (p_failed){
		playDing();
		sprintf(m_lastMsg, "曾经失败过的号码：%s\r\n"
			"该号码之前的成功放码为：[%s]\r\n"
			"请检查之前成功放码的是否被改写。是否继续本次放码操作？",
			p_failed->meterNumber_failed,
			p_failed->meterNumber_ok
		);
		if (MessageBox(m_lastMsg, "疑似误操作告警", MB_ICONWARNING | MB_YESNO) != IDYES)
			return;
	}

	if (s_has_duplicate(m_fullMeterID)){
		playDing();
		sprintf(m_lastMsg, "重号: %s\r\n"
			"厂商：%s\r\n"
			"最近扫码时间：%s\r\n"
			"是否继续本次放码操作？",
			m_oldMeter.meterNumber,
			m_oldMeter.company,
			m_oldMeter.createTime
		);
		if (MessageBox(m_lastMsg, "重号警告", MB_ICONWARNING | MB_YESNO) != IDYES)
			return;
		m_isdup = true;
	}

	UpdateData();
	m_ResultMsg = "";
	UpdateData(FALSE);

	set_TransConfig();

	g_msg_mem.clear_all_recv();

	if (m_MeterBps != m_MeterBps_old){
		m_state = 6;
		GetDlgItemText(IDC_P_Mbps, b, sizeof(b)-1);
		g_worker.notify("系统检修", b);
		m_MeterBps_old = m_MeterBps;
	}else{
		m_state = 0;
		g_worker.notify("设置节点ID", b);
		// g_worker.notify("单表抄表", b);

		m_MeterID_Edit.SetSel(0, -1);
		m_MeterID_Edit.SetFocus();
	}
}

bool CMeterProduct::s_has_duplicate(const char * meterNumber)
{
	if (!g_db.isopen())
		return false;

	int dup_count = 0;
	int len = strlen(meterNumber);  // m_meterIDlen;
	int r = g_sql.geta(
		"SELECT COUNT(*) FROM mt_meter "
		"WHERE meterNumber=? AND DateDiff('d', createtime, now) < 60",
		XPARM_COL | XPARM_LONG,&dup_count,
		XPARM_IN  | XPARM_STR, meterNumber, len,
		XPARM_END
	);
	if (r != XSQL_OK){
		g_sql.endexec();
		return false;
	}
	
	if (dup_count > 0){
		memset(&m_oldMeter, 0, sizeof(m_oldMeter));
		r = g_sql.geta(
			"SELECT meterNumber, createTime, company FROM mt_meter "
			"WHERE meterNumber=? AND DateDiff('d', createtime, now) < 60 "
			"ORDER BY createTime DESC",
			XPARM_COL | XPARM_STR, m_oldMeter.meterNumber, sizeof(m_oldMeter.meterNumber),
						XPARM_STR, m_oldMeter.createTime, sizeof(m_oldMeter.createTime),
						XPARM_STR, m_oldMeter.company, sizeof(m_oldMeter.company),
			XPARM_IN  | XPARM_STR, meterNumber, len,
			XPARM_END
		);
		if (r != XSQL_OK){
			g_sql.endexec();
			return false;
		}
	}
	g_sql.endexec();

	return (dup_count > 0);
}

MP_failed * CMeterProduct::find_failed(const char * meterNumber)
{
//	int l = strlen(meterNumber);
//	meterNumber += (l > 10)?l-10 : 0;
	int i;
	for (i = 0; i < m_fails.m_count; i++){
		MP_failed * p = m_fails.m_phandles + i;
		if (strcmp(p->meterNumber_failed, meterNumber) == 0)
			return p;
	}
	return NULL;
}

void CMeterProduct::s_record_result(const char * meterNumber)
{
	int r;
	
	++m_maxid;
	if (g_db.isopen()){
		char msg[64];
		if (m_isdup){
			r = g_sql.directexec(
				"UPDATE mt_meter SET createTime=now "
				"WHERE"
				"	meterNumber=? AND"
				"	DateDiff('d', createtime, now) < 60",
				XPARM_IN  | XPARM_STR, meterNumber, 16,
				XPARM_END
			);
			++m_dups;
			sprintf(msg, "%d", m_dups);
			SetDlgItemText(IDC_dups, msg);
		}else{
			r = g_sql.directexec(
				"INSERT INTO mt_meter(id,meterNumber,createTime,company) "
				"VALUES(?,?,now,?)",
				XPARM_IN  | XPARM_LONG, &m_maxid,
							XPARM_STR, meterNumber, 16,
							XPARM_STR, m_sCompany, strlen(m_sCompany),
				XPARM_END
			);
			++m_oks;
			sprintf(msg, "%d", m_oks);
			SetDlgItemText(IDC_oks, msg);

			if (m_meter_type == 0)	// 初始
				m_meter_type = m_meter_type_new;

			if (m_meter_type_new != 0xF1 && m_meter_type != m_meter_type_new){
				unsigned int bps = (m_meter_type_new & 0x10)? 2400 : 1200;
				sprintf(msg, "不同通讯类型(%d)的表，请确认是批次改变吗？", bps);
				if (MessageBox(msg, "请确认",  MB_YESNO) == IDYES){
					m_meter_type = m_meter_type_new;
				}else{
					if (m_meter_type == 0xF1)
						m_meter_type = 0x10 ^ m_meter_type_new;
				}
			}
		}
		if (r != XSQL_OK)
			r = g_sql.geterror(m_lastMsg, sizeof(m_lastMsg));
	}
	char result_rec[64], snow[24];
	getnowtime(snow);
	sprintf(result_rec, "%d,%s,%s,%s\r\n", m_maxid, snow, meterNumber, m_sCompany);
	r = write_file(m_dataFileName, result_rec, m_lastMsg, sizeof(m_lastMsg));
	
	strcpy(m_lastOk, meterNumber);
}

void CMeterProduct::s_record_failed(const char * meterNumber)
{
	MP_failed f;
	strcpy(f.meterNumber_failed, meterNumber);
	strcpy(f.meterNumber_ok, m_lastOk);
	
	m_fails.add(&f);

	// 添加记录到文件(CSV)
	char result_rec[64], snow[24];
	getnowtime(snow);
	sprintf(result_rec, "%d,%s,%s*,%s\r\n", m_maxid, snow, meterNumber, m_sCompany);
	int r = write_file(m_dataFileName, result_rec, m_lastMsg, sizeof(m_lastMsg));

	playAlarm();
	m_lock_error = true;
	m_color = RGB(255, 0, 0);
}

void CMeterProduct::set_meterid_parms(char * parms)
{
	const char * pvendorid = LPCTSTR(m_vendorid);
	const char * pmeterid = LPCTSTR(m_MeterID);

	int lv = strlen(pvendorid);
	int lm = strlen(pmeterid);

	char v[8];
	if (lv < 4){
		sprintf(v, "%04s", pvendorid);
	}else if (lv > 4){
		memcpy(v, pvendorid, 4);  v[4] = 0;
	}else
		memcpy(v, pvendorid, 5);

	int meterStart = lm - 10;
	if (meterStart > 0){
		int l = min(meterStart, 4);
		if (memcmp(v+4-l, pmeterid, l) != 0){
			memcpy(v+4-l, pmeterid, l);
		}
		m_MeterID = pmeterid+meterStart;
		pmeterid = LPCTSTR(m_MeterID);
		m_MeterID_Edit.SetWindowText(m_MeterID);
		m_MeterID_Edit.SetSel(0, -1);
		m_MeterID_Edit.SetFocus();
	}

	if (strcmp(v, pvendorid) && lm > 10){
		playDing();
		sprintf(m_lastMsg, "原厂商号 %s 将改成 %s, 确认修改吗？\n确认请点击“是”", LPCTSTR(m_vendorid), v);
		if (MessageBox(m_lastMsg, "请确认",  MB_YESNO) == IDYES){
			m_vendorid = v;
		}
		strcpy(v, pvendorid);
		UpdateData(FALSE);
	}

	sprintf(parms, "0,%s,%s", v, pmeterid);
}

LRESULT CMeterProduct::OnNotifyMsg(WPARAM wParam, LPARAM lParam)
{
	const char * msg = (const char *)lParam;

	// show log
	g_msg_mem.lock();
	SetDlgItemText(IDC_P_RESULT, g_msg_mem.get_buf());
	g_msg_mem.unlock();
	SendDlgItemMessage(IDC_P_RESULT, WM_VSCROLL, SB_BOTTOM);
	strncpy(m_lastMsg, msg, sizeof(m_lastMsg)-1);  m_lastMsg[sizeof(m_lastMsg)-1] = 0;

	if (m_state != 4){
		char msg[256];

		if (g_worker.iserror()){
			strcpy(msg, g_worker.get_state());
			s_record_failed(LPCTSTR(m_fullMeterID));
		}else if (g_worker.isok() || g_worker.isend()){
			if (ht_isok(g_worker.get_singleState())){
				m_color = MP_RGB_GREEN;
				if (m_state == 3 && g_worker.isok() && !m_saved){
					m_saved = true;
					s_record_result(LPCTSTR(m_fullMeterID));
					m_lock_error = false;
					playOk();
				}
			}else{
				s_record_failed(LPCTSTR(m_fullMeterID));
			}
			msg[0] = 0;
			if (g_cmd == 1)
				strcpy(msg, LPCTSTR(m_MeterID));
			else
				GetDlgItemText(IDC_R_METERID, msg, 12);
			int l = strlen(msg);
			sprintf(msg+l, ":%s", g_worker.get_singleMsg());
		}else{
			m_color = RGB(0, 0, 0);
			strcpy(msg, g_worker.get_state());
		}
		SetDlgItemText(IDC_PS_RESULT, msg);
	}

	char meterID[64];
	set_meterid_parms(meterID);

	if (g_worker.isend()){
		xsys_sleep_ms(200);
		if (g_worker.iserror())
			m_state = 4;

		if (m_state == 6){	// 设置表具通讯bps
			m_state = 0;
			g_worker.notify("设置节点ID", meterID);
		}else if (m_state == 0){
			const unsigned char * pmeter_type = (const unsigned char *)g_worker.get_result();
			m_meter_type_new = pmeter_type[0];
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
//			memcpy(meterID+2, msg+4, 2);  meterID[4] = 0;
			memcpy(meterID+2, msg+2, 4);  meterID[6] = 0;
			SetDlgItemText(IDC_R_METERID_HEAD, meterID+2);
			meterID[6] = ',';
			strcpy(meterID+7, msg+6);
			SetDlgItemText(IDC_R_METERID, meterID+7);

			m_state = 7;
			g_worker.notify("调试用单表抄收", meterID);
		}
	}

	return 0;
}

void CMeterProduct::OnPOpenValve() 
{
	g_cmd = 2;

	set_TransConfig();
	m_state = 4;

	char meterID[64];
	set_meterid_parms(meterID);
	g_worker.notify("开阀", meterID);
}

void CMeterProduct::OnPCloseValve() 
{
	g_cmd = 3;
	
	set_TransConfig();
	m_state = 4;

	char meterID[64];
	set_meterid_parms(meterID);
	g_worker.notify("关阀", meterID);
}

void CMeterProduct::OnPGetID() 
{
	g_cmd = 4;

	set_TransConfig();
	m_state = 5;

	char meterID[64];
	set_meterid_parms(meterID);
	g_worker.notify("读设备地址", "99");
}

HBRUSH CMeterProduct::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	if (nCtlColor == CTLCOLOR_STATIC){
		pDC->SetBkMode(TRANSPARENT);
		if (pWnd->GetDlgCtrlID() == IDC_PS_RESULT)
		{
			pDC->SetTextColor(m_color);
		}else if (pWnd->GetDlgCtrlID() == IDC_oks){
			pDC->SetTextColor(MP_RGB_GREEN);
		}else if (pWnd->GetDlgCtrlID() == IDC_dups){
			pDC->SetTextColor(RGB(128,128,0));
		}
	}   
	return (HBRUSH)m_brush.GetSafeHandle();
}
