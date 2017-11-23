// Ht_meterProduct.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include <xshare_recv_mem.h>

#include <treeprop/TreePropSheet.h>
#include <treeprop/TreePropSheetEx.h>
#include <treeprop/TreePropSheetOffice2003.h>
#include <treeprop/ResizableSheet.h>

#include "ht_meterProduct.h"
#include "MeterProduct.h"

#include "xwork_trans.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHt_meterProductApp

BEGIN_MESSAGE_MAP(CHt_meterProductApp, CWinApp)
	//{{AFX_MSG_MAP(CHt_meterProductApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHt_meterProductApp construction

CHt_meterProductApp::CHt_meterProductApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CHt_meterProductApp object
// extern CHt_meterProductDlg * g_pdlg;

CHt_meterProductApp theApp;
CMeterProduct * g_pdlg = 0;

xini	g_ini;
xwork_trans g_worker;

/////////////////////////////////////////////////////////////////////////////
// CHt_meterProductApp initialization

extern int		gini_dialwait;

char g_runpath[MAX_PATH];
xshare_recv_mem	g_msg_mem("MSG_MEM", 4096);

BOOL CHt_meterProductApp::InitInstance()
{
	InitCommonControls();

	CWinApp::InitInstance();

	xsys_init(false);
	{
		char buf[MAX_PATH];
		GetModuleFileName(0, buf, MAX_PATH);
		set_run_path(xsys_getpath(g_runpath, buf, true));
		openservicelog((char *)"ht_meterProduct.log", false, 30, false, "logbak");

		g_ini.open("ht_meterProduct.ini");
	}

	g_msg_mem.init();

	{
		g_worker.m_idle = -1;

		g_worker.open();
		g_worker.start();

		CMeterProduct		meter_product;

		TreePropSheet::CTreePropSheetEx sht(_T("表具生产 (Ver 1.6.3)"));
		sht.SetTreeViewMode(TRUE, TRUE, TRUE);
		sht.SetEmptyPageText(_T("请选择具体的配置子项目 '%s'."));

		sht.AddPage(&meter_product);

		sht.SetIsResizable(true);

		m_pMainWnd = &sht;

		g_pdlg = &meter_product;
		sht.DoModal();

		g_worker.stop();
		g_worker.close();
	}

	g_msg_mem.down();

	xsys_down();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL CHt_meterProductApp::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_RETURN)){
		HWND hitem;
		g_pdlg->GetDlgItem(IDC_P_START, &hitem);
		PostMessage(hitem, BM_CLICK, 0, 0);
		return TRUE;
	}
	
	return CWinApp::PreTranslateMessage(pMsg);
}
