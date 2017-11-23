#include <xsys.h>

#include <ss_service.h>
#include <lfile.h>
#include <l_str.h>
#include <xini.h>
#include <xshare_recv_mem.h>

#include "xwork_trans.h"

#include "ht_cmn.h"
#include "ht_pms.h"

#include "KL_CMN.H"
#include "ht_32_gvars.h"

extern xini	g_ini;
extern void WriteToLog(const char * pFormat, ...);
extern xshare_recv_mem	g_msg_mem;


xwork_trans::xwork_trans()
	: xwork_server("�������", 5, 1)
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
		WriteToEventLog("%s : %s�رճ����쳣.", szFunctionName, m_name);
	}
	m_trans.down();

	xwork_server::close(secs);

	return true;
}

void xwork_trans::notify_wnd_message(void)
{
	if (m_hwnd){
		PostMessage(m_hwnd, m_wnd_message_id, 0, (LPARAM)m_lastmsg);
	}
}

void xwork_trans::run(void)
{
	static const char szFunctionName[] = "xwork_trans::run";

	bool isok = true;
	bool dbisopen = false;
	try{
		WriteToEventLog("%s : run in", m_name);

		m_trans.set_hook_wait(g_ini.get("����", "ժ���ȴ�(ms)", 2500));
		m_trans.set_dail_wait(g_ini.get("����", "�������ֵȴ�(s)", 30));
		m_trans.set_hangup_wait(g_ini.get("����", "�Ҷϵȴ�(ms)", 1800));

		g_sendbuf = m_trans.m_sendbuf;
		g_recvbuf = m_trans.m_recvbuf;

		if (m_cmd[0] >= '0' && m_cmd[0] <= '9'){
			m_icmd = atoi(m_cmd);
		}else if (stricmp(m_cmd, "down") == 0){
			m_icmd = -1;
			m_state = trans_null;
			m_trans.down();
		}else if (stricmp(m_cmd, "do_list") == 0){	// ���ݱ���б��ļ�����
			do_list("KT.TXT");
		}else{
			m_icmd = find_cmd(m_cmd);
		}
		if (m_icmd >= 0){
			m_lastCmdState = trans_start;
			sprintf(m_lastmsg, "׼��ִ��: %s(%s)", g_cmdTypes[m_icmd].pname, m_parms);
			m_state = trans_start;
			WriteToLog(m_lastmsg);
			xsys_sleep_ms(10);
			int r = trans_cmd();
		}
	}catch (...) {
		m_lastCmdState = trans_error;
		isok = false;
		WriteToLog("%s : %s���г���δ֪�쳣.", szFunctionName, m_name);
	}

	WriteToEventLog("%s : run out", m_name);
	xsys_sleep_ms(10);
}

