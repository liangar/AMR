#pragma once

#include <windows.h>
#include <comm.h>

#include "ht_cmn.h"

class kh_trans : public CComm
{
public:
	kh_trans();

	/// ��ʼ��
	int  init(const char * serialParam, const char * telnumber);
	int  init(int id);	/// ��ָ��id�ŵ��豸
	void down(void);	/// �ر�

	/// ͨѶ
	/// \param recvbuf	���ջ���
	/// \param sendbuf	���ͻ���
	/// \param sendlen	���ͳ���(���������)
	/// \param cmdtype	��������:0|1|n = ACK|1|n
	/// \return >=0/errcode ���ص��ֽ���/�������
	int trans(void);

	void show_recv_buf(void);
	void set_cmdType(ha_cmdType * pcmd)  {  m_pcmd = pcmd;  }

	const char * get_lastmsg(void)  {  return m_msg;  }

	void set_hook_wait(int ms)  {  if (ms < 500)  ms = 500;  if (ms > 10000)  ms = 10000;  m_hook_wait = ms;  }
	void set_dail_wait(int s)  {  if (s < 5)  s = 5;  if (s > 90)  s = 90;  m_dail_wait = s;   }
	void set_hangup_wait(int ms)  {  if (ms < 500)  ms = 500;  if (ms > 10000)  ms = 10000;  m_hangup_wait = ms;  }
	void set_no_meter_state(bool s)  {  m_no_state = s;  }

protected:
	int  send_record(void);	/// ��������
	int  recv_recs(void);	/// ���ն�����¼����
	int  recv_rec(void);	/// ���յ�����¼����

protected:
	int  clear_recv_buf(void);
	int  clear_all_recv(void);

	int  peek_byte(char * c);
	int  recv_byte(char * c, int timeout);
	int  recv_bytes(int n);
	int  send_byte(char c);
	int  send_bytes(const char * p, int l);
	int  send_bytes(const char * p);
	int  send_bytes(int l);

	/// AT����
	/// return <0/else = error/ok
	int  send_AT_cmd(const char * p, const char * okstring, int timeout = 1500);
	void re_send(char recs);

	int  sendack(char isok);

	void showProcessing(char i, char n);

public:
	char m_sendbuf[1024];
	char * m_recvbuf;
	long m_recs;
	char m_steps;
	ha_cmdType * m_pcmd;
	char m_telnumber[32];
	char m_serialport_string[128];
	bool m_bdialed;
	bool m_no_state;	/// �ޱ�״̬

	bool m_isSingle;	/// �Ƿ��ǵ������
	char m_singleState;	/// ����״̬

	char m_msg[1024];

	int  m_hook_wait;	/// ժ���ȴ�(ms)
	int  m_dail_wait;	/// �������ֵȴ�(s)
	int  m_hangup_wait;	/// �Ҷϵȴ�(ms)

	ha_cmd_result m_result;
};

extern kh_trans	g_trans;
