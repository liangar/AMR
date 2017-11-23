#include <xsys.h>

#include <ss_service.h>
#include <lfile.h>
#include <l_str.h>
#include <xini.h>

#include "xwork_trans.h"

#include "ht_cmn.h"
#include "ht_pms.h"

#include "KL_CMN.H"
#include "ht_32_gvars.h"

extern xini	g_ini;

static int x_work_trans_bps = 2400;

xwork_trans::xwork_trans()
	: xwork_server("抄表服务", 5, 1)
	, m_state(trans_null)
	, m_lastCmdState(trans_null)
{
	m_hwnd = 0;
	m_port[0] = m_telnumber[0] = 0;
	m_bneedBackupData = false;
}

bool xwork_trans::close(int secs)
{
	static const char szFunctionName[] = "xwork_trans::close";

	try{
		m_lastCmdState = trans_null;
	}catch (...) {
		WriteToEventLog("%s : %s关闭出现异常.", szFunctionName, m_name);
	}
	m_trans.down();

	xwork_server::close(secs);

	return true;
}

void xwork_trans::notify_wnd_message(void)
{
	if (m_hwnd){
		SendMessage(m_hwnd, m_wnd_message_id, 0, (LPARAM)m_lastmsg);
	}
}

void xwork_trans::run(void)
{
	static const char szFunctionName[] = "xwork_trans::run";

	bool isok = true;
	bool dbisopen = false;
	try{
		WriteToEventLog("%s : run in(%s - %s)", m_name, m_cmd, m_parms);
		if (strcmp(m_cmd, "设置通讯速率") == 0){
			x_work_trans_bps = atoi(m_parms);
			m_lastCmdState = trans_ok;
			m_state = trans_end;
			sprintf(m_lastmsg, "%s: %s, OK", m_cmd, m_parms);
			notify_wnd_message();
			m_trans.down();
		}else{
			m_trans.set_hook_wait(g_ini.get("拨号", "摘机等待(ms)", 2500));
			m_trans.set_dail_wait(g_ini.get("拨号", "拨号握手等待(s)", 30));
			m_trans.set_hangup_wait(g_ini.get("拨号", "挂断等待(ms)", 1800));
			
			g_sendbuf = m_trans.m_sendbuf;
			g_recvbuf = m_trans.m_recvbuf;
			
			if (m_cmd[0] >= '0' && m_cmd[0] <= '9'){
				m_icmd = atoi(m_cmd);
			}else if (_stricmp(m_cmd, "down") == 0){
				m_icmd = -1;
				m_state = trans_null;
				m_trans.down();
			}else if (_stricmp(m_cmd, "do_list") == 0){	// 根据表具列表文件抄表
				do_list("KT.TXT");
			}else{
				m_icmd = find_cmd(m_cmd);
			}
			if (m_icmd >= 0){
				m_lastCmdState = trans_start;
				sprintf(m_lastmsg, "准备执行: %s(%s)", g_cmdTypes[m_icmd].pname, m_parms);
				m_state = trans_start;
				notify_wnd_message();
				int r = trans_cmd();
			}
		}
	}catch (...) {
		m_lastCmdState = trans_error;
		isok = false;
		WriteToEventLog("%s : %s运行出现未知异常.", szFunctionName, m_name);
	}

	WriteToEventLog("%s : run out", m_name);
}

int xwork_trans::trans(const char * parms, int steps)
{
	static const char szFunctionName[] = "ht_runtime::trans";

	m_trans.set_cmdType(g_cmdTypes+m_icmd);

	// 将执行,解析参数
	int r;
	memset(g_params, 0, sizeof(g_params));
	{
		// 输入表号
		if (strcmp(g_cmdTypes[m_icmd].pdescription, "123") == 0){
			const char * p = strrchr(parms, ',');
			char ps[64];

			if (p == 0)
				sprintf(ps, "0,0,%s", parms);
			else
				strcpy(ps, parms);
			r = kh_inparams(g_cmdTypes+m_icmd, ps);
		}else{
			r = kh_inparams(g_cmdTypes+m_icmd, parms);
		}
	}

	g_icmd = m_icmd;
	sprintf(m_lastmsg, "执行命令: %s(%s)", g_cmdTypes[m_icmd].pname, parms);
	m_lastCmdState = trans_doing;
	notify_wnd_message();
	r = m_trans.trans();

	// 不成功,或者非抄表,直接返回
	if (r < 0)
		m_lastCmdState = trans_error;
	else
		m_lastCmdState = trans_ok;
	strcpy(m_lastmsg, m_trans.get_lastmsg());
	WriteToEventLog(m_lastmsg);
	notify_wnd_message();

	return m_trans.m_result.rows;
}