int xwork_trans::trans(const char * parms, int steps)
{
	static const char szFunctionName[] = "ht_runtime::trans";

	m_trans.set_cmdType(g_cmdTypes+m_icmd);

	// ��ִ��,��������
	int r, n;
	memset(g_params, 0, sizeof(g_params));
	{
		// ������
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

	n = 0;
trans_begin:
	sprintf(m_lastmsg, "ִ������: %s(%s)", g_cmdTypes[m_icmd].pname, parms);
	m_lastCmdState = trans_doing;
	WriteToLog(m_lastmsg);
	xsys_sleep_ms(10);
	r = m_trans.trans();
	++n;

	// ���ɹ�,���߷ǳ���,ֱ�ӷ���
	if (r <= 0 || 
		(g_cmdTypes[m_icmd].pcmd_format[8] != '\x68' && g_cmdTypes[m_icmd].pcmd_format[8] != '\x60')
		)
	{
		if (r < 0)
			m_lastCmdState = trans_error;
		else
			m_lastCmdState = trans_ok;
		strcpy(m_lastmsg, m_trans.get_lastmsg());
		WriteToLog(m_lastmsg);

		if (n < 3 && m_lastCmdState == trans_error)
			goto trans_begin;

		return r;
	}

	// ������
	if (g_cmdTypes[m_icmd].pcmd_format[8] == '\x60' || g_cmdTypes[m_icmd].pcmd_format[8] == '\x43'){
		if (m_trans.m_result.rows != 1){
			m_lastCmdState = trans_error;
			sprintf(m_lastmsg, "%s: error, %s", szFunctionName, m_trans.get_lastmsg());
		}else
			m_lastCmdState = trans_ok;
		WriteToLog(m_lastmsg);

		if (n < 3 && m_lastCmdState == trans_error)
			goto trans_begin;

		return 1;
	}

	// �����ǳ������
	// д�ļ�
	if (g_cmdTypes[m_icmd].pcmd_format[8] == '\x68')
	{
		char fullpath[MAX_PATH];
		if (steps == 1)
			getfullname(fullpath, "DATA.TXT", sizeof(fullpath));
		else{
			fullpath[0] = '+';
			getfullname(fullpath+1, "DATA.TXT", sizeof(fullpath)-1);
		}
		r = write_file(fullpath, m_trans.m_result.pbuf, m_lastmsg, sizeof(m_lastmsg));
		if (r != 0){
			sprintf(m_lastmsg, "%s: д����ļ�(%s)����", szFunctionName, m_lastmsg);
			return r;
		}
		*m_lastmsg = 0;
	}

	// д���ݿ��¼
	if (g_cmdTypes[m_icmd].pcmd_format[8] == '\x68')
	{
		/// ��ʽ(21 bytes):
		/// <1 ������> 0
		/// <3 ���̺�> 1 - 3
		/// <8 ��ߺ�> 4 - 11
		/// <1 ��չλ> 12 = '0'
		/// <5 ��  ��> 13 - 17
		/// <2 ״̬λ> 18 - 19
		/// <1 ����λ> 20 = '\0'
		char * p = m_trans.m_result.pbuf;
		for (int i = 0; *p; p += KT_RECLEN_GY, i++){
			sprintf(m_lastmsg, "%4d\t", i+1);
			memcpy(m_lastmsg+5, p, KT_RECLEN_GY);
			m_lastmsg[KT_RECLEN_GY+5] = 0;
			m_lastmsg[12+5] = m_lastmsg[18+5] = '\t';
			notify_wnd_message();
		}
		sprintf(m_lastmsg, "������%d��", m_trans.m_result.rows);
	}
	return m_trans.m_result.rows;
}

int	xwork_trans::trans_cmd(void)
{
	int r = 0;
	// 1. ��ʼ������
	if (!m_trans.IsOpen())
	{
		char comstring[128];
		sprintf(comstring, "COM%s:baud=9600 parity=E data=8 stop=1", m_port);

		r = m_trans.init(comstring, m_telnumber);
		if (r < 0){
			strcpy(m_lastmsg, "ͨѶ��ʼ������");
			m_lastCmdState = trans_error;
			m_state = trans_end;
			return -1;
		}
	}

	int n  = 0;
	// 2. ��ʼִ������
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

	// 3. ת���ļ�
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

	/// ����ǿ��ط�����Ҫ�ȴ�6S����ִ����һ������
	if (r >= 0  && strstr(m_cmd, "��") != 0)
		Sleep(6000);

	if (r >= 0){
		if (strcmp(m_cmd, "�ط�") == 0)
			Sleep(1000);
		else if (strcmp(m_cmd, "����") == 0)
			Sleep(3000);
	}

	m_state = trans_end;
	notify_wnd_message();

	return r;
}

xwork_trans::find_cmd(const char * cmd)
{
	for (int i = 0; g_cmdTypes[i].pname[0] != 0; i++){
		if (stricmp(cmd, g_cmdTypes[i].pname) == 0)
			return i;
	}
	return -1;
}

int xwork_trans::do_list(const char * fileName)
{
	// ���ļ���������
	char * pbuf;
	int r = read_file(&pbuf, fileName, m_lastmsg, sizeof(m_lastmsg));
	if (r < 0){
		notify_wnd_message();
		sprintf(m_lastmsg, "�򿪱���б��ļ�(%s)����", fileName);
		notify_wnd_message();
		return r;
	}

	const char * p = pbuf;
	char * q = (char *)p;
	int i;
	for (i = 0; *p; i++){
		p = getaline(q, p);
		trimstr0(q);
		q += strlen(q) + 1;
	}

	// ����
	HWND hwnd = m_hwnd;
	p = pbuf;
	m_icmd = 1;
	q = 0;
	int l = 0;
	strcpy(m_lastmsg,
		"���             ��� ״̬ ����\r\n"
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

	// ���ؽ��
	m_state = trans_end;
	sprintf(m_lastmsg, "������%d��", i);
	notify_wnd_message();

	r = write_file("DATA.TXT", q, m_lastmsg, sizeof(m_lastmsg));
	if (r >= 0){
		char fullpath[MAX_PATH];
		char szNow[32];

		getsimple_now(szNow);
		sprintf(fullpath, "databak/DATA_%s.TXT", szNow);
		xsys_md("databak");

		xsys_cp(fullpath, "DATA.TXT");

		strcpy(m_lastmsg, "�Ѿ�д�����ļ�(DATA.TXT)");
	}else{
		notify_wnd_message();
		sprintf(m_lastmsg, "д����ļ�(DATA.TXT)����(%d): %s", r);
	}
	notify_wnd_message();

	free(pbuf);
	return i;
}

void AddMessageText(const char * msg)
{
	g_msg_mem.save_linecat(msg);
	g_worker.notify_wnd_message();
}