int	xwork_trans::trans_cmd(void)
{
	int r = 0;
	// 1. 初始化连接
	if (!m_trans.IsOpen())
	{
		char comstring[128];
		sprintf(comstring, "COM%s:baud=%d parity=E data=8 stop=1", m_port, x_work_trans_bps);

		r = m_trans.init(comstring, m_telnumber);
		if (r < 0){
			strcpy(m_lastmsg, "通讯初始化出错");
			m_lastCmdState = trans_error;
			m_state = trans_end;
			return -1;
		}
	}

	int n  = 0;
	// 2. 开始执行命令
	if (g_cmdTypes[m_icmd].pcmd_format[8] == '\x68'){
		char devices[256];
		strcpy(devices, m_parms);
		splitstr(devices, ',');
		int j;
		j = n = 0;
		for (char * p = devices; *p; p += strlen(p) + 1, j++){
			r = trans(p, j+1);
			if (r > 0)
				n += r;
			Sleep(1000);
		}
	}else{
		r = trans(m_parms, 1);
	}

	// 3. 转存文件
	if (r >= 0 && n > 0 && g_cmdTypes[m_icmd].pcmd_format[8] == '\x68' && m_bneedBackupData){
		char fullpath[MAX_PATH];
		char szNow[32];

		getsimple_now(szNow);
		sprintf(fullpath, "databak/DATA_%s.TXT", szNow);
		xsys_md("databak");

		xsys_cp(fullpath, "DATA.TXT");
	}
	/*
	else{
		read_file(m_trans.m_result.pbuf, "DATA.txt", m_lastmsg, sizeof(m_lastmsg));
		notify_wnd_message();
	}
	//*/

	/// 如果是开关阀，需要等待6S才能执行下一条命令
	if (r >= 0  && strstr(m_cmd, "阀") != 0)
		Sleep(6000);

	m_state = trans_end;
	notify_wnd_message();

	return r;
}

int xwork_trans::find_cmd(const char * cmd)
{
	for (int i = 0; g_cmdTypes[i].pname[0] != 0; i++){
		if (_stricmp(cmd, g_cmdTypes[i].pname) == 0)
			return i;
	}
	return -1;
}

int xwork_trans::do_list(const char * fileName)
{
	// 打开文件，读入表具
	char * pbuf;
	int r = read_file(&pbuf, fileName, m_lastmsg, sizeof(m_lastmsg));
	if (r < 0){
		notify_wnd_message();
		sprintf(m_lastmsg, "打开表号列表文件(%s)出错", fileName);
		notify_wnd_message();
		return r;
	}

	char * p = pbuf;
	char * q = p;
	int i;
	for (i = 0; *p; i++){
		p = (char *)getaline(q, (const char *)p);
		trimstr0(q);
		q += strlen(q) + 1;
	}

	// 抄表
	HWND hwnd = m_hwnd;
	p = pbuf;
	m_icmd = 1;
	q = 0;
	int l = 0;
	strcpy(m_lastmsg,
		"序号             表号 状态 度数\r\n"
		"---- ---------------- ---- -----"
	);
	notify_wnd_message();

	for (int j = 0; j < i; j++){
		sprintf(m_parms, "0,0,%s", p);
		char id[32];
		strcpy(id, p);
		ht_lpad(id, 11);

		m_lastCmdState = trans_start;
		m_state = trans_start;

		m_hwnd = 0;
		r = trans_cmd();

		if (r >= 0){
			if (q == 0)
				q = m_trans.m_recvbuf + 1500;

			sprintf(m_lastmsg, "%4d %16s %4c %5s",
				j+1,
				p, 
				m_trans.m_result.pbuf[0],
				m_trans.m_result.pbuf+1
			);

			l += sprintf(q+l, "S%11s0%5s0%c\r\n",
				id,
				m_trans.m_result.pbuf+1,
				m_trans.m_result.pbuf[0]
			);
		}else{
			sprintf(m_lastmsg, "%4d %16s %s",
				j+1,
				p,
				m_trans.get_lastmsg()
			);
		}
		m_hwnd = hwnd;
		notify_wnd_message();

		p += strlen(p) + 1;
	}
	q[l++] = 0;  q[l] = 0;

	m_icmd = -1;

	m_hwnd = hwnd;

	// 返回结果
	m_state = trans_end;
	sprintf(m_lastmsg, "共抄表%d个", i);
	notify_wnd_message();

	r = write_file("DATA.TXT", q, m_lastmsg, sizeof(m_lastmsg));
	if (r >= 0){
		char fullpath[MAX_PATH];
		char szNow[32];

		getsimple_now(szNow);
		sprintf(fullpath, "databak/DATA_%s.TXT", szNow);
		xsys_md("databak");

		xsys_cp(fullpath, "DATA.TXT");

		strcpy(m_lastmsg, "已经写入结果文件(DATA.TXT)");
	}else{
		notify_wnd_message();
		sprintf(m_lastmsg, "写结果文件(DATA.TXT)出错(%d): %s", r);
	}
	notify_wnd_message();

	free(pbuf);
	return i;
}

void AddMessageText(const char * msg)
{
	g_worker.set_msg(msg);
	g_worker.notify_wnd_message();
}
